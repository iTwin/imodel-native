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
    Distant = 8
};

//=======================================================================================
// Json parameters that define how a light affects the rendering of a scene.
// @bsiclass                                                    Keith.Bentley   03/17
//=======================================================================================
struct Parameters : Json::Value
{
private:
    static constexpr Utf8CP str_Type() {return "type";}
    static constexpr Utf8CP str_Intensity() {return "intensity";}
    static constexpr Utf8CP str_Color() {return "color";}
    static constexpr Utf8CP str_Intensity2() {return "intensity2";}
    static constexpr Utf8CP str_Color2() {return "color2";}
    JsonValueCR Value(Utf8CP name) const {return (*this)[name];}
    JsonValueR ValueR(Utf8CP name) {return (*this)[Json::StaticString(name)];}

public:
    LightType GetType() const {return (LightType) Value(str_Type()).asInt((int) LightType::Invalid);}
    void SetType(LightType type) {ValueR(str_Type()) = (int) type;}
    double GetIntensity() const {return Value(str_Intensity()).asDouble(0.0);}
    void SetIntensity(double intensity) {ValueR(str_Intensity()) = intensity;}
    ColorDef GetColor() const {return ColorDef(Value(str_Color()).asInt(ColorDef::White().GetValue()));}
    void SetColor(ColorDef color) {ValueR(str_Color()) = color.GetValue();}
    double GetIntensity2() const {return Value(str_Intensity2()).asDouble(0.0);}
    void SetIntensity2(double intensity) {ValueR(str_Intensity2()) = intensity;}
    ColorDef GetColor2() const {return ColorDef(Value(str_Color2()).asInt(ColorDef::White().GetValue()));}
    void SetColor2(ColorDef color) {ValueR(str_Color2()) = color.GetValue();}

    bool IsValid() const {return GetType() != LightType::Invalid;}
    bool IsOn() const {return IsValid() && GetIntensity() > 0.0;}
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
    static constexpr Utf8CP str_Light() {return "light";}
    JsonValueCR GetParams() const {return m_jsonProperties[str_Light()];}
    JsonValueR GetParams() {return m_jsonProperties[Json::StaticString(str_Light())];}
    
public:
    explicit Location(CreateParams const& params) : T_Super(params) {}
    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_LightLocation));}//!< @private
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

