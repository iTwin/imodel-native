/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <json/value.h>
#include "Render.h"

#define RENDER_MATERIAL_Type                                        "Type"
#define RENDER_MATERIAL_PatternFlags                                "PatternFlags"
#define RENDER_MATERIAL_NormalFlags                                 "NormalFlags"
#define RENDER_MATERIAL_NormalScale                                 "pbr_normal"
#define RENDER_MATERIAL_Diffuse                                     "diffuse"
#define RENDER_MATERIAL_Reflect                                     "reflect"
#define RENDER_MATERIAL_Transmit                                    "transmit"
#define RENDER_MATERIAL_Refract                                     "refract"
#define RENDER_MATERIAL_Specular                                    "specular"
#define RENDER_MATERIAL_Finish                                      "finish"
#define RENDER_MATERIAL_Map                                         "Map"
#define RENDER_MATERIAL_Flags                                       "Flags"
#define RENDER_MATERIAL_Ambient                                     "ambient"
#define RENDER_MATERIAL_Glow                                        "glow"

#define RENDER_MATERIAL_Color                                       "color"
#define RENDER_MATERIAL_SpecularColor                               "specular_color"
#define RENDER_MATERIAL_ReflectColor                                "reflect_color"
#define RENDER_MATERIAL_GlowColor                                   "glow_color"
#define RENDER_MATERIAL_TransparentColor                            "transparent_color"
#define RENDER_MATERIAL_TranslucencyColor                           "translucent_color"

#define RENDER_MATERIAL_FlagNoShadows                               "NoShadows"
#define RENDER_MATERIAL_FlagHasBaseColor                            "HasBaseColor"
#define RENDER_MATERIAL_FlagHasSpecularColor                        "HasSpecularColor"
#define RENDER_MATERIAL_FlagHasFinish                               "HasFinish"
#define RENDER_MATERIAL_FlagHasReflect                              "HasReflect"
#define RENDER_MATERIAL_FlagHasTransmit                             "HasTransmit"
#define RENDER_MATERIAL_FlagHasDiffuse                              "HasDiffuse"
#define RENDER_MATERIAL_FlagHasRefract                              "HasRefract"
#define RENDER_MATERIAL_FlagHasSpecular                             "HasSpecular"
#define RENDER_MATERIAL_FlagLockSpecularAndReflect                  "LockSpecularAndReflect"
#define RENDER_MATERIAL_FlagLockEfficiency                          "LockEfficiency"
#define RENDER_MATERIAL_FlagLockSpecularAndBase                     "LockSpecularAndBase"
#define RENDER_MATERIAL_FlagLockFinishToSpecular                    "LockFinishToSpecular"
#define RENDER_MATERIAL_FlagCustomSpecular                          "CustomSpecular"
#define RENDER_MATERIAL_FlagLinkedToLxp                             "LinkedToLxp"
#define RENDER_MATERIAL_FlagInvisible                               "Invisible"
#define RENDER_MATERIAL_FlagHasTransmitColor                        "HasTransmitColor"
#define RENDER_MATERIAL_FlagHasTranslucencyColor                    "HasTranslucencyColor"
#define RENDER_MATERIAL_FlagLockFresnelToReflect                    "LockFresnelToReflect"
#define RENDER_MATERIAL_FlagLockRefractionRoughnessToFinish         "LockRefractionRoughnessToFinish"
#define RENDER_MATERIAL_FlagHasGlowColor                            "HasGlowColor"
#define RENDER_MATERIAL_FlagHasReflectColor                         "HasReflectColor"
#define RENDER_MATERIAL_FlagHasExitColor                            "HasExitColor"

// Pattern (Texture maps).
#define RENDER_MATERIAL_Pattern                                     "pattern"
#define RENDER_MATERIAL_PatternOff                                  "pattern_off"
#define RENDER_MATERIAL_PatternSize                                 "pattern_size"
#define RENDER_MATERIAL_PatternAngle                                "pattern_angle"
#define RENDER_MATERIAL_PatternScale                                "pattern_scale"
#define RENDER_MATERIAL_PatternOffset                               "pattern_offset"
#define RENDER_MATERIAL_PatternFlipV                                "pattern_flip"
#define RENDER_MATERIAL_PatternLockSize                             "pattern_lock_size"
#define RENDER_MATERIAL_PatternWeight                               "pattern_weight"
#define RENDER_MATERIAL_PatternMapping                              "pattern_mapping"
#define RENDER_MATERIAL_PatternScaleMode                            "pattern_scalemode"
#define RENDER_MATERIAL_PatternTileSection                          "pattern_tilesection"
#define RENDER_MATERIAL_PatternFlipU                                "pattern_u_flip"
#define RENDER_MATERIAL_PatternTileDecalU                           "pattern_tile_decal_u"
#define RENDER_MATERIAL_PatternTileDecalV                           "pattern_tile_decal_v"
#define RENDER_MATERIAL_PatternTileMirrorU                          "pattern_tile_mirror_u"
#define RENDER_MATERIAL_PatternTileMirrorV                          "pattern_tile_mirror_v"
#define RENDER_MATERIAL_PatternLockProjectionScale                  "pattern_lock_projection_scale"
#define RENDER_MATERIAL_PatternCylCapped                            "pattern_cylindrical_projection_capped"
#define RENDER_MATERIAL_Antialiasing                                "enable_antialiasing"
#define RENDER_MATERIAL_FileName                                    "Filename"

// Geometry maps..
#define RENDER_MATERIAL_UseCellColors                               "use_cell_colors"
#define RENDER_MATERIAL_Snappable                                   "snappable"

// Map types.
#define RENDER_MATERIAL_MAP_None                                   "None"
#define RENDER_MATERIAL_MAP_Pattern                                "Pattern"
#define RENDER_MATERIAL_MAP_Normal                                 "Normal"
#define RENDER_MATERIAL_MAP_Bump                                   "Bump"
#define RENDER_MATERIAL_MAP_Specular                               "Specular"
#define RENDER_MATERIAL_MAP_Reflect                                "Reflect"
#define RENDER_MATERIAL_MAP_Transparency                           "Transparency"
#define RENDER_MATERIAL_MAP_Translucency                           "Translucency"
#define RENDER_MATERIAL_MAP_Finish                                 "Finish"
#define RENDER_MATERIAL_MAP_Diffuse                                "Diffuse"
#define RENDER_MATERIAL_MAP_GlowAmount                             "GlowAmount"
#define RENDER_MATERIAL_MAP_Geometry                               "Geometry"
#define RENDER_MATERIAL_GeometryMap                                 "geometry_map"

// DgnDB Only...
#define RENDER_MATERIAL_TextureId                                  "TextureId"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Optional parameters used when computing UV params for elevation drape.
// @bsistruct
//=======================================================================================
struct ElevationDrapeParams
{
    Transform m_transform;
    uint32_t m_width;
    uint32_t m_height;

    ElevationDrapeParams() = default; // Uninitialized!
    ElevationDrapeParams(TransformCR transform, uint32_t width = 0, uint32_t height = 0) : m_transform(transform), m_width(width), m_height(height) { }

    //! Params are computed in world space, which may result in very large coordinates.
    //! Translate coordinates to be close to zero.
    //! Requires texture dimensions to be specified (non-zero).
    void Normalize(DPoint2dP params, size_t numParams) const;
};

//=======================================================================================
//! JSON representation of a RenderMaterial.
//! See RENDER_MATERIAL_* constants for a list of the known property names.
//! Only a very small subset of those properties are actually used for rendering in the iModel.js
//! and DgnClientFx display systems. They are described below.
//!
//! An RGB color is defined in JSON as an array of three floating point values in [0..1].
//!
//! The diffuse color can be overridden by setting:
//!     RENDER_MATERIAL_FlagHasBaseColor=true; and
//!     RENDER_MATERIAL_Color=RGB
//! The specular color can be overridden by setting:
//!     RENDER_MATERIAL_FlagHasSpecularColor=true; and
//!     RENDER_MATERIAL_SpecularColor=RGB
//! The specular exponent can be specified as a floating point value > 0.0 by setting:
//!     RENDER_MATERIAL_FlagHasFinish=true; and
//!     RENDER_MATERIAL_Finish=exponent
//! A material *always* overrides element transparency as a value from 0 (fully opaque) to 1 (fully transparent).
//!     If RENDER_MATERIAL_FlagHasTransmit=true, then RENDER_MATERIAL_Transmit holds that transparency value.
//!     Otherwise, the transparency value is 0.0.
//!     If the material has a pattern map, the alpha of the sampled texture will be multiplied by the material's alpha (inverse of its transparency).
//!     ###TODO Revit wants to be able to *not* override element transparency. This should be doable.
//! The diffuse lighting weight can be specified as a value in [0..1] by setting:
//!     RENDER_MATERIAL_FlagHasDiffuse=true; and
//!     RENDER_MATERIAL_Diffuse=weight
//! The specular weight can be specified as a value in [0..1] by setting:
//!     RENDER_MATERIAL_FlagHasSpecular=true; and
//!     RENDER_MATERIAL_Specular=weight
//!     (If RENDER_MATERIAL_FlagHasSpecular=false, the weight is 0.0).
//! The pattern map can be specified by setting json[RENDER_MATERIAL_Map][RENDER_MATERIAL_MAP_Pattern] to the JSON representation of a pattern map.
// @bsiclass
//=======================================================================================
struct RenderingAsset : BeJsValue
{
public:
    struct Default
    {
        static double const Finish() {return .05;}
        static double const Specular() {return .05;}
        static double const Diffuse() {return .5;}
        static double const Reflect() {return 0.;}
        static double const Transmit() {return 0.;}
        static double const Glow() {return 0.;}
        static double const Refract() {return 0.;}
        static double const Ambient() {return 0.;}
        static bool const HasBaseColor() {return false;}
        static bool const HasSpecularColor() {return false;}
        static bool const HasFinish() {return false;}
        static bool const NoShadows() {return false;}
    };

    //=======================================================================================
    // Materials can have textures mapped into them
    // @bsiclass
    //=======================================================================================
    struct TextureMap
    {
        enum class Type
        {
            None                    = 0,
            Pattern                 = 1,
            Bump                    = 2,
            Specular                = 3,
            Reflect                 = 4,
            Transparency            = 5,
            Translucency            = 6,
            Finish                  = 7,
            Diffuse                 = 8,
            GlowAmount              = 9,
            ClearcoatAmount         = 10,
            AnisotropicDirection    = 11,
            SpecularColor           = 12,
            TransparentColor        = 13,
            TranslucencyColor       = 14,
            Displacement            = 15,
            Normal                  = 16,
            FurLength               = 17,
            FurDensity              = 18,
            FurJitter               = 19,
            FurFlex                 = 20,
            FurClumps               = 21,
            FurDirection            = 22,
            FurVector               = 23,
            FurBump                 = 24,
            FurCurls                = 25,
            GlowColor               = 26,
            ReflectColor            = 27,
            RefractionRoughness     = 28,
            SpecularFresnel         = 29,
            Geometry                = 30,
        };

        enum class Units
        {
            Relative               = 0,
            Meters                 = 3,
            Millimeters            = 4,
            Feet                   = 5,
            Inches                 = 6,
        };

        // Flags for RENDER_MATERIAL_NormalFlags
        enum NormalFlags : uint32_t
        {
            InvertGreen = 1 << 0,
        };

        BeJsConst m_value;
        Type m_type;

        bool IsValid() const {return !m_value.isNull();}
        DGNPLATFORM_EXPORT Render::TextureMapping::Trans2x3 GetTransform() const;
        DGNPLATFORM_EXPORT DPoint2d GetScale() const;
        DGNPLATFORM_EXPORT DPoint2d GetOffset() const;
        DGNPLATFORM_EXPORT Units GetUnits() const;
        DGNPLATFORM_EXPORT Render::TextureMapping::Mode GetMode() const;
        DGNPLATFORM_EXPORT Render::TextureMapping::Params GetTextureMapParams() const;
        DGNPLATFORM_EXPORT DgnTextureId GetTextureId() const;
        DGNPLATFORM_EXPORT bool IsPatternEnabled() const;
        DGNPLATFORM_EXPORT double GetPatternWeight() const;
        Type GetType() const {return m_type;}
        DGNPLATFORM_EXPORT static double GetUnitScale(Units units);
        DgnTextureId Relocate(DgnImportContext& context);
        double GetDouble(Utf8CP name, double defaultVal) const {return m_value[name].asDouble(defaultVal);}
        bool GetBool(Utf8CP name, bool defaultVal) const {return m_value[name].asBool(defaultVal);}
        uint32_t GetUint32(Utf8CP name, uint32_t defaultVal=0) const {return m_value[name].asUInt(defaultVal);}
        BeJsConst GetValue() const {return m_value;}

        DGNPLATFORM_EXPORT BentleyStatus ComputeUVParams (bvector<DPoint2d>& params, PolyfaceVisitorCR visitor, ElevationDrapeParamsCP drapeParams = nullptr) const;
        TextureMap(BeJsConst val, Type type) : m_value(val), m_type(type) {}
    }; // TextureMap

    DGNPLATFORM_EXPORT BentleyStatus Relocate(DgnImportContext& context);
    RenderingAsset(BeJsConst other) : BeJsValue(*((BeJsValue*)&other)) {}
    double GetDouble(Utf8CP name, double defaultVal) const {return GetValue(name).asDouble(defaultVal);}
    void SetDouble(Utf8CP name, double val) {GetValueR(name)=val;}
    bool GetBool(Utf8CP name, bool defaultVal) const {return GetValue(name).asBool(defaultVal);}
    void SetBool(Utf8CP name, bool val) {GetValueR(name)=val;}
    DGNPLATFORM_EXPORT RgbFactor GetColor(Utf8CP name) const;
    DGNPLATFORM_EXPORT void SetColor(Utf8CP name, RgbFactor);
    BeJsConst GetValue(Utf8CP name) const {return (*this)[name];}
    BeJsValue GetValueR(Utf8CP name) {return (*this)[name];}
    bool IsValid() const {return !isNull();}

    DGNPLATFORM_EXPORT TextureMap GetTextureMap(TextureMap::Type type, Utf8CP typeName) const;
    TextureMap GetPatternMap() const { return GetTextureMap(TextureMap::Type::Pattern, RENDER_MATERIAL_MAP_Pattern); }
    TextureMap GetNormalMap() const { return GetTextureMap(TextureMap::Type::Normal, RENDER_MATERIAL_MAP_Normal); }
    double GetNormalScale() const { return GetDouble(RENDER_MATERIAL_NormalScale, 1.0); }

    //=======================================================================================
    //! Helper class for constructing a Render::TextureMapping::Trans2x3 from scratch or
    //! from a RenderingAsset::TextureMap.
    // @bsistruct
    //=======================================================================================
    struct Trans2x3Builder
    {
        using Units = RenderingAsset::TextureMap::Units;
        using Mode = Render::TextureMapping::Mode;

        DPoint2d    m_scale;
        DPoint2d    m_offset;
        Angle       m_angle;
        Units       m_units;
        Mode        m_mode;

        DGNPLATFORM_EXPORT explicit Trans2x3Builder(Mode mode = Mode::Parametric, Units units = Units::Meters);
        DGNPLATFORM_EXPORT explicit Trans2x3Builder(RenderingAsset::TextureMap const&);

        void FlipU() { m_scale.x *= -1.0; }
        void FlipV() { m_scale.y *= -1.0; }

        DGNPLATFORM_EXPORT Render::TextureMapping::Trans2x3 Build() const;
    };
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
