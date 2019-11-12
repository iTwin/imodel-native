/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------
    Header file contains Gi classes shared by DwgGiDrawable's and DwgDbObject's etc.
---------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#ifdef DWGTOOLKIT_OpenDwg
#include    <Teigha/Kernel/Include/Gi/GiDrawable.h>
#include    <Teigha/Kernel/Include/Gi/GiVisualStyle.h>
#include    <Teigha/Kernel/Include/Gi/GiMaterial.h>
#include    <Teigha/Kernel/Include/Gi/GiLightTraits.h>
#elif DWGTOOLKIT_RealDwg
#include    <RealDwg/Base/acgi.h>
#include    <RealDwg/Base/AcGiVisualStyle.h>
#include    <RealDwg/Base/AcGiMaterial.h>
#include    <RealDwg/Base/AcGiLightTraits.h>
#else
    #error  "Must define DWGTOOLKIT!!!"
#endif


// declare common methods for DwgGiXxxxx classes which are Rx class in both Teigha and RealDWG!
#define DWGGI_DECLARE_RX_MEMBERS(_classSuffix_)                         \
    DEFINE_T_SUPER(DWGROOT_SUPER_CONSTRUCTOR(Gi##_classSuffix_##))      \
    DWGRX_DECLARE_MEMBERS_EXPIMP(DwgGi##_classSuffix_##)

#ifdef DWGTOOLKIT_OpenDwg
// declare common methods for DwgGiXxxxx classes - these are base classes, rasther than Rx classes, in Teigha!
#define DWGGI_DECLARE_BASE_MEMBERS(_classSuffix_)                       \
    DEFINE_T_SUPER(DWGROOT_SUPER_CONSTRUCTOR(Gi##_classSuffix_##))

#elif DWGTOOLKIT_RealDwg
// declare common methods for DwgGiXxxxx classes - these are Rx class in RealDWG!
#define DWGGI_DECLARE_BASE_MEMBERS(_classSuffix_)                       \
    DWGGI_DECLARE_RX_MEMBERS(_classSuffix_)
#endif


BEGIN_DWGDB_NAMESPACE


namespace DwgGiVisualStyleProperties
{
enum class Property
    {
    InvalidProperty = -1,
    FaceLightingModel,
    FaceLightingQuality,
    FaceColorMode,
    FaceModifiers,
    FaceOpacity,
    FaceSpecular,
    FaceMonoColor,
    EdgeModel,    
    EdgeStyles,    
    EdgeIntersectionColor,  
    EdgeObscuredColor,    
    EdgeObscuredLinePattern,  
    EdgeIntersectionLinePattern,
    EdgeCreaseAngle,
    EdgeModifiers,
    EdgeColor,
    EdgeOpacity,
    EdgeWidth,
    EdgeOverhang,
    EdgeJitterAmount,
    EdgeSilhouetteColor,
    EdgeSilhouetteWidth,
    EdgeHaloGap,
    EdgeIsolines,
    EdgeHidePrecision,
    DisplayStyles,
    DisplayBrightness,
    DisplayShadowType,
    UseDrawOrder,
    ViewportTransparency,
    LightingEnabled,
    PosterizeEffect,
    MonoEffect,
    BlurEffect,
    PencilEffect,
    BloomEffect,
    PastelEffect,
    BlurAmount,
    PencilAngle,
    PencilScale,
    PencilPattern,
    PencilColor,
    BloomThreshold,
    BloomRadius,
    TintColor,
    FaceAdjustment,
    PostContrast,
    PostBrightness,
    PostPower,
    TintEffect,
    BloomIntensity,
    Color,
    Transparency,
    EdgeWiggleAmount,
    EdgeTexturePath,
    DepthOfField,
    FocusDistance,
    FocusWidth,
    PropertyCount,
    PropertyCountPre2013 = UseDrawOrder,
    };  // Property

enum class FaceLightingModel
    {
    Constant,
    Phong,
    Gooch,
    Zebra
    };  // FaceLightingModel

enum class FaceColorMode
    {
    NoColorMode,
    ObjectColor,
    BackgroundColor,
    Mono,
    Tint,
    Desaturate
    };  // FaceColorMode

enum class FaceModifiers
    {
    NoFaceModifiers    = 0,
    FaceOpacityFlag    = 1,
    SpecularFlag       = 2
    };  // FaceModifiers

enum class EdgeModel
    {
    NoEdges,
    Isolines,
    FacetEdges,
    };  // EdgeModel

enum class EdgeStyles
    {
    NoEdgeStyle         = 0,
    VisibleFlag         = 1,
    SilhouetteFlag      = 2,
    ObscuredFlag        = 4,
    IntersectionFlag    = 8
    };  // EdgeStyles

enum class EdgeModifiers
    {
    NoEdgeModifiers     = 0,
    EdgeOverhangFlag    = 1,
    EdgeJitterFlag      = 2,
    EdgeWidthFlag       = 4,
    EdgeColorFlag       = 8,
    EdgeHaloGapFlag     = 16,
    AlwaysOnTopFlag     = 64,
    EdgeOpacityFlag     = 128,
    EdgeWiggleFlag      = 256,
    EdgeTextureFlag     = 512,
    };  // EdgeModifiers

enum class EdgeJitterAmount
    {
    JitterLow           = 1,
    JitterMedium,
    JitterHigh,
    };  // EdgeJitterAmount

enum class EdgeWiggleAmount
    {
    WiggleLow           = 1,
    WiggleMedium,
    WiggleHigh,
    };  // EdgeWiggleAmount

enum class EdgeLinePattern
    {
    Solid               = 1,
    DashedLine,
    Dotted,
    ShortDash,
    MediumDash,
    LongDash,
    DoubleShortDash,
    DoubleMediumDash,
    DoubleLongDash,
    MediumLongDash,
    SparseDot
    };  // EdgeLinePattern

enum class DisplayStyles
    {
    NoDisplayStyle      = 0,
    BackgroundsFlag     = 1,
    LightingFlag        = 2,
    MaterialsFlag       = 4,
    TexturesFlag        = 8,
    };  // DisplayStyles

enum class DisplayShadowType
    {
    None,
    GroundPlane,
    Full,
    FullAndGround,
    };  // DisplayShadowType

}   // namespace DwgGiVisualStyleProperties

namespace DwgGiVisualStyleOperations
{
enum class Operation
    {
    InvalidOperation    = -1,
    Inherit             = 0,
    Set,
    Disable,
    Enable
    };  // Operation
}   // namespace DwgGiVisualStyleOperations

//__PUBLISH_SECTION_END__
// Conveienent macros casting DwgGiVisualStyle names to/from toolkit's Gi namespaces
#define CASTFROMDwgGiVisProp(__prop__)  static_cast<DWGGI_Type(VisualStyleProperties::Property)>(__prop__)
#define CASTFROMDwgGiVisOp(__op__)      static_cast<DWGGI_Type(VisualStyleOperations::Operation)>(__op__)
#define CASTFROMDwgGiVisOpP(__op__)     reinterpret_cast<DWGGI_TypeP(VisualStyleOperations::Operation)>(__op__)
#define CASTTODwgGiVisProp(__prop__)    static_cast<DwgGiVisualStyleProperties::Property>(__prop__)
#define CASTTODwgGiVisOp(__op__)        static_cast<DwgGiVisualStyleOperations::Operation>(__op__)
//__PUBLISH_SECTION_START__


/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/18
+===============+===============+===============+===============+===============+======*/
class DwgGiVariant : public DWGGI_EXTENDCLASS(Variant)
    {
public:
    DWGGI_DECLARE_BASE_MEMBERS (Variant)
    DWGRX_DEFINE_SMARTPTR_BASE ()
    
    //! Same as T_Super::VariantType
    enum ValueType
        {
        Undefined = 0,
        Boolean,
        Integer,
        Double,
        Color,
        String,
        Table,
        };

    class EnumValueType : public T_Super::EnumType
        {
    public:
        explicit EnumValueType (int value) : T_Super::EnumType(value) {}
        };  // EnumValueType

    //! constructors
    DWGDB_EXPORT DwgGiVariant (bool v);
    DWGDB_EXPORT DwgGiVariant (DwgDbInt32 v);
    DWGDB_EXPORT DwgGiVariant (double v);
    DWGDB_EXPORT DwgGiVariant (DwgCmColorCR v);
    DWGDB_EXPORT DwgGiVariant (WCharCP v);

    // access methods
    DWGDB_EXPORT ValueType      GetType() const;
    DWGDB_EXPORT void           Set (bool v);
    DWGDB_EXPORT void           Set (DwgDbInt32 v);
    DWGDB_EXPORT void           Set (double v);
    DWGDB_EXPORT void           Set (DwgCmColorCR v); 
    DWGDB_EXPORT void           Set (WCharCP v); 
    DWGDB_EXPORT bool           AsBoolean () const;
    DWGDB_EXPORT int            AsInteger () const;
    DWGDB_EXPORT double         AsDouble () const;
    DWGDB_EXPORT DwgCmColor     AsColor () const;
    DWGDB_EXPORT DwgString      AsString () const;    
    DWGDB_EXPORT float          AsFloat () const;
    DWGDB_EXPORT char           AsChar () const;
    DWGDB_EXPORT unsigned char  AsUChar () const;
    DWGDB_EXPORT short          AsShort () const;
    DWGDB_EXPORT unsigned short AsUShort () const;
    DWGDB_EXPORT unsigned int   AsUInt () const;
    DWGDB_EXPORT int32_t        AsLong () const;
    DWGDB_EXPORT uint32_t       AsULong () const;
    DWGDB_EXPORT EnumValueType  AsEnum () const;
    // single entry
    DWGDB_EXPORT DwgGiVariant const*    GetElem (WCharCP key) const;
    DWGDB_EXPORT DwgDbStatus    GetElem (WCharCP key, DwgGiVariant& v) const;
    DWGDB_EXPORT void           SetElem (WCharCP key, DwgGiVariant const& v);
    DWGDB_EXPORT void           DeleteElem (WCharCP key);
    // table entries
    DWGDB_EXPORT uint32_t       GetElemCount () const;
    DWGDB_EXPORT DwgDbStatus    GetElemAt (int i, DwgStringR s, DwgGiVariant& v) const;
    DWGDB_EXPORT DwgGiVariant const*    GetElemAt (int i, DwgStringR s) const;

    // operators
    DWGDB_EXPORT DwgGiVariant&   operator = (DWGGI_TypeCR(Variant) value);
    DWGDB_EXPORT virtual bool    operator == (DWGGI_TypeCR(Variant) value) const;
    };  // DwgGiVariant
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiVariant)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/18
+===============+===============+===============+===============+===============+======*/
class DwgGiVisualStyle : public RefCountedBase
    {
//__PUBLISH_SECTION_END__
// OdGiVisualStyle is an abstract class, so we have to encapsulate it and make this a container class only.
protected:
    DWGGI_TypeR(VisualStyle)    m_toolkitVisualStyle;

//__PUBLISH_SECTION_START__
public:
    DwgGiVisualStyle (DWGGI_TypeR(VisualStyle) giStyle) : m_toolkitVisualStyle(giStyle) {}

    enum class RenderType
        {
        Flat,               // Flat shaded visual style.
        FlatWithEdges,      // Flat shaded visual style with edges displayed.
        Gouraud,            // Gouraud shaded visual style.
        GouraudWithEdges,   // Gouraud shaded visual style with edges displayed.
        Wireframe2d,        // 2D wireframe visual style (using 2D graphics system).
        Wireframe3d,        // 3D wireframe visual style (using 3D graphics system).
        Hidden,             // Hidden visual style.
        Basic,              // Basic visual style (default).
        Realistic,          // Phong shaded visual style.
        Conceptual,         // Custom, user defined visual visual style.
        Custom,             // Custom, user defined visual visual style.
        Dim,                // Visual style used for a dimming effect.
        Brighten,           // Visual style used for a brightening effect.
        Thicken,            // Visual style used for a thickening effect.
        LinePattern,        // Visual style used to apply a line pattern.
        FacePattern,        // Visual style used to apply a face pattern.
        ColorChange,        // Visual style used to apply a change of color.
        FaceOnly,           // Visual style with only face properties. All non-face properties are set to inherit.
        EdgeOnly,           // Visual stle with edge properties only. All non-edge properties are set to inherit.
        DisplayOnly,        // Visual style with display properties only. All non-display properties are set to inherit.
        JitterOff,          // Edge style override visual style with jitter edges off. All other properties are set to inherit.
        OverhangOff,        // Edge style override visual style with overhang edges off. All other properties are set to inherit.
        EdgeColorOff,       // Edge style override visual style with edge color off. All other properties are set to inherit.
        ShadesOfGray,       // Shades of gray visual style.
        Sketchy,            // Sketchy visual style.
        XRay,               // Xray visual style.
        ShadedWithEdges,    // Shade visual style with edges displayed.
        Shaded,             // Shaded visual style.
        ByViewport,         // Visual style by viewport.
        ByLayer,            // Visual style by layer.
        ByBlock,            // Visual style by block.
        EmptyStyle          // Visual style with all properties set to inherit. This effectively creates an empty style upon which a custom visual style can be built.
        };

    //! Type of the visual style.
    DWGDB_EXPORT RenderType     GetType () const;
    //! Gets a property of the visual style.  
    //! @param prop Input DwgGiVisualStyleProperties::Property to get from the visual style.
    //! @param op DwgGiVisualStyleOperations::Operation to get the operation currently in effect for this property. If nullptr, nothing is returned.
    //! @return The DwgGiVariant value of the property if successful; otherwise, an AcGiVariant of type DwgGiVariant::Undefined.
    DWGDB_EXPORT DwgGiVariantCR GetTrait (DwgGiVisualStyleProperties::Property prop, DwgGiVisualStyleOperations::Operation* op = nullptr) const;
    //! Gets the operation associated with a property.
    //! @param prop Name of the property for which to get the associated operation value.
    DWGDB_EXPORT DwgGiVisualStyleOperations::Operation  GetOperation (DwgGiVisualStyleProperties::Property prop) const;
    //! Gets a property flag from the visual style, for properties which are bitfield enums.
    //! @param prop A bitfield enum DwgGiVisualStyleProperties::Property to get from the visual style.
    //! @param flags A bitfield flag enum property to get from the visual style.
    DWGDB_EXPORT bool   GetTraitFlag (DwgGiVisualStyleProperties::Property prop, DwgDbUInt32 flags) const;
    };  // DwgGiVisualStyle
typedef RefCountedPtr<DwgGiVisualStyle> DwgGiVisualStylePtr;
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiVisualStyle)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          10/16
+===============+===============+===============+===============+===============+======*/
class DwgGiMapper : public DWGGI_EXTENDCLASS(Mapper)
    {
public:
    DWGGI_DECLARE_BASE_MEMBERS (Mapper)
    DWGROOTCLASS_ADD_CONSTRUCTORS (GiMapper)

    enum TransformBy    // == T_Super::AutoTransform
        {
        InheritTransform    = 0x0,
        None                = 0x1,
        Object              = 0x2,
        Model               = 0x4
        };

    enum ProjectBy      // == T_Super::Projection
        {
        InheritProjection   = 0,
        Planar              = 1,
        Box                 = 2,
        Cylinder            = 3,
        Sphere              = 4,
        };

    enum TileBy         // == T_Super::Tiling
        {
        InheritTiling       = 0,
        Tile                = 1,
        Crop                = 2,
        Clamp               = 3,
        Mirror              = 4,
        };

    DWGDB_EXPORT void           GetTransform (TransformR t) const;
    DWGDB_EXPORT TransformBy    GetAutoTransform () const;
    DWGDB_EXPORT ProjectBy      GetProjection () const;
    DWGDB_EXPORT TileBy         GetUTiling () const;
    DWGDB_EXPORT TileBy         GetVTiling () const;
    }; // DwgGiMapper
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiMapper)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          10/16
+===============+===============+===============+===============+===============+======*/
class DwgGiMaterialColor : public DWGGI_EXTENDCLASS(MaterialColor)
    {
public:
    DWGGI_DECLARE_BASE_MEMBERS (MaterialColor)
    DWGROOTCLASS_ADD_CONSTRUCTORS (GiMaterialColor)

    enum ColorBy        // == T_Super::Method
        {
        Inherit             = 0,
        Override            = 1,
        };

    DWGDB_EXPORT void           GetColor (DwgCmEntityColorR color) const;
    DWGDB_EXPORT double         GetFactor () const;
    DWGDB_EXPORT ColorBy        GetMethod () const;
    }; // DwgGiMaterialColor
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiMaterialColor)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          10/16
+===============+===============+===============+===============+===============+======*/
class DwgGiImageFileTexture : public DWGGI_EXTENDCLASS(ImageFileTexture)
    {
public:
    DWGGI_DECLARE_RX_MEMBERS (ImageFileTexture)
    DwgGiImageFileTexture (DWGGI_TypeCR(ImageFileTexture) t) { T_Super::operator=(t); }

    DWGDB_EXPORT DwgString          GetSourceFileName () const;
    }; // DwgGiImageFileTexture
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiImageFileTexture)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          10/16
+===============+===============+===============+===============+===============+======*/
class DwgGiGenericTexture : public DWGGI_EXTENDCLASS(GenericTexture)
    {
public:
    DWGGI_DECLARE_RX_MEMBERS (GenericTexture)

    DwgGiGenericTexture () : T_Super() {}
    DwgGiGenericTexture (DWGGI_TypeCR(GenericTexture) t) { T_Super::operator=(t); }
    }; // DwgGiGenericTexture
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiGenericTexture)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          10/16
+===============+===============+===============+===============+===============+======*/
class DwgGiMarbleTexture : public DWGGI_EXTENDCLASS(MarbleTexture)
    {
public:
    DWGGI_DECLARE_RX_MEMBERS (MarbleTexture)

    DwgGiMarbleTexture () : T_Super() {}
    DwgGiMarbleTexture (DWGGI_TypeCR(MarbleTexture) t) { T_Super::operator=(t); }
    }; // DwgGiMarbleTexture
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiMarbleTexture)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          10/16
+===============+===============+===============+===============+===============+======*/
class DwgGiWoodTexture : public DWGGI_EXTENDCLASS(WoodTexture)
    {
public:
    DWGGI_DECLARE_RX_MEMBERS (WoodTexture)

    DwgGiWoodTexture () : T_Super() {}
    DwgGiWoodTexture (DWGGI_TypeCR(WoodTexture) t) { T_Super::operator=(t); }
    }; // DwgGiWoodTexture
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiWoodTexture)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          10/16
+===============+===============+===============+===============+===============+======*/
class DwgGiMaterialTexture : public DWGGI_EXTENDCLASS(MaterialTexture)
    {
public:
    DWGGI_DECLARE_RX_MEMBERS (MaterialTexture)

    DwgGiMaterialTexture () : T_Super() {}
    DWGDB_EXPORT virtual bool operator==(const DwgGiMaterialTexture& t) const;

    // Image file:
    DWGDB_EXPORT DwgGiImageFileTextureCP    ToDwgGiImageFileTextureCP () const;
    // Procedural types:
    DWGDB_EXPORT DwgGiGenericTextureCP      ToDwgGiGenericTextureCP () const;
    DWGDB_EXPORT DwgGiMarbleTextureCP       ToDwgGiMarbleTextureCP () const;
    DWGDB_EXPORT DwgGiWoodTextureCP         ToDwgGiWoodTextureCP () const;
    };  // DwgGiMaterialTexture
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiMaterialTexture)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          10/16
+===============+===============+===============+===============+===============+======*/
class DwgGiMaterialMap : public DWGGI_EXTENDCLASS(MaterialMap)
    {
public:
    DWGGI_DECLARE_BASE_MEMBERS (MaterialMap)
    DWGROOTCLASS_ADD_CONSTRUCTORS (GiMaterialMap)

    enum FilterBy       // == T_Super::Filter
        {
        Default             = 0,
        None                = 1,
        };

    enum ImageBy        // == T_Super::Source
        {
        Scene               = 0,
        File                = 1,
        Procedural          = 2,
        };

    DWGDB_EXPORT double                     GetBlendFactor () const;
    DWGDB_EXPORT FilterBy                   GetFilter () const;
    DWGDB_EXPORT void                       GetMapper (DwgGiMapperR m) const;
    DWGDB_EXPORT ImageBy                    GetSource () const;
    DWGDB_EXPORT DwgGiMaterialTextureCP     GetTexture () const;
    }; // DwgGiMaterialMap
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiMaterialMap)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          11/17
+===============+===============+===============+===============+===============+======*/
class DwgGiLightAttenuation : public DWGGI_EXTENDCLASS(LightAttenuation)
    {
public:
    DWGGI_DECLARE_BASE_MEMBERS (LightAttenuation)
    DWGROOTCLASS_ADD_CONSTRUCTORS (GiLightAttenuation)

    //! The attenuation type, or decay, of a point or spot light. A distant light has no attenuation.
    enum Type
        {
        //! No attenuation; emitted light has the same brightness (intensity) regardless of the distance to the source.
        None =      0,
        //! The attenuation is the inverse of the linear distance from the light. 
        InverseLinear,
        //! The attenuation is the inverse of the square of the distance from the light. 
        InverseSquare
        };

    //! Get the type of the attenuation of a point or spot light
    DWGDB_EXPORT Type   GetType () const;
    //! Are the start and end limits used?
    DWGDB_EXPORT bool   UseLimits () const;
    //! Get the distance from the light source where light begins to affect the model; objects closer than this are not affected by the light.
    DWGDB_EXPORT double GetStartLimit () const;
    //! Get the distance from the light source beyond which the light has no affect.
    DWGDB_EXPORT double GetEndLimit () const;
    };  // DwgGiLightAttenuation
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiLightAttenuation)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          11/17
+===============+===============+===============+===============+===============+======*/
class DwgGiShadowParameters : public DWGGI_EXTENDCLASS(ShadowParameters)
    {
public:
    DWGGI_DECLARE_BASE_MEMBERS (ShadowParameters)
    DWGROOTCLASS_ADD_CONSTRUCTORS (GiShadowParameters)

    //! Shadowing methods
    enum Type
        {
        RayTraced = 0,
        Mapped,
        Sampled,
        };  // Type

    //! The shape of the extended light source
    enum Shape
        {
        Linear  = 0,
        Rectangle,
        Disk,
        Cylinder,
        Sphere
        };  // Shape

    //! Are shadows casted by the light?
    DWGDB_EXPORT bool   AreShadowsOn () const;
    //! Get the method used to calculate shadows cast by this light.
    //! @return RayTraced - shadows are calculated using a ray-trace algorithmm, Mapped - shadow maps are created for each light, or Sampled - the area-sampled shadow algorithm models the effect of extended light sources which typically exhibit penumbra.
    DWGDB_EXPORT Type   GetShadowType () const;
    //! Get the size of the shadow map, in pixels.
    //! @note Only applies if shadow type is Mapped.
    //! @return 64, 128, 256, 512, 1024, 2048, or 4096
    DWGDB_EXPORT uint16_t   GetShadowMapSize () const;
    //! Get the softness (or fuzziness) of the edge of the shadow. The value represents the number of pixels at the edge of the shadow that are blended into the underlying image.
    //! @note Only applies if shadow type is Mapped.
    //! @return The number of pixels at the edge of the shadow to blend.
    DWGDB_EXPORT uint8_t    GetShadowMapSoftness () const;
    //! The number of shadow rays to shoot for the light.
    //! @note Only applies if shadow type is Sampled.
    //! @return The number of shadow rays to shoot for the light.
    DWGDB_EXPORT uint16_t   GetShadowSamples () const;
    //! Determines if the light shape is visible in the rendering.
    DWGDB_EXPORT bool   IsShapeVisible() const;
    //! Get the shape of the extended light source. Valid only if shadow type is Sampled.
    DWGDB_EXPORT Shape  GetExtendedLightShape() const;
    //! Get the length of the extended light source.
    //! @note Only applies if shadow type is Sampled, and light source shape is Linear, Rectangle or Cylinder.
    DWGDB_EXPORT double GetExtendedLightLength() const;
    //! Get the width of the extended light source.
    //! @note Only applies if shadow type is Sampled, and light source shape is Rectangle.
    DWGDB_EXPORT double GetExtendedLightWidth() const;
    //! Get the radius of the extended light source.
    //! @note Only applies if shadow type is Sampled, and light source shape is Disk, Cylinder or Sphere.
    DWGDB_EXPORT double GetExtendedLightRadius() const;
    };  // DwgGiShadowParameters
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiShadowParameters)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          11/17
+===============+===============+===============+===============+===============+======*/
class DwgGiSkyParameters : public RefCountedBase
    {
//__PUBLISH_SECTION_END__
    // AcGiSkyParameters has an implementation layer that is not inlined, effectively preventing us from deriving the class without providing the implementation!
private:
    DWGGI_Type(SkyParameters)    m_toolkitImpl;
//__PUBLISH_SECTION_START__
public:
    DwgGiSkyParameters () {}
    DwgGiSkyParameters (DWGGI_TypeCR(SkyParameters) in);

    //! Should aerial perspective be applied? 
    DWGDB_EXPORT bool       HasAerialPerspective () const;
    //! The intensity of the sun disc.
    DWGDB_EXPORT double     GetDiskIntensity () const;
    //! The scale of the sun disk (1.0 = correct size). 
    DWGDB_EXPORT double     GetDiskScale () const;
    //! The intensity of the sun glow.
    DWGDB_EXPORT double     GetGlowIntensity () const;
    //! The color of the ground.
    DWGDB_EXPORT DwgCmEntityColor GetGroundColor () const;
    //! The color of the night sky.
    DWGDB_EXPORT DwgCmEntityColor GetNightColor () const;
    //! The turbidity or atmosphere value.
    DWGDB_EXPORT double     GetHaze () const;
    //! The amount of blurring between ground plane and sky. 
    DWGDB_EXPORT double     GetHorizonBlur () const;
    //! The world-space height of the horizon plane. 
    DWGDB_EXPORT double     GetHorizonHeight () const;
    //! Should skylight illumination be calculated?
    DWGDB_EXPORT bool       HasIllumination () const;
    //! The intensity factor which determines the level of non-physical modulation of skylight.
    DWGDB_EXPORT double     GetIntensityFactor () const;
    //! The red-blue shift on the sky. 
    //! @note This provides control on the "redness" of the sky. The default of 0.0 is physically accurate. A minimum value of -1.0 will produce an extremely blue sky, whereas the maximum value of 1.0 will produce an extremely red sky.
    DWGDB_EXPORT double     GetRedBlueShift () const;
    //! The the sky's saturation level.
    //! @note The minimum value of 0.0 will produce an extreme of black and white whereas the maximum value of 2.0 will produce highly boosted saturation.
    DWGDB_EXPORT double     GetSaturation () const;
    //! The number of samples to take on the solar disk.
    DWGDB_EXPORT uint16_t   GetSolarDiskSamples () const;
    //! The direction from the Sun to the model.
    DWGDB_EXPORT DVec3d     GetSunDirection () const;
    //! The distance at which 10% haze occlusion results. 
    DWGDB_EXPORT double     GetVisibilityDistance () const;
    };  // DwgGiSkyParameters
typedef RefCountedPtr<DwgGiSkyParameters>   DwgGiSkyParametersPtr;
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiSkyParameters)

END_DWGDB_NAMESPACE
//__PUBLISH_SECTION_END__
