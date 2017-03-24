/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/Lighting.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnElement.h"

#define BEGIN_LIGHTING_NAMESPACE BEGIN_BENTLEY_DGN_NAMESPACE namespace Lighting {
#define END_LIGHTING_NAMESPACE   } END_BENTLEY_DGN_NAMESPACE
#define USING_NAMESPACE_LIGHTING using namespace BentleyApi::Dgn::Lighting;

BEGIN_LIGHTING_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(Location);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Parameters);
DEFINE_REF_COUNTED_PTR(Location);

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   03/17
//=======================================================================================
enum class LightType
{
    Invalid = 0,
    Solar = 1,      //!< Sunlight
    Ambient = 2,    //!< ambient light
    Flash = 3,      //!< flash bulb at camera
    Portrait = 4, //!< over the sholder (left and right)
    Point = 5, //<! non directional point light source
    Spot = 6,
    Area = 7,
    Distant = 8,
    SkyOpening = 9,
};

//=======================================================================================
// Json parameters that define how a light affects the rendering of a scene.
// @bsiclass                                                    Keith.Bentley   03/17
//=======================================================================================
struct Parameters : Json::Value
{
    struct Spot
    {
        AngleInDegrees m_inner;
        AngleInDegrees m_outer;
        Spot(AngleInDegrees inner, AngleInDegrees outer) : m_inner(inner), m_outer(outer) {}
    };

private:
    static constexpr Utf8CP str_Type() {return "type";}
    static constexpr Utf8CP str_Intensity() {return "intensity";}
    static constexpr Utf8CP str_Color() {return "color";}
    static constexpr Utf8CP str_Intensity2() {return "intensity2";}
    static constexpr Utf8CP str_Color2() {return "color2";}
    static constexpr Utf8CP str_Kelvins() {return "kelvin";} 
    static constexpr Utf8CP str_Shadows() {return "shadows";}
    static constexpr Utf8CP str_Bulbs() {return "bulbs";}
    static constexpr Utf8CP str_Lumens() {return "lumens";}
    static constexpr Utf8CP str_Spot() {return "spot";}
    static constexpr Utf8CP str_Inner() {return "inner";}
    static constexpr Utf8CP str_Outer() {return "outer";}

    JsonValueCR Value(Utf8CP name) const {return (*this)[name];}
public:

    LightType GetType() const {return (LightType) Value(str_Type()).asInt((int) LightType::Invalid);}
    void SetType(LightType type) {SetOrRemoveInt(str_Type(), (int) type, (int) LightType::Invalid);}

    double GetIntensity() const {return Value(str_Intensity()).asDouble(1.0);}
    void SetIntensity(double intensity) {SetOrRemoveDouble(str_Intensity(), intensity, 1.0);}

    ColorDef GetColor() const {return ColorDef(Value(str_Color()).asInt(ColorDef::White().GetValue()));}
    void SetColor(ColorDef color) {SetOrRemoveInt(str_Color(), color.GetValue(), ColorDef::White().GetValue());}

    double GetIntensity2() const {return Value(str_Intensity2()).asDouble(1.0);}
    void SetIntensity2(double intensity) {SetOrRemoveDouble(str_Intensity2(), intensity, 1.0);}

    ColorDef GetColor2() const {return ColorDef(Value(str_Color2()).asInt(ColorDef::White().GetValue()));}
    void SetColor2(ColorDef color) {SetOrRemoveInt(str_Color2(), color.GetValue(), ColorDef::White().GetValue());}

    double GetLumens() const {return Value(str_Lumens()).asDouble(0.0);}
    void SetLumens(double lumens) {SetOrRemoveDouble(str_Lumens(), lumens, 0.0);}

    uint32_t GetKelvins() const {return Value(str_Kelvins()).asUInt();} 
    void SetKelvins(uint32_t kelvins) {SetOrRemoveUInt(str_Kelvins(), kelvins, 0);}

    uint32_t GetShadowSamples() const {return Value(str_Shadows()).asUInt();}
    void SetShadowSamples(uint32_t val) {SetOrRemoveUInt(str_Shadows(), val, 0);}

    uint32_t GetBulbs() const {return Value(str_Bulbs()).asUInt(1);}
    void SetBulbs(uint32_t bulbs) {SetOrRemoveUInt(str_Bulbs(), bulbs, 1);}

    Spot GetSpot() const {auto& spot=Value(str_Spot()); return Spot(AngleInDegrees::FromDegrees(spot[str_Inner()].asDouble()), AngleInDegrees::FromDegrees(spot[str_Outer()].asDouble()));}
    void SetSpot(Spot val) {auto& spot=(*this)[str_Spot()]; spot[str_Inner()] = val.m_inner.Degrees(); spot[str_Outer()] = val.m_outer.Degrees();}

    bool IsValid() const {return GetType() != LightType::Invalid;}
    bool IsVisible() const {return IsValid() && GetIntensity() > 0.0;}
    Parameters(LightType type = LightType::Invalid) {SetType(type);}
};

//=======================================================================================
//! Holds the location of a light.
//=======================================================================================
struct Location : SpatialLocationElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_LightLocation, SpatialLocationElement);
public:

protected:
    static constexpr Utf8CP str_Params() {return "light";}
    
public:
    explicit Location(CreateParams const& params) : T_Super(params) {}
    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_LightLocation));}//!< @private
    ParametersCR GetParameters() const {return (ParametersCR) m_jsonProperties[str_Params()];}
    void SetParameters(ParametersCR val) {m_jsonProperties[Json::StaticString(str_Params())] = val;}
};

namespace Handlers
{
    //=======================================================================================
    //! The handler for light location elements.
    // @bsiclass                                                    Keith.Bentley   03/17
    //=======================================================================================
    struct LightLoc : dgn_ElementHandler::SpatialLocation
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_LightLocation, Location, LightLoc, SpatialLocation, DGNPLATFORM_EXPORT);
    };
}

END_LIGHTING_NAMESPACE

