/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "DgnElement.h"

#define BEGIN_LIGHTING_NAMESPACE BEGIN_BENTLEY_DGN_NAMESPACE namespace Lighting {
#define END_LIGHTING_NAMESPACE   } END_BENTLEY_DGN_NAMESPACE
#define USING_NAMESPACE_LIGHTING using namespace BentleyApi::Dgn::Lighting;

BEGIN_LIGHTING_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(Location);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Parameters);
DEFINE_REF_COUNTED_PTR(Location);

//=======================================================================================
// @bsiclass
//=======================================================================================
enum class LightType
{
    Invalid = 0,
    Solar = 1, //!< Sunlight
    Ambient = 2, //!< ambient light
    Flash = 3,   //!< flash bulb at camera
    Portrait = 4, //!< over the shoulder (left and right)
    Point = 5, //!< non directional point light source
    Spot = 6,
    Area = 7,
    Distant = 8,
    SkyOpening = 9,
};

//=======================================================================================
//! Json parameters that define how a light affects the rendering of a scene.
// @bsiclass
//=======================================================================================
struct Parameters : BeJsValue
{
    struct Spot
    {
        AngleInDegrees m_inner;
        AngleInDegrees m_outer;
        Spot(AngleInDegrees inner, AngleInDegrees outer) : m_inner(inner), m_outer(outer) {}
    };

private:
    BeJsConst Value(Utf8CP name) const {return (*this)[name];}

public:
    BE_JSON_NAME(type)          // the type of light from LightType enum
    BE_JSON_NAME(intensity)     // intensity of the light
    BE_JSON_NAME(color)         // color of the light. ColorDef as integer
    BE_JSON_NAME(intensity2)    // for portrait lights, intensity of the "over the left shoulder" light (intensity is the right shoulder light).
    BE_JSON_NAME(color2)        // for left portrait light
    BE_JSON_NAME(kelvin)        // color temperature, in kelvins. Note that color and kelvins are not independent. Useful for UI, I guess?
    BE_JSON_NAME(shadows)       // the number of shadow samples
    BE_JSON_NAME(bulbs)         // number of bulbs
    BE_JSON_NAME(lumens)
    BE_JSON_NAME(spot)          // container for spotlight parameters
    BE_JSON_NAME(inner)         //  spotlight inner angle, in degrees
    BE_JSON_NAME(outer)         //  spotlight outer angle, in degrees

    LightType GetType() const {return (LightType) Value(json_type()).asInt((int) LightType::Invalid);}
    void SetType(LightType type) {SetOrRemoveInt(json_type(), (int) type, (int) LightType::Invalid);}

    double GetIntensity() const {return Value(json_intensity()).asDouble(1.0);}
    void SetIntensity(double intensity) {SetOrRemoveDouble(json_intensity(), intensity, 1.0);}

    ColorDef GetColor() const {return ColorDef(Value(json_color()).asInt(ColorDef::White().GetValue()));}
    void SetColor(ColorDef color) {SetOrRemoveInt(json_color(), color.GetValue(), ColorDef::White().GetValue());}

    double GetIntensity2() const {return Value(json_intensity2()).asDouble(1.0);}
    void SetIntensity2(double intensity) {SetOrRemoveDouble(json_intensity2(), intensity, 1.0);}

    ColorDef GetColor2() const {return ColorDef(Value(json_color2()).asInt(ColorDef::White().GetValue()));}
    void SetColor2(ColorDef color) {SetOrRemoveInt(json_color2(), color.GetValue(), ColorDef::White().GetValue());}

    double GetLumens() const {return Value(json_lumens()).asDouble(0.0);}
    void SetLumens(double lumens) {SetOrRemoveDouble(json_lumens(), lumens, 0.0);}

    uint32_t GetKelvin() const {return Value(json_kelvin()).asUInt();}
    void SetKelvin(uint32_t kelvins) {SetOrRemoveUInt(json_kelvin(), kelvins, 0);}

    uint32_t GetShadowSamples() const {return Value(json_shadows()).asUInt();}
    void SetShadowSamples(uint32_t val) {SetOrRemoveUInt(json_shadows(), val, 0);}

    uint32_t GetBulbCount() const {return Value(json_bulbs()).asUInt(1);}
    void SetBulbCount(uint32_t bulbs) {SetOrRemoveUInt(json_bulbs(), bulbs, 1);}

    Spot GetSpot() const {auto spot=Value(json_spot()); return Spot(AngleInDegrees::FromDegrees(spot[json_inner()].asDouble()), AngleInDegrees::FromDegrees(spot[json_outer()].asDouble()));}
    void SetSpot(Spot val) {auto spot=(*this)[json_spot()]; spot[json_inner()] = val.m_inner.Degrees(); spot[json_outer()] = val.m_outer.Degrees();}

    bool IsValid() const {return GetType() != LightType::Invalid;}
    bool IsVisible() const {return IsValid() && GetIntensity() > 0.0;}

    Parameters(BeJsValue const& other, LightType type=LightType::Invalid) : BeJsValue(other) {SetType(type);}
};

//=======================================================================================
//! Holds the location of a light.
//=======================================================================================
struct Location : SpatialLocationElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_LightLocation, SpatialLocationElement);
public:

    BE_JSON_NAME(light)
    explicit Location(CreateParams const& params) : T_Super(params) {}
    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_LightLocation));} //!< @private
    BeJsConst GetParameters() const {return GetJsonProperties(json_light());}
    BeJsValue GetParametersR()  {return GetJsonPropertiesR(json_light());}
    void SetParameters(ParametersCR val) {GetParametersR().From(val);}
};

namespace Handlers
{
    //=======================================================================================
    //! The handler for light location elements.
    // @bsiclass
    //=======================================================================================
    struct LightLoc : dgn_ElementHandler::SpatialLocation
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_LightLocation, Location, LightLoc, SpatialLocation, DGNPLATFORM_EXPORT);
    };
}

END_LIGHTING_NAMESPACE

