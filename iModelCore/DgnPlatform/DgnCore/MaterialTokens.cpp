/*----------------------------------------------------------------------+
|
|   $Source: DgnCore/MaterialTokens.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatformInternal/DgnCore/MaterialTokens.h>

struct Keyword
    {
    int32_t     m_token;
    char        m_keyword[40];
    };

static Keyword s_paletteKeywords[] =
    {
        { PALETTETOKEN_Material,                            "material" },
        { PALETTETOKEN_Color,                               "color" },
        { PALETTETOKEN_SpecularColor,                       "specular_color" },
        { PALETTETOKEN_Diffuse,                             "diffuse" },
        { PALETTETOKEN_Reflect,                             "reflect" },
        { PALETTETOKEN_Transmit,                            "transmit" },
        { PALETTETOKEN_Refract,                             "refract" },
        { PALETTETOKEN_Specular,                            "specular" },
        { PALETTETOKEN_Finish,                              "finish" },
        { PALETTETOKEN_Highlight,                           "highlight" },
        { PALETTETOKEN_Pattern,                             "pattern" },
        { PALETTETOKEN_PatternOff,                          "pattern_off" },
        { PALETTETOKEN_PatternSize,                         "pattern_size" },
        { PALETTETOKEN_PatternAngle,                        "pattern_angle" },
        { PALETTETOKEN_PatternScale,                        "pattern_scale" },
        { PALETTETOKEN_PatternOffset,                       "pattern_offset" },
        { PALETTETOKEN_PatternFlipV,                        "pattern_flip" },
        { PALETTETOKEN_PatternLockSize,                     "pattern_lock_size" },
        { PALETTETOKEN_PatternWeight,                       "pattern_weight" },
        { PALETTETOKEN_PatternMapping,                      "pattern_mapping" },
        { PALETTETOKEN_PatternScaleMode,                    "pattern_scalemode" },
        { PALETTETOKEN_BgTrans,                             "bgtrans" },
        { PALETTETOKEN_Fresnel,                             "fresnel" },
        { PALETTETOKEN_PixiePattern,                        "pixie_pattern" },
        { PALETTETOKEN_PixieColor,                          "pixie_color" },
        { PALETTETOKEN_BumpMap,                             "bump_map" },
        { PALETTETOKEN_BumpMapOff,                          "bump_map_off" },
        { PALETTETOKEN_BumpMapScale,                        "bump_map_scale" },
        { PALETTETOKEN_BumpMapInvert,                       "bump_map_invert" },
        { PALETTETOKEN_Shadows,                             "shadows" },
        { PALETTETOKEN_End,                                 "end" },
        { PALETTETOKEN_Ambient,                             "ambient" },
        { PALETTETOKEN_Radiosity,                           "radiosity" },
        { PALETTETOKEN_LockEfficiency,                      "lock_efficiency" },
        { PALETTETOKEN_LockSpecularAndReflect,              "lock_specular_and_reflect" },
        { PALETTETOKEN_LockFinishToSpecular,                "lock_finish_to_specular" },
        { PALETTETOKEN_ActualReflect,                       "actual_reflect" },
        { PALETTETOKEN_Translucency,                        "translucency" },
        { PALETTETOKEN_Glow,                                "glow" },
        { PALETTETOKEN_Thickness,                           "thickness" },
        { PALETTETOKEN_MapState,                            "map_state" },
        { PALETTETOKEN_MapLink,                             "map_link" },
        { PALETTETOKEN_PatternUnits,                        "pattern_units" },
        { PALETTETOKEN_MaterialVersion,                     "material_version" },
        { PALETTETOKEN_LayerType,                           "layer" },
        { PALETTETOKEN_LayerState,                          "layer_state" },
        { PALETTETOKEN_LayerGamma,                          "layer_gamma" },
        { PALETTETOKEN_LayerColor,                          "layer_color" },
        { PALETTETOKEN_LayerBrightness,                     "layer_brightness" },
        { PALETTETOKEN_LayerContrast,                       "layer_contrast" },
        { PALETTETOKEN_LayerOpacity,                        "pattern_opacity" },
        { PALETTETOKEN_LayerBlend,                          "layer_blend" },
        { PALETTETOKEN_LayerTileMode,                       "layer_tile_mode" },
        { PALETTETOKEN_ProjectionOffset,                    "pattern_proj_offset" },
        { PALETTETOKEN_ProjectionAngles,                    "pattern_proj_angles" },
        { PALETTETOKEN_ProjectionScale,                     "pattern_proj_scale" },
        { PALETTETOKEN_PatternFlipU,                        "pattern_u_flip" },
        { PALETTETOKEN_PatternTileDecalU,                   "pattern_tile_decal_u" },
        { PALETTETOKEN_PatternTileDecalV,                   "pattern_tile_decal_v" },
        { PALETTETOKEN_PatternTileMirrorU,                  "pattern_tile_mirror_u" },
        { PALETTETOKEN_PatternTileMirrorV,                  "pattern_tile_mirror_v" },
        { PALETTETOKEN_PatternLockProjectionScale,          "pattern_lock_projection_scale" },
        { PALETTETOKEN_PatternCylCapped,                    "pattern_cylindrical_projection_capped" },
        { PALETTETOKEN_SpecularMap,                         "specular_map" },
        { PALETTETOKEN_SpecularMapOff,                      "specular_map_off" },
        { PALETTETOKEN_ReflectMap,                          "reflect_map" },
        { PALETTETOKEN_ReflectMapOff,                       "reflect_map_off" },
        { PALETTETOKEN_TransparencyMap,                     "transparency_map" },
        { PALETTETOKEN_TransparencyMapOff,                  "transparency_map_off" },
        { PALETTETOKEN_TranslucencyMap,                     "translucency_map" },
        { PALETTETOKEN_TranslucencyMapOff,                  "translucency_map_off" },
        { PALETTETOKEN_FinishMap,                           "finish_map" },
        { PALETTETOKEN_FinishMapOff,                        "finish_map_off" },
        { PALETTETOKEN_AssignmentOrAttachmentString,        "assignments_attachments" },
        { PALETTETOKEN_BlurReflections,                     "blur_reflections" },
        { PALETTETOKEN_ReflectionRays,                      "reflection_rays" },
        { PALETTETOKEN_DisplacementDistance,                "displacement_distance" },
        { PALETTETOKEN_LinkedToLXP,                         "linked_to_lxp" },
        { PALETTETOKEN_InvisibleToEye,                      "invisible_to_eye" },
        { PALETTETOKEN_Dispersion,                          "dispersion" },
        { PALETTETOKEN_Clearcoat,                           "clearcoat" },
        { PALETTETOKEN_BlurRefractions,                     "blur_refractions" },
        { PALETTETOKEN_RefractionRays,                      "refraction_rays" },
        { PALETTETOKEN_ShaderType,                          "shader_type" },
        { PALETTETOKEN_ShaderEffect,                        "shader_effect" },
        { PALETTETOKEN_ShaderFlags,                         "shader_flags" },
        { PALETTETOKEN_ShaderBlend,                         "shader_blend" },
        { PALETTETOKEN_ShaderOpacity,                       "shader_opacity" },
        { PALETTETOKEN_ShaderRate,                          "shader_rate" },
        { PALETTETOKEN_ShaderDirectMultiplier,              "shader_direct_mult" },
        { PALETTETOKEN_ShaderIndirectMultiplier,            "shader_indirect_mult" },
        { PALETTETOKEN_ShaderSaturation,                    "shader_saturation" },
        { PALETTETOKEN_ShaderIndirectIllumType,             "shader_indirect_illum_type" },
        { PALETTETOKEN_ShaderFlagEnabled,                   "shader_flag_enabled" },
        { PALETTETOKEN_ShaderFlagInvert,                    "shader_flag_invert" },
        { PALETTETOKEN_ShaderFlagReceiveShadows,            "shader_flag_receive_shadows" },
        { PALETTETOKEN_ShaderFlagVisibleToCamera,           "shader_flag_visible_to_camera" },
        { PALETTETOKEN_ShaderFlagVisibleToIndirectRays,     "shader_flag_visible_to_indirect_rays" },
        { PALETTETOKEN_ShaderFlagVisibleToReflectionRays,   "shader_flag_visible_to_reflection_rays" },
        { PALETTETOKEN_ShaderFlagVisibleToRefractionRays,   "shader_flag_visible_to_refraction_rays" },
        { PALETTETOKEN_DiffuseMap,                          "diffuse_map" },
        { PALETTETOKEN_DiffuseMapOff,                       "diffuse_map_off" },
        { PALETTETOKEN_GlowAmountMap,                       "glow_amount_map" },
        { PALETTETOKEN_GlowAmountMapOff,                    "glow_amount_map_off" },
        { PALETTETOKEN_ClearcoatAmountMap,                  "clearcoat_amount_map" },
        { PALETTETOKEN_ClearcoatAmountMapOff,               "clearcoat_amount_map_off" },
        { PALETTETOKEN_AnisotropicDirectionMap,             "anisotropic_direction_map" },
        { PALETTETOKEN_AnisotropicDirectionMapOff,          "anisotropic_direction_map_off" },
        { PALETTETOKEN_SpecularColorMap,                    "specular_color_map" },
        { PALETTETOKEN_SpecularColorMapOff,                 "specular_color_map_off" },
        { PALETTETOKEN_TransparentColorMap,                 "transparent_color_map" },
        { PALETTETOKEN_TransparentColorMapOff,              "transparent_color_map_off" },
        { PALETTETOKEN_TranslucencyColorMap,                "translucent_color_map" },
        { PALETTETOKEN_TranslucencyColorMapOff,             "translucent_color_map_off" },
        { PALETTETOKEN_TransparentColor,                    "transparent_color" },
        { PALETTETOKEN_TranslucencyColor,                   "translucent_color" },
        { PALETTETOKEN_Anisotropy,                          "anisotropy" },
        { PALETTETOKEN_FrontWeighting,                      "front_weighting" },
        { PALETTETOKEN_ScatteringDistance,                  "scattering_distance" },
        { PALETTETOKEN_SubsurfaceSamples,                   "subsurface_samples" },
        { PALETTETOKEN_ValueMapInvert,                      "value_map_invert" },
        { PALETTETOKEN_FurSpacing,                          "fur_spacing" },
        { PALETTETOKEN_FurLength,                           "fur_length" },
        { PALETTETOKEN_FurWidth,                            "fur_width" },
        { PALETTETOKEN_FurTaper,                            "fur_taper" },
        { PALETTETOKEN_FurOffset,                           "fur_offset" },
        { PALETTETOKEN_FurStripRotation,                    "fur_strip_rotation" },
        { PALETTETOKEN_FurGrowthJitter,                     "fur_growth_jitter" },
        { PALETTETOKEN_FurPositionJitter,                   "fur_position_jitter" },
        { PALETTETOKEN_FurDirectionJitter,                  "fur_direction_jitter" },
        { PALETTETOKEN_FurSizeJitter,                       "fur_size_jitter" },
        { PALETTETOKEN_FurFlex,                             "fur_flex" },
        { PALETTETOKEN_FurRootBend,                         "fur_root_bend" },
        { PALETTETOKEN_FurCurls,                            "fur_curls" },
        { PALETTETOKEN_FurBumpAmplitude,                    "fur_bump_amplitude" },
        { PALETTETOKEN_FurGuideRange,                       "fur_guide_range" },
        { PALETTETOKEN_FurGuideLength,                      "fur_guide_length" },
        { PALETTETOKEN_FurBlendAmount,                      "fur_blend_amount" },
        { PALETTETOKEN_FurBlendAngle,                       "fur_blend_angle" },
        { PALETTETOKEN_FurClumps,                           "fur_clumps" },
        { PALETTETOKEN_FurClumpRange,                       "fur_clump_range" },
        { PALETTETOKEN_FurRate,                             "fur_rate" },
        { PALETTETOKEN_FurType,                             "fur_type" },
        { PALETTETOKEN_FurGuides,                           "fur_guides" },
        { PALETTETOKEN_FurSegments,                         "fur_segments" },
        { PALETTETOKEN_FurAdaptiveSampling,                 "fur_flag_adaptive_sampling" },
        { PALETTETOKEN_FurRemoveBaseSurface,                "fur_flag_remove_base_surface" },
        { PALETTETOKEN_UseFur,                              "use_fur" },
        { PALETTETOKEN_DisplacementMap,                     "displacement_map" },
        { PALETTETOKEN_DisplacementMapOff,                  "displacement_map_off" },
        { PALETTETOKEN_NormalMap,                           "normal_map" },
        { PALETTETOKEN_NormalMapOff,                        "normal_map_off" },
        { PALETTETOKEN_FurLengthMap,                        "fur_lengthM_map" },
        { PALETTETOKEN_FurLengthMapOff,                     "fur_lengthO_map_off" },
        { PALETTETOKEN_FurDensityMap,                       "fur_density_map" },
        { PALETTETOKEN_FurDensityMapOff,                    "fur_density_map_off" },
        { PALETTETOKEN_FurJitterMap,                        "fur_jitter_map" },
        { PALETTETOKEN_FurJitterMapOff,                     "fur_jitter_map_off" },
        { PALETTETOKEN_FurFlexMap,                          "fur_flex_map" },
        { PALETTETOKEN_FurFlexMapOff,                       "fur_flex_map_off" },
        { PALETTETOKEN_FurClumpsMap,                        "fur_clumps_map" },
        { PALETTETOKEN_FurClumpsMapOff,                     "fur_clumps_map_off" },
        { PALETTETOKEN_FurDirectionMap,                     "fur_direction_map" },
        { PALETTETOKEN_FurDirectionMapOff,                  "fur_direction_map_off" },
        { PALETTETOKEN_FurVectorMap,                        "fur_vector_map" },
        { PALETTETOKEN_FurVectorMapOff,                     "fur_vector_map_off" },
        { PALETTETOKEN_FurBumpMap,                          "fur_bump_map" },
        { PALETTETOKEN_FurBumpMapOff,                       "fur_bump_map_off" },
        { PALETTETOKEN_FurCurlsMap,                         "fur_curls_map" },
        { PALETTETOKEN_FurCurlsMapOff,                      "fur_curls_map_off" },
        { PALETTETOKEN_MapLayerType,                        "map_type" },
        { PALETTETOKEN_LxoProcType,                         "lxo_procedure_type" },
        { PALETTETOKEN_LxoNoiseColor1,                      "lxo_noise_color1" },
        { PALETTETOKEN_LxoNoiseAlpha1,                      "lxo_noise_alpha1" },
        { PALETTETOKEN_LxoNoiseColor2,                      "lxo_noise_color2" },
        { PALETTETOKEN_LxoNoiseAlpha2,                      "lxo_noise_alpha2" },
        { PALETTETOKEN_LxoNoiseType,                        "lxo_noise_type" },
        { PALETTETOKEN_LxoNoiseFreq,                        "lxo_noise_freq" },
        { PALETTETOKEN_LxoNoiseFreqRatio,                   "lxo_noise_freq_ratio" },
        { PALETTETOKEN_LxoNoiseAmplitudeRatio,              "lxo_noise_amplitude_ratio" },
        { PALETTETOKEN_LxoNoiseBias,                        "lxo_noise_bias" },
        { PALETTETOKEN_LxoNoiseGain,                        "lxo_noise_gain" },
        { PALETTETOKEN_LxoCheckerColor1,                    "lxo_checker_color1" },
        { PALETTETOKEN_LxoCheckerAlpha1,                    "lxo_checker_alpha1" },
        { PALETTETOKEN_LxoCheckerColor2,                    "lxo_checker_color2" },
        { PALETTETOKEN_LxoCheckerAlpha2,                    "lxo_checker_alpha2" },
        { PALETTETOKEN_LxoCheckerType,                      "lxo_checker_type" },
        { PALETTETOKEN_LxoCheckerTransitionWidth,           "lxo_checker_transition_width" },
        { PALETTETOKEN_LxoCheckerBias,                      "lxo_checker_bias" },
        { PALETTETOKEN_LxoCheckerGain,                      "lxo_checker_gain" },
        { PALETTETOKEN_LxoGridLineColor,                    "lxo_grid_line_color" },
        { PALETTETOKEN_LxoGridLineAlpha,                    "lxo_grid_line_alpha" },
        { PALETTETOKEN_LxoGridFillerColor,                  "lxo_grid_filler_color" },
        { PALETTETOKEN_LxoGridFillerAlpha,                  "lxo_grid_filler_alpha" },
        { PALETTETOKEN_LxoGridTransitionWidth,              "lxo_grid_transition_width" },
        { PALETTETOKEN_LxoGridType,                         "lxo_grid_type" },
        { PALETTETOKEN_LxoGridLineWidth,                    "lxo_grid_line_width" },
        { PALETTETOKEN_LxoGridBias,                         "lxo_grid_bias" },
        { PALETTETOKEN_LxoGridGain,                         "lxo_grid_gain" },
        { PALETTETOKEN_LxoDotColor,                         "lxo_dot_color" },
        { PALETTETOKEN_LxoDotAlpha,                         "lxo_dot_alpha" },
        { PALETTETOKEN_LxoDotFillerColor,                   "lxo_dot_filler_color" },
        { PALETTETOKEN_LxoDotFillerAlpha,                   "lxo_dot_filler_alpha" },
        { PALETTETOKEN_LxoDotTransitionWidth,               "lxo_dot_transition_width" },
        { PALETTETOKEN_LxoDotType,                          "lxo_dot_type" },
        { PALETTETOKEN_LxoDotWidth,                         "lxo_dot_width" },
        { PALETTETOKEN_LxoDotBias,                          "lxo_dot_bias" },
        { PALETTETOKEN_LxoDotGain,                          "lxo_dot_gain" },
        { PALETTETOKEN_LxoConstantColor,                    "lxo_constant_color" },
        { PALETTETOKEN_LxoCellularColor,                    "lxo_cellular_color" },
        { PALETTETOKEN_LxoCellularFillerColor,              "lxo_cellular_filler_color" },
        { PALETTETOKEN_LxoCellularAlpha,                    "lxo_cellular_alpha" },
        { PALETTETOKEN_LxoCellularFillerAlpha,              "lxo_cellular_filler_alpha" },
        { PALETTETOKEN_LxoCellularTransitionWidth,          "lxo_cellular_transition_width" },
        { PALETTETOKEN_LxoCellularWidth,                    "lxo_cellular_width" },
        { PALETTETOKEN_LxoCellularBias,                     "lxo_cellular_bias" },
        { PALETTETOKEN_LxoCellularGain,                     "lxo_cellular_gain" },
        { PALETTETOKEN_LxoCellularFreqRatio,                "lxo_cellular_freq_ratio" },
        { PALETTETOKEN_LxoCellularAmplitudeRatio,           "lxo_cellular_amplitude_ratio" },
        { PALETTETOKEN_LxoCellularValueVariation,           "lxo_cellular_value_variation" },
        { PALETTETOKEN_LxoCellularHueVariation,             "lxo_cellular_hue_variation" },
        { PALETTETOKEN_LxoCellularSaturationVariation,      "lxo_cellular_saturation_variation" },
        { PALETTETOKEN_LxoCellularType,                     "lxo_cellular_type" },
        { PALETTETOKEN_LxoCellularFrequencies,              "lxo_cellular_freq" },
        { PALETTETOKEN_LxoWoodColor,                        "lxo_wood_color" },
        { PALETTETOKEN_LxoWoodAlpha,                        "lxo_wood_alpha" },
        { PALETTETOKEN_LxoWoodFillerColor,                  "lxo_wood_filter_color" },
        { PALETTETOKEN_LxoWoodFillerAlpha,                  "lxo_wood_filter_alpha" },
        { PALETTETOKEN_LxoWoodRingScale,                    "lxo_wood_ring_scale" },
        { PALETTETOKEN_LxoWoodWaveScale,                    "lxo_wood_wave_scale" },
        { PALETTETOKEN_LxoWoodWaviness,                     "lxo_wood_waviness" },
        { PALETTETOKEN_LxoWoodGraininess,                   "lxo_wood_graininess" },
        { PALETTETOKEN_LxoWoodGrainScale,                   "lxo_wood_grain_scale" },
        { PALETTETOKEN_LxoWoodBias,                         "lxo_wood_bias" },
        { PALETTETOKEN_LxoWoodGain,                         "lxo_wood_gain" },
        { PALETTETOKEN_LxoWeaveYarnColor,                   "lxo_weave_yarn_color" },
        { PALETTETOKEN_LxoWeaveGapColor,                    "lxo_weave_gap_color" },
        { PALETTETOKEN_LxoWeaveYarnAlpha,                   "lxo_weave_yarn_alpha" },
        { PALETTETOKEN_LxoWeaveGapAlpha,                    "lxo_weave_gap_alpha" },
        { PALETTETOKEN_LxoWeaveYarnWidth,                   "lxo_weave_yarn_width" },
        { PALETTETOKEN_LxoWeaveRoundness,                   "lxo_weave_roundness" },
        { PALETTETOKEN_LxoWeaveBias,                        "lxo_weave_bias" },
        { PALETTETOKEN_LxoWeaveGain,                        "lxo_weave_gain" },
        { PALETTETOKEN_LxoRipplesCrestColor,                "lxo_ripples_crest_color" },
        { PALETTETOKEN_LxoRipplesCrestAlpha,                "lxo_ripples_crest_alpha" },
        { PALETTETOKEN_LxoRipplesTroughColor,               "lxo_ripples_trough_color" },
        { PALETTETOKEN_LxoRipplesTroughAlpha,               "lxo_ripples_trough_alpha" },
        { PALETTETOKEN_LxoRipplesWavelength,                "lxo_ripples_wavelength" },
        { PALETTETOKEN_LxoRipplesPhase,                     "lxo_ripples_phase" },
        { PALETTETOKEN_LxoRipplesBias,                      "lxo_ripples_bias" },
        { PALETTETOKEN_LxoRipplesGain,                      "lxo_ripples_gain" },
        { PALETTETOKEN_LxoRipplesSources,                   "lxo_ripples_sources" },
        { PALETTETOKEN_LxoGradientVersion,                  "lxo_gradient_version" },
        { PALETTETOKEN_LxoGradientEnvType,                  "lxo_gradient_env_type" },
        { PALETTETOKEN_LxoGradientPre,                      "lxo_gradient_pre" },
        { PALETTETOKEN_LxoGradientPost,                     "lxo_gradient_post" },
        { PALETTETOKEN_LxoGradientKeyX,                     "lxo_gradient_keyX" },
        { PALETTETOKEN_LxoGradientKeyY,                     "lxo_gradient_keyY" },
        { PALETTETOKEN_LxoGradientTANISlopeType,            "lxo_gradient_TANI_slope_type" },
        { PALETTETOKEN_LxoGradientTANIWeightType,           "lxo_gradient_TANI_weight_type" },
        { PALETTETOKEN_LxoGradientTANISlope,                "lxo_gradient_TANI_slope" },
        { PALETTETOKEN_LxoGradientTANIWeight,               "lxo_gradient_TANI_weight" },
        { PALETTETOKEN_LxoGradientTANIValue,                "lxo_gradient_TANI_value" },
        { PALETTETOKEN_LxoGradientTANOBreaks,               "lxo_gradient_TANO_breaks" },
        { PALETTETOKEN_LxoGradientTANOSlopeType,            "lxo_gradient_TANO_slope_type" },
        { PALETTETOKEN_LxoGradientTANOWeightType,           "lxo_gradient_TANO_weight_type" },
        { PALETTETOKEN_LxoGradientTANOSlope,                "lxo_gradient_TANO_slope" },
        { PALETTETOKEN_LxoGradientTANOWeight,               "lxo_gradient_TANO_weight" },
        { PALETTETOKEN_LxoGradientTANOValue,                "lxo_gradient_TANO_value" },
        { PALETTETOKEN_LxoGradientFlag,                     "lxo_gradient_flag" },
        { PALETTETOKEN_LxoGradientInput,                    "lxo_gradient_input" },
        { PALETTETOKEN_OffsetZ,                             "offset_z" },
        { PALETTETOKEN_ScaleZ,                              "scale_z" },
        { PALETTETOKEN_LxoNoiseValue1,                      "lxo_noise_value1" },
        { PALETTETOKEN_LxoNoiseValue2,                      "lxo_noise_value2" },
        { PALETTETOKEN_LxoCheckerValue1,                    "lxo_checker_value1" },
        { PALETTETOKEN_LxoCheckerValue2,                    "lxo_checker_value2" },
        { PALETTETOKEN_LxoGridLineValue,                    "lxo_grid_line_value" },
        { PALETTETOKEN_LxoGridFillerValue,                  "lxo_grid_filler_value" },
        { PALETTETOKEN_LxoDotValue,                         "lxo_dot_value" },
        { PALETTETOKEN_LxoDotFillerValue,                   "lxo_dot_filler_value" },
        { PALETTETOKEN_LxoConstantValue,                    "lxo_constant_value" },
        { PALETTETOKEN_LxoCellularCellValue,                "lxo_cellular_cell_value" },
        { PALETTETOKEN_LxoCellularFillerValue,              "lxo_cellular_filler_value" },
        { PALETTETOKEN_LxoWoodRingValue,                    "lxo_wood_ring_value" },
        { PALETTETOKEN_LxoWoodFillerValue,                  "lxo_wood_filler_value" },
        { PALETTETOKEN_LxoWeaveYarnValue,                   "lxo_weave_yarn_value" },
        { PALETTETOKEN_LxoWeaveGapValue,                    "lxo_weave_gap_value" },
        { PALETTETOKEN_LxoRipplesCrestValue,                "lxo_ripples_crest_value" },
        { PALETTETOKEN_LxoRipplesTroughValue,               "lxo_ripples_trough_value" },
        { PALETTETOKEN_MapEnd,                              "map_end" },
        { PALETTETOKEN_Invisible,                           "invisible" },
        { PALETTETOKEN_ReflectFresnel,                      "reflect_fresnel" },
        { PALETTETOKEN_Dissolve,                            "dissolve" },
        { PALETTETOKEN_AbsorptionDistance,                  "absorption_distance" },
        { PALETTETOKEN_RefractionRoughness,                 "refraction_roughness" },
        { PALETTETOKEN_GlowColor,                           "glow_color" },
        { PALETTETOKEN_GlowColorMap,                        "glow_color_map" },
        { PALETTETOKEN_GlowColorMapOff,                     "glow_color_map_off" },
        { PALETTETOKEN_FurAutoFading,                       "fur_auto_fading" },
        { PALETTETOKEN_FurHairShader,                       "fur_hair_shader" },
        { PALETTETOKEN_UseFurMaterial,                      "use_fur_material" },
        { PALETTETOKEN_FurMaterialName,                     "fur_material_name" },
        { PALETTETOKEN_FurPaletteName,                      "fur_palette_name" },
        { PALETTETOKEN_LockFresnelToReflect,                "lock_fresnel_to_reflect" },
        { PALETTETOKEN_LockRefractionRoughnessToFinish,     "lock_refraction_roughness_to_finish" },
        { PALETTETOKEN_ImageGamma,                          "image_gamma" },
        { PALETTETOKEN_ReflectColor,                        "reflect_color" },
        { PALETTETOKEN_ReflectColorMap,                     "reflect_color_map" },
        { PALETTETOKEN_ReflectColorMapOff,                  "reflect_color_map_off" },
        { PALETTETOKEN_RefractionRoughnessMap,              "refraction_roughness_map" },
        { PALETTETOKEN_RefractionRoughnessMapOff,           "refraction_roughness_map_off" },
        { PALETTETOKEN_SpecularFresnelMap,                  "specular_fresnel_map" },
        { PALETTETOKEN_SpecularFresnelMapOff,               "specular_fresnel_map_off" },
        { PALETTETOKEN_GeometryMap,                         "geometry_map"},
        { PALETTETOKEN_GeometryMapOff,                      "geometry_map_off"},
        { PALETTETOKEN_CutSectionUseMaterial,               "cut_section_use_material"},
        { PALETTETOKEN_CutSectionMaterialName,              "cut_section_material_name"},
        { PALETTETOKEN_CutSectionMaterialPalette,           "cut_section_material_palette"},
        { PALETTETOKEN_CutSectionMaterialID,                "cut_section_material_ID"},
        { PALETTETOKEN_FurBillboard,                        "fur_billboard"},
        { PALETTETOKEN_FurFrustumCulling,                   "fur_frustum_culling"},
        { PALETTETOKEN_LxoOcclusionDistance,                "lxo_occlusion_distance"},
        { PALETTETOKEN_LxoOcclusionVariance,                "lxo_occlusion_variance"},
        { PALETTETOKEN_LxoOcclusionVarianceScale,           "lxo_occlusion_variance_scale"},
        { PALETTETOKEN_LxoOcclusionSpreadAngle,             "lxo_occlusion_spread_angle"},
        { PALETTETOKEN_LxoOcclusionMaxCavityAngle ,         "lxo_occlusion_max_cavity_angle"},
        { PALETTETOKEN_LxoOcclusionType,                    "lxo_occlusion_type"},
        { PALETTETOKEN_LxoOcclusionRays,                    "lxo_occlusion_rays"},
        { PALETTETOKEN_LxoOcclusionColor1,                  "lxo_occlusion_color1"},
        { PALETTETOKEN_LxoOcclusionColor2,                  "lxo_occlusion_color2"},
        { PALETTETOKEN_LxoOcclusionAlpha1,                  "lxo_occlusion_alpha1"},
        { PALETTETOKEN_LxoOcclusionAlpha2,                  "lxo_occlusion_alpha2"},
        { PALETTETOKEN_LxoOcclusionBias,                    "lxo_occlusion_bias"},
        { PALETTETOKEN_LxoOcclusionGain,                    "lxo_occlusion_gain"},
        { PALETTETOKEN_LxoOcclusionValue1,                  "lxo_occlusion_value1"},
        { PALETTETOKEN_LxoOcclusionValue2,                  "lxo_occlusion_value2"},
        { PALETTETOKEN_LxoOcclusionSameSurface,             "lxo_occlusion_same_surface"},
        { PALETTETOKEN_ExitColor,                           "exit_color"},
        { PALETTETOKEN_BackFaceCulling,                     "back_face_culling"},
        { PALETTETOKEN_Antialiasing,                        "enable_antialiasing"},
        { PALETTETOKEN_TextureFilterType,                   "texture_filter_type"},
        { PALETTETOKEN_LowValue,                            "low_value"},
        { PALETTETOKEN_HighValue,                           "high_value"},
        { PALETTETOKEN_AntialiasStrength,                   "antialias_strength"},
        { PALETTETOKEN_MinimumSpot,                         "minimum_spot"},
        { PALETTETOKEN_UseCellColors,                       "use_cell_colors"},
        { PALETTETOKEN_Snappable,                           "snappable"},
        { PALETTETOKEN_BrickPrimaryColor,	    	    "brick_primary_color"},
        { PALETTETOKEN_BrickSecondaryColor,		    "brick_secondary_color"},
        { PALETTETOKEN_BrickPatternId,		            "brick_pattern_id"},
        { PALETTETOKEN_BrickHeaderCourseInterval,	    "brick_header_course_interval"},
        { PALETTETOKEN_BrickBondType,			    "brick_bond_type"},
        { PALETTETOKEN_BrickFlemishHeaders,		    "brick_flemish_headers"},
        { PALETTETOKEN_BrickMortarWidth,		    "brick_morter_width"},
        { PALETTETOKEN_BrickBrightnessVariation,	    "brick_brightness_variation"},
        { PALETTETOKEN_BrickHorizontalVariation,	    "brick_horizontal_variation"},
        { PALETTETOKEN_BrickNoisiness,			    "brick_noisiness"},
        { PALETTETOKEN_BrickAspectRatio,		    "brick_aspect_ratio"},
        { PALETTETOKEN_CheckerPrimaryColor,		    "checker_primary_color"},
        { PALETTETOKEN_CheckerSecondaryColor,		    "checker_secondary_color"},
        { PALETTETOKEN_CheckerScaleFactor,		    "checker_scale_factor"},
        { PALETTETOKEN_Checker3dPrimaryColor,		    "checkr3d_primary_color"},
        { PALETTETOKEN_Checker3dSecondaryColor,		    "checkr3d_secondary_color"},
        { PALETTETOKEN_Checker3dScaleFactor,		    "checkr3d_scale_factor"},
        { PALETTETOKEN_CloudsPrimaryColor,		    "clouds_primary_color"},
        { PALETTETOKEN_CloudsSecondaryColor,		    "clouds_secondary_color"},
        { PALETTETOKEN_CloudsThickness,			    "clouds_thickness"},
        { PALETTETOKEN_CloudsLOD,			    "clouds_level_of_detail"},
        { PALETTETOKEN_CloudsStepSize,			    "clouds_step_size"},
        { PALETTETOKEN_CloudsOnly,			    "clouds_only"},
        { PALETTETOKEN_FlamePrimaryColor,		    "flame_primary_color"},
        { PALETTETOKEN_FlameHeight,			    "flame_flame_height"},
        { PALETTETOKEN_FlameWidth,			    "flame_flame_width"},
        { PALETTETOKEN_FlameFlickerSpeed,		    "flame_flicker_speed"},
        { PALETTETOKEN_FlameTurbulenceScale,		    "flame_turbulence_scale"},
        { PALETTETOKEN_FlameLOD,			    "flame_level_of_detail"},
        { PALETTETOKEN_FogPrimaryColor,			    "fog_primary_color"},
        { PALETTETOKEN_FogMinimumDensity,		    "fog_minimum_density"},
        { PALETTETOKEN_FogMaximumDensity,		    "fog_maximum_density"},
        { PALETTETOKEN_FogDriftSpeed,			    "fog_drift_speed"},
        { PALETTETOKEN_FogSwirlSpeed,			    "fog_swirl_speed"},
        { PALETTETOKEN_FogThickness,			    "fog_thickness"},
        { PALETTETOKEN_FogContrast,			    "fog_contrast"},
        { PALETTETOKEN_FogLOD,				    "fog_level_of_detail"},
        { PALETTETOKEN_MarblePrimaryColor,		    "marble_primary_color"},
        { PALETTETOKEN_MarbleSecondaryColor,		    "marble_secondary_color"},
        { PALETTETOKEN_MarbleLOD,			    "marble_level_of_detail"},
        { PALETTETOKEN_MarbleVeinTightness,		    "marble_vein_tightness"},
        { PALETTETOKEN_SandPrimaryColor,		    "sand_primary_color"},
        { PALETTETOKEN_SandSecondaryColor,		    "sand_secondary_color"},
        { PALETTETOKEN_SandLOD,		                    "sand_secondary_fraction"},
        { PALETTETOKEN_StonePrimaryColor,		    "stone_primary_color"},
        { PALETTETOKEN_StoneSecondaryColor,		    "stone_secondary_color"},
        { PALETTETOKEN_StoneColors,		            "stone_stone_colors"},
        { PALETTETOKEN_StoneColorTableOffset,		    "stone_stone_color_table_offset"},
        { PALETTETOKEN_StoneMortarThickness,		    "stone_mortar_thickness"},
        { PALETTETOKEN_StoneNoisiness,		            "stone_noisiness"},
        { PALETTETOKEN_TurbulenceLOD,		            "turbulnc_level_of_detail"},
        { PALETTETOKEN_TurfPrimaryColor,		    "turf_primary_color"},
        { PALETTETOKEN_TurfSecondaryColor,		    "turf_secondary_color"},
        { PALETTETOKEN_WaterPrimaryColor,		    "water_primary_color"},
        { PALETTETOKEN_WaterRippleScale,		    "water_ripple_scale"},
        { PALETTETOKEN_WaterRippleLOD,		            "water_ripple_level_of_detail"},
        { PALETTETOKEN_WaterWaveScale,		            "water_wave_scale"},
        { PALETTETOKEN_WaterWaveLOD,		            "water_wave_level_of_detail"},
        { PALETTETOKEN_WaterWaveMinimum,		    "water_wave_minimum"},
        { PALETTETOKEN_WaterRipplesPerWave,		    "water_ripples_per_wave"},
        { PALETTETOKEN_WaterStepSize,		            "water_step_size"},
        { PALETTETOKEN_WavesPrimaryColor,		    "waves_primary_color"},
        { PALETTETOKEN_WavesSecondaryColor,		    "waves_secondary_color"},
        { PALETTETOKEN_WoodPrimaryColor,		    "wood_primary_color"},
        { PALETTETOKEN_WoodSecondaryColor,		    "wood_secondary_color"},
        { PALETTETOKEN_AdvancedWoodPrimaryColor,	    "wood01_primary_color"},
        { PALETTETOKEN_AdvancedWoodSecondaryColor,	    "wood01_secondary_color"},
        { PALETTETOKEN_BoardsPrimaryColor,	            "boards_primary_color"},
        { PALETTETOKEN_BoardsSecondaryColor,	            "boards_secondary_color"},
        { PALETTETOKEN_BoardsPatternId,	                    "boards_pattern_id"},
        { PALETTETOKEN_BoardsPerColumn,	                    "boards_boards_per_column"},
        { PALETTETOKEN_BoardsPerRow,	                    "boards_boards_per_row"},
        { PALETTETOKEN_BoardsCrackWidth,	            "boards_crack_width"},
        { PALETTETOKEN_BoardsBrightnessVariation,	    "boards_brightness_variation"},
        { PALETTETOKEN_BoardsHorizontalVariation,	    "boards_horizontal_variation"},
        { PALETTETOKEN_BoardsWoodGrainScaleFactor,	    "boards_wood_grain_scale_factor"},
        { PALETTETOKEN_GradientInput,	                    "grad1d_gradient_input"},
        { PALETTETOKEN_GradientType,	                    "grad1d_gradient_type"},
        { PALETTETOKEN_GradientRelativeCoords,	            "grad1d_relative_coords"},
        { PALETTETOKEN_GradientPatternScaleZ,	            "grad1d_pattern_scale_z"},
        { PALETTETOKEN_GradientPatternOffsetZ,	            "grad1d_pattern_offset_z"},
        { PALETTETOKEN_GradientOffsets,	                    "grad1d_u_x_offsets"},
        { PALETTETOKEN_GradientColors,	                    "grad1d_x_colors"},
	};

static Keyword s_tableKeywords[] =
    {
        { TABLETOKEN_RenderingParameters,  "rendering_parameters" },
        { TABLETOKEN_Palette,              "palette" },
        { TABLETOKEN_Assign,               "assign" },
        { TABLETOKEN_End,                  "end" },
    };

static Keyword s_highlightKeywords[] =
    {
        { HIGHLIGHTTOKEN_Plastic,     "plastic" },
        { HIGHLIGHTTOKEN_Metallic,    "metallic" },
        { HIGHLIGHTTOKEN_UseElement,  "use_element" },
        { HIGHLIGHTTOKEN_Custom,      "custom" },
    };

static Keyword s_mapLinkKeywords[] =
    {
        { MAPLINKTOKEN_Pattern,                 "pattern" },
        { MAPLINKTOKEN_Bump,                    "bump" },
        { MAPLINKTOKEN_Specular,                "specular" },
        { MAPLINKTOKEN_Reflect,                 "reflect" },
        { MAPLINKTOKEN_Transparency,            "transparency" },
        { MAPLINKTOKEN_Translucency,            "translucency" },
        { MAPLINKTOKEN_Finish,                  "finish" },
        { MAPLINKTOKEN_Diffuse,                 "diffuse" },
        { MAPLINKTOKEN_GlowAmount,              "glow_amount" },
        { MAPLINKTOKEN_ClearcoatAmount,         "clearcoat_amount" },
        { MAPLINKTOKEN_AnisotropicDirection,    "anisotropic" },
        { MAPLINKTOKEN_SpecularColor,           "specular_color" },
        { MAPLINKTOKEN_TransparentColor,        "transparent_color" },
        { MAPLINKTOKEN_TranslucencyColor,       "translucent_color" },
        { MAPLINKTOKEN_Displacement,            "displacement" },
        { MAPLINKTOKEN_Normal,                  "normal" },
        { MAPLINKTOKEN_FurLength,               "fur_length" },
        { MAPLINKTOKEN_FurDensity,              "fur_density" },
        { MAPLINKTOKEN_FurJitter,               "fur_jitter" },
        { MAPLINKTOKEN_FurFlex,                 "fur_flex" },
        { MAPLINKTOKEN_FurClumps,               "fur_clumps" },
        { MAPLINKTOKEN_FurDirection,            "fur_direction" },
        { MAPLINKTOKEN_FurVector,               "fur_vector" },
        { MAPLINKTOKEN_FurBump,                 "fur_bump" },
        { MAPLINKTOKEN_FurCurls,                "fur_curls" },
        { MAPLINKTOKEN_GlowColor,               "glow_color" },
        { MAPLINKTOKEN_ReflectColor,            "reflect_color" },
        { MAPLINKTOKEN_RefractionRoughness,     "refraction_roughness" },
        { MAPLINKTOKEN_SpecularFresnel,         "specular_fresnel" },
        { MAPLINKTOKEN_Geometry,                "geometry" },
    };

static Keyword s_layerTypeKeywords[] =
    {
        { LAYERTYPETOKEN_Image,                 "IMAGE" },
        { LAYERTYPETOKEN_Procedure,             "PROCEDURE" },
        { LAYERTYPETOKEN_Gradient,              "GRADIENT" },
        { LAYERTYPETOKEN_Normal,                "NORMAL" },
        { LAYERTYPETOKEN_Add,                   "ADD" },
        { LAYERTYPETOKEN_Subtract,              "SUBTRACT" },
        { LAYERTYPETOKEN_Alpha,                 "ALPHA" },
        { LAYERTYPETOKEN_Dissolve,              "DISSOLVE" },
        { LAYERTYPETOKEN_Atop,                  "ATOP" },
        { LAYERTYPETOKEN_In,                    "IN" },
        { LAYERTYPETOKEN_Out,                   "OUT" },
        { LAYERTYPETOKEN_Gamma,                 "GAMMA" },
        { LAYERTYPETOKEN_Tint,                  "TINT" },
        { LAYERTYPETOKEN_Brightness,            "BRIGHTNESS" },
        { LAYERTYPETOKEN_Contrast,              "CONTRAST" },
        { LAYERTYPETOKEN_GroupStart,            "GROUP_START" },
        { LAYERTYPETOKEN_GroupEnd,              "GROUP_END" },
        { LAYERTYPETOKEN_AlphaBackgroundStart,  "ALPHABACKGROUND_START" },
        { LAYERTYPETOKEN_AlphaBackgroundEnd,    "ALPHABACKGROUND_END" },
        { LAYERTYPETOKEN_LxoProcedure,          "LXOPROCEDURE" },
        { LAYERTYPETOKEN_Difference,            "DIFFERENCE" },
        { LAYERTYPETOKEN_NormalMultiply,        "NORMALMULTIPLY" },
        { LAYERTYPETOKEN_Divide,                "DIVIDE" },
        { LAYERTYPETOKEN_Multiply,              "MULTIPLY" },
        { LAYERTYPETOKEN_Screen,                "SCREEN" },
        { LAYERTYPETOKEN_Overlay,               "OVERLAY" },
        { LAYERTYPETOKEN_SoftLight,             "SOFTLIGHT" },
        { LAYERTYPETOKEN_HardLight,             "HARDLIGHT" },
        { LAYERTYPETOKEN_Darken,                "DARKEN" },
        { LAYERTYPETOKEN_Lighten,               "LIGHTEN" },
        { LAYERTYPETOKEN_ColorDodge,            "COLORDODGE" },
        { LAYERTYPETOKEN_ColorBurn,             "COLORBURN" },
        { LAYERTYPETOKEN_8119LxoProcedure,      "8119LXOPROCEDURE" },
        { LAYERTYPETOKEN_Cell,                  "CELL" },
    };

static Keyword s_lxoProcTypeKeywords[] =
    {
        { LXOPROCTYPETOKEN_None,      "None" },
        { LXOPROCTYPETOKEN_Noise,     "Noise" },
        { LXOPROCTYPETOKEN_Checker,   "Checker" },
        { LXOPROCTYPETOKEN_Grid,      "Grid" },
        { LXOPROCTYPETOKEN_Dot,       "Dots" },
        { LXOPROCTYPETOKEN_Constant,  "Constant" },
        { LXOPROCTYPETOKEN_Cellular,  "Cellular" },
        { LXOPROCTYPETOKEN_Wood,      "Wood" },
        { LXOPROCTYPETOKEN_Weave,     "Weave" },
        { LXOPROCTYPETOKEN_Ripples,   "Ripples" },
        { LXOPROCTYPETOKEN_Gradient,  "Gradient" },
        { LXOPROCTYPETOKEN_Occlusion, "Occlusion" },
   };

static Keyword s_tileKeywords[] =
    {
        { TILETOKEN_Repeat,  "repeat" },
        { TILETOKEN_Mirror,  "mirror" },
        { TILETOKEN_Decal,   "decal" },
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void fillMap (KeywordTokenMap& map, Keyword* keywords, size_t count)
    {
    for (size_t i = 0; i < count; ++i)
        map.insert (KeywordTokenPair (WString (keywords[i].m_keyword, false), keywords[i].m_token));
    }

#if !defined (DIM)
#define DIM(a) ((sizeof(a)/sizeof((a)[0])))
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialTokenManager::MaterialTokenManager ()
    {
    BeAssert (DIM (s_paletteKeywords)     == PALETTETOKEN_TokenCount);
    BeAssert (DIM (s_tableKeywords)       == TABLETOKEN_TokenCount);
    BeAssert (DIM (s_highlightKeywords)   == HIGHLIGHTTOKEN_TokenCount);
    BeAssert (DIM (s_mapLinkKeywords)     == MAPLINKTOKEN_TokenCount);
    BeAssert (DIM (s_layerTypeKeywords)   == LAYERTYPETOKEN_TokenCount);
    BeAssert (DIM (s_lxoProcTypeKeywords) == LXOPROCTYPETOKEN_TokenCount);
    BeAssert (DIM (s_tileKeywords)        == TILETOKEN_TokenCount);

    fillMap (m_paletteMap,      s_paletteKeywords,     DIM (s_paletteKeywords));
    fillMap (m_tableMap,        s_tableKeywords,       DIM (s_tableKeywords));
    fillMap (m_highlightMap,    s_highlightKeywords,   DIM (s_highlightKeywords));
    fillMap (m_mapLinkMap,      s_mapLinkKeywords,     DIM (s_mapLinkKeywords));
    fillMap (m_layerTypeMap,    s_layerTypeKeywords,   DIM (s_layerTypeKeywords));
    fillMap (m_lxoProcTypeMap,  s_lxoProcTypeKeywords, DIM (s_lxoProcTypeKeywords));
    fillMap (m_tileMap,         s_tileKeywords,        DIM (s_tileKeywords));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialTokenManager& MaterialTokenManager::GetManagerR ()
    {
    static MaterialTokenManager s_manager;
    return s_manager;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t MaterialTokenManager::ResolveToken (WStringR arg, WCharCP rawInput, KeywordTokenMap const& map)
    {
    WString input        = rawInput;
    size_t  keywordBegin = input.find_first_not_of (L" ");
    if (WString::npos == keywordBegin)
        return -1;

    size_t  keywordSize = input.find (L" ", keywordBegin);
    if (WString::npos != keywordSize)
        keywordSize -= keywordBegin;
    WString keyword     = input.substr (keywordBegin, keywordSize);

    if (WString::npos == keywordSize)
        arg = L"";
    else
        {
        arg = input.substr (keywordBegin + keywordSize);
        arg.Trim ();
        }

    KeywordTokenMap::const_iterator iter = map.find (keyword);

    return (map.end () == iter) ? -1 : iter->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
PaletteToken        MaterialTokenManager::ResolvePaletteToken (WStringR arg, WCharCP input)       { return static_cast <PaletteToken>         (ResolveToken (arg, input, m_paletteMap)); }
TableToken          MaterialTokenManager::ResolveTableToken (WStringR arg, WCharCP input)         { return static_cast <TableToken>           (ResolveToken (arg, input, m_tableMap)); }
HighlightToken      MaterialTokenManager::ResolveHighlightToken (WStringR arg, WCharCP input)     { return static_cast <HighlightToken>       (ResolveToken (arg, input, m_highlightMap)); }
MapLinkToken        MaterialTokenManager::ResolveMapLinkToken (WStringR arg, WCharCP input)       { return static_cast <MapLinkToken>         (ResolveToken (arg, input, m_mapLinkMap)); }
LayerTypeToken      MaterialTokenManager::ResolveLayerTypeToken (WStringR arg, WCharCP input)     { return static_cast <LayerTypeToken>       (ResolveToken (arg, input, m_layerTypeMap)); }
LxoProcTypeToken    MaterialTokenManager::ResolveLxoProcTypeToken (WStringR arg, WCharCP input)   { return static_cast <LxoProcTypeToken>     (ResolveToken (arg, input, m_lxoProcTypeMap)); }
TileToken           MaterialTokenManager::ResolveTileToken (WStringR arg, WCharCP input)          { return static_cast <TileToken>            (ResolveToken (arg, input, m_tileMap)); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialTokenManager::ResolveTableKeyword (WStringR keyword, TableToken token)
    {
    BeAssert (token == s_tableKeywords[token].m_token);
    keyword.AssignA (s_tableKeywords[token].m_keyword);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialTokenManager::ResolveHighlightKeyword (WStringR keyword, HighlightToken token)
    {
    BeAssert (token == s_highlightKeywords[token].m_token);
    keyword.AssignA (s_highlightKeywords[token].m_keyword);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialTokenManager::ResolveMapLinkKeyword (WStringR keyword, MapLinkToken token)
    {
    BeAssert (token == s_mapLinkKeywords[token].m_token);
    keyword.AssignA (s_mapLinkKeywords[token].m_keyword);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialTokenManager::ResolveLayerTypeKeyword (WStringR keyword, LayerTypeToken token)
    {
    BeAssert (token == s_layerTypeKeywords[token].m_token);
    keyword.AssignA (s_layerTypeKeywords[token].m_keyword);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialTokenManager::ResolveLxoProcTypeKeyword (WStringR keyword, LxoProcTypeToken token)
    {
    BeAssert (token == s_lxoProcTypeKeywords[token].m_token);
    keyword.AssignA (s_lxoProcTypeKeywords[token].m_keyword);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialTokenManager::ResolveTileKeyword (WStringR keyword, TileToken token)
    {
    BeAssert (token == s_tileKeywords[token].m_token);
    keyword.AssignA (s_tileKeywords[token].m_keyword);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialMapLayer::LayerType MaterialTokenManager::ResolveLayerTypeFromToken (LayerTypeToken token)
    {
    switch (token)
        {
        case LAYERTYPETOKEN_Image:
            return MaterialMapLayer::LAYERTYPE_Image;
        case LAYERTYPETOKEN_Procedure:
            return MaterialMapLayer::LAYERTYPE_Procedure;
        case LAYERTYPETOKEN_Gradient:
            return MaterialMapLayer::LAYERTYPE_Gradient;
        case LAYERTYPETOKEN_8119LxoProcedure:
        case LAYERTYPETOKEN_LxoProcedure:
            return MaterialMapLayer::LAYERTYPE_LxoProcedure;
        case LAYERTYPETOKEN_Cell:
            return MaterialMapLayer::LAYERTYPE_Cell;
        case LAYERTYPETOKEN_Normal:
            return MaterialMapLayer::LAYERTYPE_Normal;
        case LAYERTYPETOKEN_Add:
            return MaterialMapLayer::LAYERTYPE_Add;
        case LAYERTYPETOKEN_Subtract:
            return MaterialMapLayer::LAYERTYPE_Subtract;
        case LAYERTYPETOKEN_Alpha:
            return MaterialMapLayer::LAYERTYPE_Alpha;
        case LAYERTYPETOKEN_Gamma:
            return MaterialMapLayer::LAYERTYPE_Gamma;
        case LAYERTYPETOKEN_Tint:
            return MaterialMapLayer::LAYERTYPE_Tint;
        case LAYERTYPETOKEN_Brightness:
            return MaterialMapLayer::LAYERTYPE_Brightness;
        case LAYERTYPETOKEN_Contrast:
            return MaterialMapLayer::LAYERTYPE_Contrast;
        case LAYERTYPETOKEN_GroupStart:
            return MaterialMapLayer::LAYERTYPE_GroupStart;
        case LAYERTYPETOKEN_GroupEnd:
            return MaterialMapLayer::LAYERTYPE_GroupEnd;
        case LAYERTYPETOKEN_AlphaBackgroundStart:
            return MaterialMapLayer::LAYERTYPE_AlphaBackgroundStart;
        case LAYERTYPETOKEN_AlphaBackgroundEnd:
            return MaterialMapLayer::LAYERTYPE_AlphaBackgroundEnd;
        case LAYERTYPETOKEN_Difference:
            return MaterialMapLayer::LAYERTYPE_Difference;
        case LAYERTYPETOKEN_NormalMultiply:
            return MaterialMapLayer::LAYERTYPE_NormalMultiply;
        case LAYERTYPETOKEN_Divide:
            return MaterialMapLayer::LAYERTYPE_Divide;
        case LAYERTYPETOKEN_Multiply:
            return MaterialMapLayer::LAYERTYPE_Multiply;
        case LAYERTYPETOKEN_Screen:
            return MaterialMapLayer::LAYERTYPE_Screen;
        case LAYERTYPETOKEN_Overlay:
            return MaterialMapLayer::LAYERTYPE_Overlay;
        case LAYERTYPETOKEN_SoftLight:
            return MaterialMapLayer::LAYERTYPE_SoftLight;
        case LAYERTYPETOKEN_HardLight:
            return MaterialMapLayer::LAYERTYPE_HardLight;
        case LAYERTYPETOKEN_Darken:
            return MaterialMapLayer::LAYERTYPE_Darken;
        case LAYERTYPETOKEN_Lighten:
            return MaterialMapLayer::LAYERTYPE_Lighten;
        case LAYERTYPETOKEN_ColorDodge:
            return MaterialMapLayer::LAYERTYPE_ColorDodge;
        case LAYERTYPETOKEN_ColorBurn:
            return MaterialMapLayer::LAYERTYPE_ColorBurn;
        default:
            return MaterialMapLayer::LAYERTYPE_Image;
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
LayerTypeToken MaterialTokenManager::ResolveLayerTokenFromType (MaterialMapLayer::LayerType type)
    {
    switch (type)
        {
        case MaterialMapLayer::LAYERTYPE_Procedure:
            return LAYERTYPETOKEN_Procedure;
        case MaterialMapLayer::LAYERTYPE_Gradient:
            return LAYERTYPETOKEN_Gradient;
        case MaterialMapLayer::LAYERTYPE_LxoProcedure:
            return LAYERTYPETOKEN_LxoProcedure;
        case MaterialMapLayer::LAYERTYPE_8119LxoProcedure:
            return LAYERTYPETOKEN_8119LxoProcedure;
        case MaterialMapLayer::LAYERTYPE_Cell:
            return LAYERTYPETOKEN_Cell;
        case MaterialMapLayer::LAYERTYPE_Normal:
            return LAYERTYPETOKEN_Normal;
        case MaterialMapLayer::LAYERTYPE_Add:
            return LAYERTYPETOKEN_Add;
        case MaterialMapLayer::LAYERTYPE_Subtract:
            return LAYERTYPETOKEN_Subtract;
        case MaterialMapLayer::LAYERTYPE_Alpha:
            return LAYERTYPETOKEN_Alpha;
        case MaterialMapLayer::LAYERTYPE_Gamma:
            return LAYERTYPETOKEN_Gamma;
        case MaterialMapLayer::LAYERTYPE_Tint:
            return LAYERTYPETOKEN_Tint;
        case MaterialMapLayer::LAYERTYPE_Brightness:
            return LAYERTYPETOKEN_Brightness;
        case MaterialMapLayer::LAYERTYPE_Contrast:
            return LAYERTYPETOKEN_Contrast;
        case MaterialMapLayer::LAYERTYPE_GroupStart:
            return LAYERTYPETOKEN_GroupStart;
        case MaterialMapLayer::LAYERTYPE_GroupEnd:
            return LAYERTYPETOKEN_GroupEnd;
        case MaterialMapLayer::LAYERTYPE_AlphaBackgroundStart:
            return LAYERTYPETOKEN_AlphaBackgroundStart;
        case MaterialMapLayer::LAYERTYPE_AlphaBackgroundEnd:
            return LAYERTYPETOKEN_AlphaBackgroundEnd;
        case MaterialMapLayer::LAYERTYPE_Difference:
            return LAYERTYPETOKEN_Difference;
        case MaterialMapLayer::LAYERTYPE_NormalMultiply:
            return LAYERTYPETOKEN_NormalMultiply;
        case MaterialMapLayer::LAYERTYPE_Divide:
            return LAYERTYPETOKEN_Divide;
        case MaterialMapLayer::LAYERTYPE_Multiply:
            return LAYERTYPETOKEN_Multiply;
        case MaterialMapLayer::LAYERTYPE_Screen:
            return LAYERTYPETOKEN_Screen;
        case MaterialMapLayer::LAYERTYPE_Overlay:
            return LAYERTYPETOKEN_Overlay;
        case MaterialMapLayer::LAYERTYPE_SoftLight:
            return LAYERTYPETOKEN_SoftLight;
        case MaterialMapLayer::LAYERTYPE_HardLight:
            return LAYERTYPETOKEN_HardLight;
        case MaterialMapLayer::LAYERTYPE_Darken:
            return LAYERTYPETOKEN_Darken;
        case MaterialMapLayer::LAYERTYPE_Lighten:
            return LAYERTYPETOKEN_Lighten;
        case MaterialMapLayer::LAYERTYPE_ColorDodge:
            return LAYERTYPETOKEN_ColorDodge;
        case MaterialMapLayer::LAYERTYPE_ColorBurn:
            return LAYERTYPETOKEN_ColorBurn;
        case MaterialMapLayer::LAYERTYPE_Image:
        default:
            return LAYERTYPETOKEN_Image;
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialMap::MapType MaterialTokenManager::ResolveMapTypeFromToken (MapLinkToken token)
    {
    switch (token)
        {
        case MAPLINKTOKEN_Pattern:
            return MaterialMap::MAPTYPE_Pattern;
        case MAPLINKTOKEN_Bump:
            return MaterialMap::MAPTYPE_Bump;
        case MAPLINKTOKEN_Specular:
            return MaterialMap::MAPTYPE_Specular;
        case MAPLINKTOKEN_Reflect:
            return MaterialMap::MAPTYPE_Reflect;
        case MAPLINKTOKEN_Transparency:
            return MaterialMap::MAPTYPE_Transparency;
        case MAPLINKTOKEN_Translucency:
            return MaterialMap::MAPTYPE_Translucency;
        case MAPLINKTOKEN_Finish:
            return MaterialMap::MAPTYPE_Finish;
        case MAPLINKTOKEN_Diffuse:
            return MaterialMap::MAPTYPE_Diffuse;
        case MAPLINKTOKEN_GlowAmount:
            return MaterialMap::MAPTYPE_GlowAmount;
        case MAPLINKTOKEN_ClearcoatAmount:
            return MaterialMap::MAPTYPE_ClearcoatAmount;
        case MAPLINKTOKEN_AnisotropicDirection:
            return MaterialMap::MAPTYPE_AnisotropicDirection;
        case MAPLINKTOKEN_SpecularColor:
            return MaterialMap::MAPTYPE_SpecularColor;
        case MAPLINKTOKEN_TransparentColor:
            return MaterialMap::MAPTYPE_TransparentColor;
        case MAPLINKTOKEN_TranslucencyColor:
            return MaterialMap::MAPTYPE_TranslucencyColor;
        case MAPLINKTOKEN_Displacement:
            return MaterialMap::MAPTYPE_Displacement;
        case MAPLINKTOKEN_Normal:
            return MaterialMap::MAPTYPE_Normal;
        case MAPLINKTOKEN_FurLength:
            return MaterialMap::MAPTYPE_FurLength;
        case MAPLINKTOKEN_FurDensity:
            return MaterialMap::MAPTYPE_FurDensity;
        case MAPLINKTOKEN_FurJitter:
            return MaterialMap::MAPTYPE_FurJitter;
        case MAPLINKTOKEN_FurFlex:
            return MaterialMap::MAPTYPE_FurFlex;
        case MAPLINKTOKEN_FurClumps:
            return MaterialMap::MAPTYPE_FurClumps;
        case MAPLINKTOKEN_FurDirection:
            return MaterialMap::MAPTYPE_FurDirection;
        case MAPLINKTOKEN_FurVector:
            return MaterialMap::MAPTYPE_FurVector;
        case MAPLINKTOKEN_FurBump:
            return MaterialMap::MAPTYPE_FurBump;
        case MAPLINKTOKEN_FurCurls:
            return MaterialMap::MAPTYPE_FurCurls;
        case MAPLINKTOKEN_GlowColor:
            return MaterialMap::MAPTYPE_GlowColor;
        case MAPLINKTOKEN_ReflectColor:
            return MaterialMap::MAPTYPE_ReflectColor;
        case MAPLINKTOKEN_RefractionRoughness:
            return MaterialMap::MAPTYPE_RefractionRoughness;
        case MAPLINKTOKEN_SpecularFresnel:
            return MaterialMap::MAPTYPE_SpecularFresnel;
        default:
            return MaterialMap::MAPTYPE_None;
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
MapLinkToken MaterialTokenManager::ResolveMapTokenFromType (MaterialMap::MapType type)
    {
    switch (type)
        {
        case MaterialMap::MAPTYPE_Pattern:
            return MAPLINKTOKEN_Pattern;
        case MaterialMap::MAPTYPE_Bump:
            return MAPLINKTOKEN_Bump;
        case MaterialMap::MAPTYPE_Specular:
            return MAPLINKTOKEN_Specular;
        case MaterialMap::MAPTYPE_Reflect:
            return MAPLINKTOKEN_Reflect;
        case MaterialMap::MAPTYPE_Transparency:
            return MAPLINKTOKEN_Transparency;
        case MaterialMap::MAPTYPE_Translucency:
            return MAPLINKTOKEN_Translucency;
        case MaterialMap::MAPTYPE_Finish:
            return MAPLINKTOKEN_Finish;
        case MaterialMap::MAPTYPE_Diffuse:
            return MAPLINKTOKEN_Diffuse;
        case MaterialMap::MAPTYPE_GlowAmount:
            return MAPLINKTOKEN_GlowAmount;
        case MaterialMap::MAPTYPE_ClearcoatAmount:
            return MAPLINKTOKEN_ClearcoatAmount;
        case MaterialMap::MAPTYPE_AnisotropicDirection:
            return MAPLINKTOKEN_AnisotropicDirection;
        case MaterialMap::MAPTYPE_SpecularColor:
            return MAPLINKTOKEN_SpecularColor;
        case MaterialMap::MAPTYPE_TransparentColor:
            return MAPLINKTOKEN_TransparentColor;
        case MaterialMap::MAPTYPE_TranslucencyColor:
            return MAPLINKTOKEN_TranslucencyColor;
        case MaterialMap::MAPTYPE_Displacement:
            return MAPLINKTOKEN_Displacement;
        case MaterialMap::MAPTYPE_Normal:
            return MAPLINKTOKEN_Normal;
        case MaterialMap::MAPTYPE_FurLength:
            return MAPLINKTOKEN_FurLength;
        case MaterialMap::MAPTYPE_FurDensity:
            return MAPLINKTOKEN_FurDensity;
        case MaterialMap::MAPTYPE_FurJitter:
            return MAPLINKTOKEN_FurJitter;
        case MaterialMap::MAPTYPE_FurFlex:
            return MAPLINKTOKEN_FurFlex;
        case MaterialMap::MAPTYPE_FurClumps:
            return MAPLINKTOKEN_FurClumps;
        case MaterialMap::MAPTYPE_FurDirection:
            return MAPLINKTOKEN_FurDirection;
        case MaterialMap::MAPTYPE_FurVector:
            return MAPLINKTOKEN_FurVector;
        case MaterialMap::MAPTYPE_FurBump:
            return MAPLINKTOKEN_FurBump;
        case MaterialMap::MAPTYPE_FurCurls:
            return MAPLINKTOKEN_FurCurls;
        case MaterialMap::MAPTYPE_GlowColor:
            return MAPLINKTOKEN_GlowColor;
        case MaterialMap::MAPTYPE_ReflectColor:
            return MAPLINKTOKEN_ReflectColor;
        case MaterialMap::MAPTYPE_RefractionRoughness:
            return MAPLINKTOKEN_RefractionRoughness;
        case MaterialMap::MAPTYPE_SpecularFresnel:
            return MAPLINKTOKEN_SpecularFresnel;
        default:
            return MAPLINKTOKEN_Invalid;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialTokenManager::ResolvePaletteKeyword (Utf8StringR keyword, PaletteToken token, MaterialVersion version)
    {
    // Fix due to a hole in the table where token != s_paletteKeyWords[token].m_token which meant
    // code using only s_paletteKeyWords[token] would output the wrong string... stuck with for
    // the forseeable future since there are a sizable numbers of serialized materials with their
    // keywords shifted.
    if (MATERIALVERSION_Invalid == version && token > PALETTETOKEN_MaterialVersion)
        token = static_cast <PaletteToken> (static_cast<int32_t> (token) + 1);

    BeAssert (token == s_paletteKeywords[token].m_token);
    keyword.assign (s_paletteKeywords[token].m_keyword);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialTokenManager::ResolvePaletteKeyword (WStringR keyword, PaletteToken token, MaterialVersion version)
    {
    // Fix due to a hole in the table where token != s_paletteKeyWords[token].m_token which meant
    // code using only s_paletteKeyWords[token] would output the wrong string... stuck with for
    // the forseeable future since there are a sizable numbers of serialized materials with their
    // keywords shifted.
    if (MATERIALVERSION_Invalid == version && token > PALETTETOKEN_MaterialVersion)
        token = static_cast <PaletteToken> (static_cast<int32_t> (token) + 1);

    BeAssert (token == s_paletteKeywords[token].m_token);
    keyword.AssignA (s_paletteKeywords[token].m_keyword);
    }
