/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbSync/Dwg/DwgDb/DwgDrawables.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include    <DgnDbSync/Dwg/DwgDb/DwgDbObjects.h>
#include    <DgnDbSync/Dwg/DwgDb/DwgDbSymbolTables.h>

#ifdef DWGTOOLKIT_OpenDwg
#include    <Teigha/Core/Include/Gi/GiDrawable.h>
#include    <Teigha/Core/Include/Gi/GiWorldDraw.h>
#include    <Teigha/Core/Include/Gi/GiViewportDraw.h>
#include    <Teigha/Core/Include/Gi/GiTextStyle.h>
#include    <Teigha/Core/Include/Gi/GiImage.h>
#include    <Teigha/Core/Include/Gi/GiClipBoundary.h>
#include    <Teigha/Core/Include/Gi/GiMaterial.h>
#elif DWGTOOLKIT_RealDwg
#include    <RealDwg/Base/acgi.h>
#else
    #error  "Must define DWGTOOLKIT!!!"
#endif

BEGIN_DWGDB_NAMESPACE


enum class DwgGiArcType
    {
    Simple                  = 0,    // just the arc (not fillable)
    Sector                  = 1,    // area bounded by the arc and its center of curvature
    Chord                   = 2,    // area bounded by the arc and its end points
    };

enum class DwgGiTransparencyMode
    {
    Default                 = -1, // Default for native raster image (alpha 0 - transparent, 1-255 opaque)
    AlphaOff                = 0,  // Alpha ignored for 32bpp formats
    Alpha1Bit               = 1,  // Alpha 0-254 transparent, 255 opaque
    Alpha8Bit               = 2   // Complete alpha level support for 32bpp formats
    };

enum class DwgGiRegenType
    {
    Invalid                 = 0,
    StandardDisplay         = 2,
    HideOrShadeCommand      = 3,
    RenderCommand           = 4,
    ForExplode              = 5,
    SaveWorldDrawForProxy   = 6,
    ForExtents              = 7,    // OpenDWG only
    };  // DwgGiRegenType

enum class DwgGiDeviationType
    {
    MaxDevForCircle         = 0,
    MaxDevForCurve          = 1,
    MaxDevForBoundary       = 2,
    MaxDevForIsoline        = 3,
    MaxDevForFacet          = 4,
    };

enum class DwgGiFillType
    {
    Default                 = 0,    // Neither toolkit has this one - we added it to denote status of "not set yet".
    Always                  = 1,
    Never                   = 2,
    };

enum class DwgGiVisibility
    {
    Invisible               = 0,
    Visible                 = 1,
    Silhouette              = 3,
    };

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgGiEdgeData : public DWGGI_EXTENDCLASS(EdgeData)
    {
public:
    DEFINE_T_SUPER (DWGROOT_SUPER_CONSTRUCTOR(GiEdgeData))
    DWGROOTCLASS_ADD_CONSTRUCTORS (GiEdgeData)
#ifdef DWGTOOLKIT_RealDwg
    DWGRX_DECLARE_MEMBERS_EXPIMP(DwgGiEdgeData)
#endif

    DWGDB_EXPORT uint8_t const*         GetVisibility () const;
    DWGDB_EXPORT int16_t const*         GetColors () const;
    DWGDB_EXPORT DwgCmEntityColorCP     GetTrueColors () const;
    DWGDB_EXPORT DwgDbObjectIdCP        GetLayers () const;
    DWGDB_EXPORT DwgDbObjectIdCP        GetLinetypes () const;
    DWGDB_EXPORT DwgDbLongPtr const*    GetSelectionMarkers () const;
    };  // DwgGiEdgeData
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiEdgeData)

class DwgGiMapper;
/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgGiFaceData : public DWGGI_EXTENDCLASS(FaceData)
    {
public:
    DEFINE_T_SUPER (DWGROOT_SUPER_CONSTRUCTOR(GiFaceData))
    DWGROOTCLASS_ADD_CONSTRUCTORS (GiFaceData)
#ifdef DWGTOOLKIT_RealDwg
    DWGRX_DECLARE_MEMBERS_EXPIMP(DwgGiFaceData)
#endif
    
    DWGDB_EXPORT int16_t const*         GetColors () const;
    DWGDB_EXPORT DwgCmEntityColorCP     GetTrueColors () const;
    DWGDB_EXPORT DwgTransparencyCP      GetTransparencies () const;
    DWGDB_EXPORT DwgDbObjectIdCP        GetLayers () const;
    DWGDB_EXPORT DwgDbObjectIdCP        GetMaterials () const;
    DWGDB_EXPORT DwgGiMapper const*     GetMappers () const;
    DWGDB_EXPORT DVec3dCP               GetNormals () const;
    };  // DwgGiFaceData
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiFaceData)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgGiVertexData : public DWGGI_EXTENDCLASS(VertexData)
    {
public:
    DEFINE_T_SUPER (DWGROOT_SUPER_CONSTRUCTOR(GiVertexData))
    DWGROOTCLASS_ADD_CONSTRUCTORS (GiVertexData)
#ifdef DWGTOOLKIT_RealDwg
    DWGRX_DECLARE_MEMBERS_EXPIMP(DwgGiVertexData)
#endif

    enum MappingChannel
        {
        All     = DWGGI_Type(VertexData)::kAllChannels,
        Map     = 1,                // OpenDWG does not have this prop
        };
    
    DWGDB_EXPORT DPoint3dCP             GetMappingCoords (MappingChannel channel = All) const;
    DWGDB_EXPORT DVec3dCP               GetNormals () const;
    DWGDB_EXPORT DwgCmEntityColorCP     GetTrueColors () const;
    };  // DwgGiVertexData
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiVertexData)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgGiTextStyle : public DWGGI_EXTENDCLASS(TextStyle)
    {
public:
    DEFINE_T_SUPER (DWGROOT_SUPER_CONSTRUCTOR(GiTextStyle))
    DWGROOTCLASS_ADD_CONSTRUCTORS (GiTextStyle)

    DWGDB_EXPORT DwgDbStatus            GetFontInfo (DwgFontInfoR info) const;
    DWGDB_EXPORT WString                GetFileName () const;
    DWGDB_EXPORT WString                GetBigFontFileName () const;
    DWGDB_EXPORT double                 GetTextSize () const;
    DWGDB_EXPORT double                 GetWidthFactor () const;
    DWGDB_EXPORT double                 GetObliqueAngle () const;
    DWGDB_EXPORT bool                   IsVertical () const;
    DWGDB_EXPORT bool                   IsShapeFile () const;
    DWGDB_EXPORT bool                   IsUnderlined () const;
    DWGDB_EXPORT bool                   IsOverlined () const;
    DWGDB_EXPORT bool                   IsStrikethrough () const;
    DWGDB_EXPORT bool                   IsBackward () const;
    DWGDB_EXPORT bool                   IsUpsideDown () const;
    DWGDB_EXPORT DwgDbStatus            CopyFrom (DwgDbTextStyleTableRecordCR dbStyle);
    };  // DwgGiTextStyle
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiTextStyle)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgGiImageBGRA32 : public DWGGI_EXTENDCLASS(ImageBGRA32)
    {
public:
    DWGROOTCLASS_ADD_CONSTRUCTORS (GiImageBGRA32)
    
    };  // DwgGiImageBGRA32
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiImageBGRA32)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgGiClipBoundary : public DWGGI_EXTENDCLASS(ClipBoundary)
    {
public:
    DWGROOTCLASS_ADD_CONSTRUCTORS (GiClipBoundary)
    
    };  // DwgGiClipBoundary
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiClipBoundary)

/*-------------------------------------------------------------------------------------------*/
//! We have choosen encapsulating, instead of extending, below gradient fill classes based on
//! the facts that only RealDWG has these classes and some of its public contructors exposes 
//! arguments such as AcArray<AcCmColor> which cause unresolved link errors in an end app that 
//! includes this header file. 
/*-------------------------------------------------------------------------------------------*/
/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgGiFill : public RefCountedBase
    {
//__PUBLISH_SECTION_END__
protected:
#ifdef DWGTOOLKIT_OpenDwg
    double                  m_deviation;

#elif DWGTOOLKIT_RealDwg
    AcGiFill*               m_acFill;

public:
    const AcGiFill*         GetAcGiFill () const { return m_acFill; }
    void                    SetAcGiFill (const AcGiFill* fill);

    DwgGiFill (const AcGiFill& fill);
#endif
//__PUBLISH_SECTION_START__

public:
    DwgGiFill ();
    ~DwgGiFill ();

    DWGDB_EXPORT double     GetDeviation () const;
    DWGDB_EXPORT void       SetDeviation (double deviation);
    };  // DwgGiFill
typedef RefCountedPtr<DwgGiFill>        DwgGiFillPtr;
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiFill)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgGiGradientFill : public DwgGiFill
    {
    DEFINE_T_SUPER (DwgGiFill)

public:
    enum class Type
        {
        Linear,
        Cylinder,
        InvCylinder,
        Spherical,
        Hemispherical,
        Curved,
        InvSpherical,
        InvHemispherical,
        InvCurved
        };  // Type

//__PUBLISH_SECTION_END__
private:
    double          m_tint;     // 0.0 - 1.0

#ifdef DWGTOOLKIT_OpenDwg
    double          m_angle;
    double          m_shift;
    bool            m_adjustAspect;
    Type            m_type;
    DwgColorArray   m_colorArray;

public:
    DwgGiGradientFill () : m_angle(0.0), m_shift(0.0), m_adjustAspect(false), m_type(Type::Linear), m_colorArray(0) {}

#elif DWGTOOLKIT_RealDwg
public:
    DwgGiGradientFill (const AcGiGradientFill& grad);
    DwgGiGradientFill () : T_Super() {}
#endif
//__PUBLISH_SECTION_START__

public:
    DWGDB_EXPORT Type           GetType () const; 
    DWGDB_EXPORT void           SetType (Type t); 
    DWGDB_EXPORT double         GetAngle () const;
    DWGDB_EXPORT void           SetAngle (double angle);
    DWGDB_EXPORT double         GetShift () const;
    DWGDB_EXPORT void           SetShift (double shift);
    DWGDB_EXPORT bool           IsAdjustAspect () const;
    DWGDB_EXPORT void           SetAdjustAspect (bool adjust);
    DWGDB_EXPORT size_t         GetColors (DwgColorArrayR colors) const;
    DWGDB_EXPORT void           SetColors (DwgColorArrayCR colors);
    DWGDB_EXPORT double         GetTint () const;
    DWGDB_EXPORT void           SetTint (double tint);

    DwgGiGradientFill (double angle, double shift, double tint, bool adjustAspect, DwgColorArrayCR colors, Type t);

    DWGDB_EXPORT static RefCountedPtr<DwgGiGradientFill>    CreateFrom (DwgDbHatchCP hatch);
    };  // DwgGiGradientFill
typedef RefCountedPtr<DwgGiGradientFill>    DwgGiGradientFillPtr;
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiGradientFill)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgGiHatchPatternDefinition 
#ifdef DWGTOOLKIT_OpenDwg
    {
public:
    double              m_angle;
    double              m_baseX;
    double              m_baseY;
    double              m_offsetX;
    double              m_offsetY;
    DwgDbDoubleArray    m_dashArray;

#elif DWGTOOLKIT_RealDwg
    : public AcGiHatchPatternDefinition
    {
    DEFINE_T_SUPER (AcGiHatchPatternDefinition)
#endif

    DwgGiHatchPatternDefinition ();
//__PUBLISH_SECTION_END__
    DwgGiHatchPatternDefinition (double angle, double baseX, double baseY, double offX, double offY, DWGGE_TypeR(DoubleArray) dashes);
//__PUBLISH_SECTION_START__

    };  // DwgGiHatchPatternDefinition
typedef bvector<DwgGiHatchPatternDefinition>    DwgPatternArray;
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiHatchPatternDefinition)
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgPatternArray)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgGiHatchPattern : public DwgGiFill
    {
    DEFINE_T_SUPER (DwgGiHatchPattern)

//__PUBLISH_SECTION_END__
private:
#ifdef DWGTOOLKIT_OpenDwg
    DwgPatternArray     m_patternArray;

#elif DWGTOOLKIT_RealDwg
public:
    DwgGiHatchPattern (const AcGiHatchPattern& pattern);
#endif
//__PUBLISH_SECTION_START__

public:
    DwgGiHatchPattern ();
    DwgGiHatchPattern (DwgPatternArrayCR patterns);

    DWGDB_EXPORT static RefCountedPtr<DwgGiHatchPattern>    CreateFrom (DwgDbHatchCP hatch);
    };  // DwgGiHatchPattern
typedef RefCountedPtr<DwgGiHatchPattern>    DwgGiHatchPatternPtr;
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiHatchPattern)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          10/16
+===============+===============+===============+===============+===============+======*/
class DwgGiMapper : public DWGGI_EXTENDCLASS(Mapper)
    {
public:
    DEFINE_T_SUPER (DWGROOT_SUPER_CONSTRUCTOR(GiMapper))
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

    enum TileBy         // == T_Super:: Tiling
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
    DEFINE_T_SUPER (DWGROOT_SUPER_CONSTRUCTOR(GiMaterialColor))
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
    DEFINE_T_SUPER (DWGROOT_SUPER_CONSTRUCTOR(GiImageFileTexture))
#ifdef DWGTOOLKIT_RealDwg
    DWGRX_DECLARE_MEMBERS_EXPIMP(DwgGiImageFileTexture)
#endif
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
    DEFINE_T_SUPER (DWGROOT_SUPER_CONSTRUCTOR(GiGenericTexture))
#ifdef DWGTOOLKIT_RealDwg
    DWGRX_DECLARE_MEMBERS_EXPIMP(DwgGiGenericTexture)
#endif
    DwgGiGenericTexture (DWGGI_TypeCR(GenericTexture) t) { T_Super::operator=(t); }

    }; // DwgGiGenericTexture
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiGenericTexture)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          10/16
+===============+===============+===============+===============+===============+======*/
class DwgGiMarbleTexture : public DWGGI_EXTENDCLASS(MarbleTexture)
    {
public:
    DEFINE_T_SUPER (DWGROOT_SUPER_CONSTRUCTOR(GiMarbleTexture))
#ifdef DWGTOOLKIT_RealDwg
    DWGRX_DECLARE_MEMBERS_EXPIMP(DwgGiMarbleTexture)
#endif
    DwgGiMarbleTexture (DWGGI_TypeCR(MarbleTexture) t) { T_Super::operator=(t); }
    }; // DwgGiMarbleTexture
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiMarbleTexture)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          10/16
+===============+===============+===============+===============+===============+======*/
class DwgGiWoodTexture : public DWGGI_EXTENDCLASS(WoodTexture)
    {
public:
    DEFINE_T_SUPER (DWGROOT_SUPER_CONSTRUCTOR(GiWoodTexture))
#ifdef DWGTOOLKIT_RealDwg
    DWGRX_DECLARE_MEMBERS_EXPIMP(DwgGiWoodTexture)
#endif
    DwgGiWoodTexture (DWGGI_TypeCR(WoodTexture) t) { T_Super::operator=(t); }
    }; // DwgGiWoodTexture
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiWoodTexture)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          10/16
+===============+===============+===============+===============+===============+======*/
class DwgGiMaterialTexture : public DWGGI_EXTENDCLASS(MaterialTexture)
    {
public:
    DEFINE_T_SUPER (DWGROOT_SUPER_CONSTRUCTOR(GiMaterialTexture))
#ifdef DWGTOOLKIT_RealDwg
    DWGRX_DECLARE_MEMBERS_EXPIMP(DwgGiMaterialTexture)
#endif

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
    DEFINE_T_SUPER (DWGROOT_SUPER_CONSTRUCTOR(GiMaterialMap))
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
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
struct DwgGiDrawable : public RefCountedBase
    {
    friend struct IDwgDrawGeometry;
    friend struct IDwgDrawOptions;
    friend struct IDwgDrawParameters;

private:
#ifdef DWGTOOLKIT_OpenDwg
    const OdGiDrawable*         m_toolkitDrawable;

public:
    DwgGiDrawable (const OdGiDrawable* dr) : m_toolkitDrawable(dr) {}

#elif DWGTOOLKIT_RealDwg
    AcGiDrawable*               m_toolkitDrawable;

public:
    DwgGiDrawable (AcGiDrawable* dr) : m_toolkitDrawable(dr) {}
#endif

    DWGDB_EXPORT bool                       IsValid ();
    DWGDB_EXPORT bool                       IsPersistent ();
    DWGDB_EXPORT DwgDbObjectId              GetId ();
    DWGDB_EXPORT DwgDbEntityP               GetEntityP ();
    DWGDB_EXPORT DwgDbLineP                 GetLineP ();
    DWGDB_EXPORT DwgDbCircleP               GetCircleP ();
    DWGDB_EXPORT DwgDbArcP                  GetArcP ();
    DWGDB_EXPORT DwgDbBlockTableRecordP     GetBlockP ();
    DWGDB_EXPORT DwgDbAttributeDefinitionP  GetAttributeDefinitionP ();
    DWGDB_EXPORT void                       Draw (IDwgDrawGeometry& drawGeom, IDwgDrawOptions& options, IDwgDrawParameters& drawParams);
    };  // DwgGiDrawable
typedef RefCountedPtr<DwgGiDrawable>    DwgGiDrawablePtr;
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiDrawable)

/*=================================================================================**//**
//! Each of these metheds should be implemented such that they can be called from the toolkit
//! when DwgDbEntity::Draw is called.
//! @see IDwgDrawParameters
+===============+===============+===============+===============+===============+======*/
struct IDwgDrawGeometry
    {
public:
    virtual void _Circle (DPoint3dCR center, double radius, DVec3dCR normal) = 0;
    virtual void _Circle (DPoint3dCR point1, DPoint3dCR point2, DPoint3dCR point3) = 0;
    virtual void _CircularArc (DPoint3dCR center, double rad, DVec3dCR normal, DVec3dCR start, double swept, DwgGiArcType type = DwgGiArcType::Simple) = 0;
    virtual void _CircularArc (DPoint3dCR start, DPoint3dCR point, DPoint3dCR end, DwgGiArcType type = DwgGiArcType::Simple) = 0;
    virtual void _Curve (MSBsplineCurveCR curve) = 0;
    virtual void _Edge (CurveVectorCR edges) = 0; 
    virtual void _Ellipse (DEllipse3dCR ellipse, DwgGiArcType type = DwgGiArcType::Simple) = 0;
    virtual void _Polyline (size_t nPoints, DPoint3dCP points, DVec3dCP normal = nullptr, std::ptrdiff_t subentMarker = -1) = 0;
    virtual void _Pline (DwgDbPolylineCR pline, size_t fromIndex = 0, size_t numSegs = 0) = 0;
    virtual void _Polygon (size_t nPoints, DPoint3dCP points) = 0;
    virtual void _Mesh (size_t nRows, size_t nColumns, DPoint3dCP points, DwgGiEdgeDataCP edges = nullptr, DwgGiFaceDataCP faces = nullptr, DwgGiVertexDataCP verts = nullptr, bool autonorm = false) = 0;
    virtual void _Shell (size_t nPoints, DPoint3dCP points, size_t nFaceList, int32_t const* faces, DwgGiEdgeDataCP edgeData = nullptr, DwgGiFaceDataCP faceData = nullptr, DwgGiVertexDataCP vertData = nullptr, DwgResBufCP xData = nullptr, bool autonorm = false) = 0;
    virtual void _Text (DPoint3dCR position, DVec3dCR normal, DVec3dCR xdir, double h, double w, double oblique, DwgStringCR msg) = 0;
    virtual void _Text (DPoint3dCR position, DVec3dCR normal, DVec3dCR xdir, DwgStringCR msg, bool raw, DwgGiTextStyleCR style) = 0;
    virtual void _Xline (DPoint3dCR point1, DPoint3dCR point2) = 0;
    virtual void _Ray (DPoint3dCR origin, DPoint3dCR point) = 0;
    virtual void _Draw (DwgGiDrawableR drawable) = 0;
    virtual void _Image (DwgGiImageBGRA32CR image, DPoint3dCR pos, DVec3dCR u, DVec3dCR v, DwgGiTransparencyMode mode = DwgGiTransparencyMode::Alpha8Bit) = 0;
    virtual void _RowOfDots (size_t count, DPoint3dCR start, DVec3dCR step) = 0;
    virtual void _WorldLine (DPoint3d points[2]) = 0;
    virtual void _PushModelTransform (TransformCR newTransform) = 0;
    virtual void _PopModelTransform () = 0;
    virtual void _GetModelTransform (TransformR newTransform) = 0;
    virtual void _PushClipBoundary (DwgGiClipBoundaryCP boundary) = 0;
    virtual void _PopClipBoundary () = 0;
    };  // IDwgDrawGeometry
DEFINE_NO_NAMESPACE_TYPEDEFS (IDwgDrawGeometry)

/*=================================================================================**//**
//! Implement these methods such that prior to a graphical component of an entity is drawn,
//! these methods may be called from the toolkit to set the symbology etc for the component.
//! Therefore it is used and controlled by the toolkit and is a per component operation.
//! @see DwgDrawGeometry
+===============+===============+===============+===============+===============+======*/
struct IDwgDrawParameters
    {
    virtual DwgCmEntityColorCR  _GetColor () const = 0;
    virtual DwgDbObjectIdCR     _GetLayer () const = 0;
    virtual DwgDbObjectIdCR     _GetLineType () const = 0;
    virtual DwgGiFillType       _GetFillType () const = 0;
    virtual DwgGiFillCP         _GetFill () const = 0;
    virtual DwgDbLineWeight     _GetLineWeight () const = 0;
    virtual double              _GetLineTypeScale () const = 0;
    virtual double              _GetThickness () const = 0;
    virtual DwgTransparencyCR   _GetTransparency () const = 0;
    virtual DwgDbObjectIdCR     _GetMaterial () const = 0;

    virtual void                _SetColor (DwgCmEntityColorCR color) = 0;
    virtual void                _SetLayer (DwgDbObjectIdCR layerId) = 0;
    virtual void                _SetLineType (DwgDbObjectIdCR linetypeId) = 0;
    virtual void                _SetSelectionMarker (std::ptrdiff_t markerId) = 0;
    virtual void                _SetFillType (DwgGiFillType filltype) = 0;
    virtual void                _SetFill (DwgGiFillCP fill) = 0;
    virtual void                _SetLineWeight (DwgDbLineWeight weight) = 0;
    virtual void                _SetLineTypeScale (double scale) = 0;
    virtual void                _SetThickness (double thickness) = 0;
    virtual void                _SetTransparency (DwgTransparency transparency) = 0;
    virtual void                _SetMaterial (DwgDbObjectIdCR materialId) = 0;
    };  // IDwgDrawParameters
DEFINE_NO_NAMESPACE_TYPEDEFS (IDwgDrawParameters)

/*=================================================================================**//**
//! Options to control a toolkit on how to draw an entity when DwgDbEntity::Draw is called.
//! These options may come from viewports or user prefs, and therefore are per viewport or per file.
//! @see DwgDrawParameters
+===============+===============+===============+===============+===============+======*/
struct IDwgDrawOptions
    {
    virtual DwgGiRegenType  _GetRegenType () = 0;
    virtual DwgDbDatabaseP  _GetDatabase () = 0;
    virtual DwgDbObjectId   _GetViewportId () = 0;
    virtual DVec3d          _GetViewDirection () = 0;
    virtual DVec3d          _GetCameraUpDirection () = 0;
    virtual DPoint3d        _GetCameraLocation () = 0;
    virtual size_t          _GetNumberOfIsolines () = 0;
    virtual bool            _GetViewportRange (DRange2dR area) { return false; }
    };  // DwgDrawOptions
DEFINE_NO_NAMESPACE_TYPEDEFS (IDwgDrawOptions)


/*=================================================================================**//**
// ! A helper class to drop shapes, i.e. text using a shapfile, to an array of linestrings
+===============+===============+===============+===============+===============+======*/
struct ShapeTextProcessor : NonCopyableClass
    {
private:
    DwgGiTextStyleR     m_textstyle;
    DwgStringCR         m_textstring;

public:
    ShapeTextProcessor (DwgGiTextStyleR styleIn, DwgStringCR stringIn) :  m_textstyle(styleIn), m_textstring(stringIn) { }
    DWGDB_EXPORT DwgDbStatus    Drop (bvector<DPoint3dArray>& linestringsOut, double deviation = 0.0);
    DWGDB_EXPORT DwgDbStatus    GetExtents (DRange2dR extentsOut, bool bearings = false, bool raw = true);
    };  // ShapeTextProcessor

END_DWGDB_NAMESPACE
//__PUBLISH_SECTION_END__
