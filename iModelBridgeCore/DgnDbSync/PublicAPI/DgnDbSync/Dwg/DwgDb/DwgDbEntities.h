/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbSync/Dwg/DwgDb/DwgDbEntities.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include    <DgnDbSync/Dwg/DwgDb/DwgDbObjects.h>
#include    <DgnDbSync/Dwg/DwgDb/DwgDrawables.h>

#ifdef DWGTOOLKIT_OpenDwg
#include    <Teigha/Core/Include/Entities.h>
#include    <Teigha/Core/Include/DbPointCloudObj/DbPointCloudEx.h>
#elif DWGTOOLKIT_RealDwg
#include    <RealDwg/Base/dbents.h>
#include    <RealDwg/Base/dbelipse.h>
#include    <RealDwg/Base/dbhatch.h>
#include    <RealDwg/Base/dbregion.h>
#include    <RealDwg/Base/dbspline.h>
#include    <RealDwg/Base/imgdef.h>
#include    <RealDwg/Base/imgent.h>
#include    <RealDwg/Base/AcDbPointCloudApi.h>
#include    <RealDwg/Base/AcDbPointCloudEx.h>
#else
    #error  "Must define DWGTOOLKIT!!!"
#endif

#include    <Geom/GeomApi.h>

BEGIN_DWGDB_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbEntity : public DWGDB_EXTENDCLASS(Entity)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(Entity)

public:
    DWGDB_EXPORT uint16_t           GetColorIndex () const;
    DWGDB_EXPORT DwgCmColor         GetColor () const;
    DWGDB_EXPORT DwgCmEntityColor   GetEntityColor () const;
    DWGDB_EXPORT DwgDbObjectId      GetLayerId () const;
    DWGDB_EXPORT DwgDbObjectId      GetLinetypeId () const;
    DWGDB_EXPORT double             GetLinetypeScale () const;
    DWGDB_EXPORT DwgDbLineWeight    GetLineweight () const;
    DWGDB_EXPORT DwgDbObjectId      GetMaterialId () const;
    DWGDB_EXPORT DwgTransparency    GetTransparency () const;
    DWGDB_EXPORT DwgDbVisibility    GetVisibility () const;
    DWGDB_EXPORT void               GetEcs (TransformR ecs) const;
    DWGDB_EXPORT DwgDbStatus        GetGripPoints (DPoint3dArrayR points, DwgDbIntArrayP snapModes = nullptr, DwgDbIntArrayP geomIds = nullptr) const;
    DWGDB_EXPORT DwgDbStatus        GetRange (DRange3dR range) const;
    DWGDB_EXPORT void               Draw (IDwgDrawGeometryR geometry, IDwgDrawOptionsR options, IDwgDrawParametersR params);
    DWGDB_EXPORT void               List () const;
    DWGDB_EXPORT DwgGiDrawablePtr   GetDrawable ();
    };  // DwgDbEntity
DWGDB_DEFINE_OBJECTPTR (Entity)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbLine : public DWGDB_EXTENDCLASS(Line)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(Line)

public:
    DWGDB_EXPORT DPoint3d   GetStartPoint () const;
    DWGDB_EXPORT DPoint3d   GetEndPoint () const;
    DWGDB_EXPORT DVec3d     GetNormal () const;
    DWGDB_EXPORT double     GetThickness () const;
    };  // DwgDbLine
DWGDB_DEFINE_OBJECTPTR (Line)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbPolyline : public DWGDB_EXTENDCLASS(Polyline)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(Polyline)

public:
    DWGDB_EXPORT bool       IsClosed () const;
    DWGDB_EXPORT size_t     GetNumPoints () const;
    DWGDB_EXPORT DPoint3d   GetPointAt (size_t index) const;
    DWGDB_EXPORT bool       GetWidthsAt (size_t index, double& start, double& end) const;
    DWGDB_EXPORT DPoint2d   GetWidthsAt (size_t index) const;
    DWGDB_EXPORT bool       HasWidth () const;
    DWGDB_EXPORT bool       GetConstantWidth (double& width) const;
    DWGDB_EXPORT bool       HasBulges () const;
    DWGDB_EXPORT double     GetBulgeAt (size_t index) const;
    DWGDB_EXPORT double     GetElevation () const;
    DWGDB_EXPORT double     GetThickness () const;
    DWGDB_EXPORT bool       HasPlinegen () const;
    DWGDB_EXPORT DVec3d     GetNormal () const;
    DWGDB_EXPORT void       GetEcs (TransformR ecs) const;

//__PUBLISH_SECTION_END__
    DwgDbStatus             SetFromGiPolyline (DWGGI_TypeCR(Polyline) giPolyline);
//__PUBLISH_SECTION_START__
    };  // DwgDbPolyline
DWGDB_DEFINE_OBJECTPTR (Polyline)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDb2dPolyline : public DWGDB_EXTENDCLASS(2dPolyline)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(2dPolyline)

public:
    DWGDB_EXPORT bool       IsClosed () const;
    DWGDB_EXPORT size_t     GetNumPoints () const;
    DWGDB_EXPORT bool       GetConstantWidth (double& width) const;
    DWGDB_EXPORT double     GetElevation () const;
    DWGDB_EXPORT double     GetThickness () const;
    DWGDB_EXPORT bool       HasPlinegen () const;
    DWGDB_EXPORT DVec3d     GetNormal () const;
    DWGDB_EXPORT void       GetEcs (TransformR ecs) const;
    DWGDB_EXPORT DwgDbObjectIterator    GetVertexIterator () const;
    };  // DwgDb2dPolyline
DWGDB_DEFINE_OBJECTPTR (2dPolyline)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDb3dPolyline : public DWGDB_EXTENDCLASS(3dPolyline)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(3dPolyline)

public:
    enum Type
        {
        Simple      = DWGDB_SDKENUM_DB(k3dSimplePoly),
        QuadSpline  = DWGDB_SDKENUM_DB(k3dQuadSplinePoly),
        CubicSpline = DWGDB_SDKENUM_DB(k3dCubicSplinePoly),
        };  // Type

    DWGDB_EXPORT Type                   GetType () const;
    DWGDB_EXPORT DwgDbObjectIterator    GetVertexIterator () const;
    DWGDB_EXPORT DwgDbStatus            Straighten ();
    };  // DwgDb3dPolyline
DWGDB_DEFINE_OBJECTPTR (3dPolyline)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbArc : public DWGDB_EXTENDCLASS(Arc)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(Arc)

public:
    DWGDB_EXPORT DPoint3d   GetCenter () const;
    DWGDB_EXPORT double     GetRadius () const;
    DWGDB_EXPORT double     GetStartAngle () const;
    DWGDB_EXPORT double     GetEndAngle () const;
    DWGDB_EXPORT double     GetTotalAngle () const;
    DWGDB_EXPORT DVec3d     GetNormal () const;
    DWGDB_EXPORT double     GetThickness () const;
    };  // DwgDbArc
DWGDB_DEFINE_OBJECTPTR (Arc)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbCircle : public DWGDB_EXTENDCLASS(Circle)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(Circle)

public:
    DWGDB_EXPORT DPoint3d   GetCenter () const;
    DWGDB_EXPORT double     GetDiameter () const;
    DWGDB_EXPORT DVec3d     GetNormal () const;
    DWGDB_EXPORT double     GetThickness () const;
    };  // DwgDbCircle
DWGDB_DEFINE_OBJECTPTR (Circle)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbEllipse : public DWGDB_EXTENDCLASS(Ellipse)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(Ellipse)

public:
    DWGDB_EXPORT DPoint3d   GetCenter () const;
    DWGDB_EXPORT double     GetMajorRadius () const;
    DWGDB_EXPORT double     GetMinorRadius () const;
    DWGDB_EXPORT DVec3d     GetMajorAxis () const;
    DWGDB_EXPORT DVec3d     GetMinorAxis () const;
    DWGDB_EXPORT DVec3d     GetNormal () const;
    DWGDB_EXPORT double     GetStartAngle () const;
    DWGDB_EXPORT double     GetEndAngle () const;
    };  // DwgDbEllipse
DWGDB_DEFINE_OBJECTPTR (Ellipse)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbSpline : public DWGDB_EXTENDCLASS(Spline)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(Spline)

public:
    DWGDB_EXPORT DwgDbStatus    GetNurbsData (int16_t& degree, bool& rational, bool& closed, bool& periodic, DPoint3dArrayR poles, DwgDbDoubleArrayR knots, DwgDbDoubleArrayR weights, double& poleTol, double& knotTol) const;
    };  // DwgDbSpline
DWGDB_DEFINE_OBJECTPTR (Spline)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbAttribute : public DWGDB_EXTENDCLASS(Attribute)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(Attribute)

public:
    DWGDB_EXPORT DPoint3d   GetOrigin () const;
    DWGDB_EXPORT DVec3d     GetNormal () const;
    DWGDB_EXPORT double     GetThickness () const;
    DWGDB_EXPORT bool       IsInvisible () const;
    DWGDB_EXPORT bool       IsConstant () const;
    DWGDB_EXPORT bool       IsMTextAttribute () const;
    DWGDB_EXPORT bool       IsPreset () const;
    DWGDB_EXPORT bool       IsVerifiable () const;
    DWGDB_EXPORT bool       IsLocked () const;
    DWGDB_EXPORT DwgString  GetTag () const;
    DWGDB_EXPORT bool       GetValueString (DwgStringR value) const;
    DWGDB_EXPORT DwgDbStatus    SetFrom (DwgDbAttributeDefinition const* attrdef, TransformCR toBlockRef);
    };  // DwgDbAttribute
DWGDB_DEFINE_OBJECTPTR (Attribute)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbAttributeDefinition : public DWGDB_EXTENDCLASS(AttributeDefinition)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(AttributeDefinition)

public:
    DWGDB_EXPORT DPoint3d   GetOrigin () const;
    DWGDB_EXPORT DVec3d     GetNormal () const;
    DWGDB_EXPORT double     GetThickness () const;
    DWGDB_EXPORT bool       IsInvisible () const;
    DWGDB_EXPORT bool       IsConstant () const;
    DWGDB_EXPORT bool       IsMTextAttributeDefinition () const;
    DWGDB_EXPORT bool       IsPreset () const;
    DWGDB_EXPORT bool       IsVerifiable () const;
    DWGDB_EXPORT DwgString  GetTag () const;
    DWGDB_EXPORT bool       GetValueString (DwgStringR value) const;
    };  // DwgDbAttributeDefinition
DWGDB_DEFINE_OBJECTPTR (AttributeDefinition)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbViewport : public DWGDB_EXTENDCLASS(Viewport)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(Viewport)

public:
    DWGDB_EXPORT bool           IsOn () const;
    DWGDB_EXPORT bool           IsGridEnabled () const;
    DWGDB_EXPORT bool           IsUcsIconEnabled () const;
    DWGDB_EXPORT bool           IsFrontClipEnabled () const;
    DWGDB_EXPORT bool           IsFrontClipAtEye () const;
    DWGDB_EXPORT double         GetFrontClipDistance () const;
    DWGDB_EXPORT bool           IsBackClipEnabled () const;
    DWGDB_EXPORT double         GetBackClipDistance () const;
    DWGDB_EXPORT DwgDbObjectId  GetClipEntity () const;
    DWGDB_EXPORT bool           IsPerspectiveEnabled () const;
    DWGDB_EXPORT bool           IsDefaultLightingOn () const;
    DWGDB_EXPORT bool           IsTransparentOn () const;
    DWGDB_EXPORT bool           GetFillMode () const;
    DWGDB_EXPORT DwgDbObjectId  GetBackground () const;
    DWGDB_EXPORT DwgDbObjectId  GetVisualStyle () const;
    DWGDB_EXPORT DVec3d         GetViewDirection () const;
    DWGDB_EXPORT DwgDbStatus    GetUcs (DPoint3dR origin, DVec3dR xAxis, DVec3d yAxis) const;
    DWGDB_EXPORT bool           IsUcsSavedWithViewport () const;
    DWGDB_EXPORT double         GetHeight () const;
    DWGDB_EXPORT double         GetViewHeight () const;
    DWGDB_EXPORT double         GetWidth () const;
    DWGDB_EXPORT double         GetLensLength () const;
    DWGDB_EXPORT double         GetViewTwist () const;
    DWGDB_EXPORT double         GetUcsElevation () const;
    DWGDB_EXPORT DPoint3d       GetCenterPoint () const;
    DWGDB_EXPORT DPoint2d       GetViewCenter () const;
    DWGDB_EXPORT DPoint3d       GetViewTarget () const;
    DWGDB_EXPORT DwgDbStatus    SetCenterPoint (DPoint3dCR center);
    DWGDB_EXPORT DwgDbStatus    SetViewCenter (DPoint2dCR center);
    DWGDB_EXPORT DwgDbStatus    SetHeight (double height);
    DWGDB_EXPORT DwgDbStatus    SetViewHeight (double height);
    DWGDB_EXPORT DPoint2d       GetGridIncrements () const;
    DWGDB_EXPORT DPoint2d       GetSnapIncrements () const;
    DWGDB_EXPORT DPoint2d       GetSnapBase () const;
    DWGDB_EXPORT double         GetSnapAngle () const;
    DWGDB_EXPORT SnapIsoPair    GetSnapPair () const;
    DWGDB_EXPORT bool           IsSnapEnabled () const;
    DWGDB_EXPORT bool           IsIsometricSnapEnabled () const;
    DWGDB_EXPORT int16_t        GetGridMajor () const;
    DWGDB_EXPORT DwgDbStatus    SetWidth (double width);
    DWGDB_EXPORT DwgDbStatus    GetFrozenLayers (DwgDbObjectIdArrayR ids) const;
    DWGDB_EXPORT bool           IsLayerFrozen (DwgDbObjectIdCR layerId) const;
    DWGDB_EXPORT double         GetCustomScale () const;
    DWGDB_EXPORT DwgDbStatus    GetAnnotationScale (double& scale) const;
    };  // DwgDbViewport
DWGDB_DEFINE_OBJECTPTR (Viewport)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbBlockReference : public DWGDB_EXTENDCLASS(BlockReference)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(BlockReference)

public:
    DWGDB_EXPORT DwgDbObjectId  GetBlockTableRecordId () const;
    DWGDB_EXPORT DPoint3d       GetPosition () const;
    DWGDB_EXPORT void           GetBlockTransform (TransformR tranform) const;
    DWGDB_EXPORT DPoint3d       GetScaleFactors () const;
    DWGDB_EXPORT DVec3d         GetNormal () const;
    DWGDB_EXPORT DwgDbStatus    ExplodeToOwnerSpace () const;
    DWGDB_EXPORT DwgDbStatus    GetExtentsBestFit (DRange3dR extents, TransformCR parentXform = Transform::FromIdentity()) const;
    DWGDB_EXPORT bool           IsXAttachment (WStringP blockName = nullptr, WStringP path = nullptr) const;
    DWGDB_EXPORT DwgDbStatus    OpenSpatialFilter (DwgDbSpatialFilterPtr& filterOut, DwgDbOpenMode mode) const;
    DWGDB_EXPORT DwgDbObjectIterator    GetAttributeIterator () const;
    };  // DwgDbBlockReference
DWGDB_DEFINE_OBJECTPTR (BlockReference)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbHatch : public DWGDB_EXTENDCLASS(Hatch)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(Hatch)

public:
    enum class HatchType        // == Super::HatchObjectType
        {
        Hatch               = 0,
        Gradient            = 1,
        };  // HatchType

    enum class PatternType      // == Super::HatchPatternType
        {
        UserDefined         = 0,
        PreDefined          = 1,
        CustomDefined       = 2,
        };  // PatternType

    DWGDB_EXPORT bool           IsGradient () const;
    DWGDB_EXPORT double         GetGradientAngle () const;
    DWGDB_EXPORT double         GetGradientShift () const;
    DWGDB_EXPORT double         GetGradientTint () const;
    DWGDB_EXPORT DwgString      GetGradientName () const;
    DWGDB_EXPORT size_t         GetGradientColors (DwgColorArrayR colorsOut, DwgDbDoubleArrayR valuesOut);
    DWGDB_EXPORT size_t         GetPatternDefinitions (DwgPatternArrayR patternsOut) const;
    DWGDB_EXPORT PatternType    GetPatternType () const;
    DWGDB_EXPORT DwgDbStatus    SetPattern (PatternType type, DwgStringCR name);
    DWGDB_EXPORT HatchType      GetHatchType () const;
    DWGDB_EXPORT DwgDbStatus    SetHatchType (HatchType type);
    
    };  // DwgDbHatch
DWGDB_DEFINE_OBJECTPTR (Hatch)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          06/16
+===============+===============+===============+===============+===============+======*/
class DwgDbRasterImage : public DWGDB_EXTENDCLASS(RasterImage)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(RasterImage)
    
public:
    enum class ClipType     // == T_Super::ClipBoundaryType
        {
        Invalid,
        Rectangle,
        Polygon,
        };  // ClipType

    DWGDB_EXPORT DwgDbStatus    GetFileName (DwgStringR sourceFile, DwgStringP activeFile = nullptr) const;
    DWGDB_EXPORT DVec2d         GetImageSize (bool cachedValue = false) const;
    DWGDB_EXPORT DwgDbObjectId  GetImageDefinitionId () const;
    DWGDB_EXPORT DwgDbStatus    GetVertices (DPoint3dArrayR vertices) const;
    DWGDB_EXPORT DVec2d         GetScale () const;
    DWGDB_EXPORT void           GetOrientation (DPoint3dR origin, DVec3dR xAxis, DVec3dR yAxis) const;
    //! Get raster transformation either in pixel or model coordinates, with origin at lower-left corner.
    DWGDB_EXPORT void           GetOrientation (TransformR orientation, bool pixelsToModel = false) const;
    //! Get raster transformation in model coordinates, with origin at upper-left corner.
    DWGDB_EXPORT DwgDbStatus    GetModelTransform (TransformR pixelToModel) const;
    //! Get clipping points in pixel coordinates
    DWGDB_EXPORT size_t         GetClippingBoundary (DPoint3dArrayR out) const;
    //! Get clipping points in model coordinates
    DWGDB_EXPORT size_t         GetClippingBoundary (DPoint2dArrayR out) const;
    DWGDB_EXPORT ClipType       GetClipBoundaryType () const;
    DWGDB_EXPORT bool           IsShownClipped () const;
    DWGDB_EXPORT bool           IsClipInverted () const;
    DWGDB_EXPORT bool           IsClipped () const;
    DWGDB_EXPORT bool           IsDisplayed () const;
    };  // DwgDbRasterImage
DWGDB_DEFINE_OBJECTPTR (RasterImage)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          06/16
+===============+===============+===============+===============+===============+======*/
class PointCloudDataQuery
    {
    friend class IPointsProcessor;

public:
    enum Type       // ==> IAcDbPointCloudDataBuffer::DataType
        {
        Intensity      = 0x00000001,
        Classification = 0x00000002,
        Color          = 0x00000004,
        Normal         = 0x00000008,
        };

    typedef uint8_t     Rgba[4];

    DWGDB_EXPORT size_t         GetNumPoints () const;
    DWGDB_EXPORT double const*  GetPointsAsDoubles () const;
    DWGDB_EXPORT double const*  GetNormalsAsDoubles () const;
    DWGDB_EXPORT Rgba const*    GetColors () const;
    DWGDB_EXPORT uint8_t const* GetIntensity () const;
    DWGDB_EXPORT uint8_t const* GetClassifications () const;
    DWGDB_EXPORT void           GetTransform (TransformR matrix) const;

//__PUBLISH_SECTION_END__
private:
#ifdef DWGTOOLKIT_RealDwg
    PointCloudDataQuery () : m_pointCloudDataBuffer(nullptr) {}
    PointCloudDataQuery (const IAcDbPointCloudDataBuffer* in) : m_pointCloudDataBuffer(in) {}
    IAcDbPointCloudDataBuffer const*    m_pointCloudDataBuffer;
#endif
//__PUBLISH_SECTION_START__
    };  // PointCloudDataQuery
DEFINE_NO_NAMESPACE_TYPEDEFS(PointCloudDataQuery)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          06/16
+===============+===============+===============+===============+===============+======*/
class IPointsProcessor 
#ifdef DWGTOOLKIT_OpenDwg
    {
#elif DWGTOOLKIT_RealDwg
            : public IAcDbPointCloudPointProcessor
    {
    DEFINE_T_SUPER (IAcDbPointCloudPointProcessor)
#endif
public:
    enum Status             // == T_Super::ProcessState
        {
        Abort,
        Continue
        };
        
    DWGDB_EXPORT virtual Status     _Process (PointCloudDataQueryCP accessor) = 0;

//__PUBLISH_SECTION_END__
#ifdef DWGTOOLKIT_RealDwg
private:
    DWGDB_EXPORT virtual ProcessSate process (const IAcDbPointCloudDataBuffer* buffer) override;
#endif
//__PUBLISH_SECTION_START__
    };  // IPointsProcessor

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          06/16
+===============+===============+===============+===============+===============+======*/
class IPointsFilter
#ifdef DWGTOOLKIT_OpenDwg
    {
#elif DWGTOOLKIT_RealDwg
            : public IAcDbPointCloudSpatialFilter
    {
    DEFINE_T_SUPER (DWGDB_SDKNAME(,IAcDbPointCloudSpatialFilter))
#endif
public:
    enum Result         // == T_Super::FilterResult
        {
        Inside,
        Outside,
        Intersects
        };
    };  // IPointsFilter

/*=================================================================================**//**
*! This class is currently only supported for RealDWG.
* @bsiclass                                                     Don.Fu          06/16
+===============+===============+===============+===============+===============+======*/
class DwgDbPointCloudEx : public DWGDB_EXTENDCLASS(PointCloudEx)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(PointCloudEx)
    
public:
    DWGDB_EXPORT DwgDbStatus    TraversePointData (IPointsProcessor* processor, IPointsFilter* filter, PointCloudDataQuery::Type type, int lod) const;
    DWGDB_EXPORT void           GetEcs (TransformR ecs) const;
    DWGDB_EXPORT DPoint3d       GetLocation () const;
    DWGDB_EXPORT double         GetScale () const;
    DWGDB_EXPORT double         GetRotation () const;
    DWGDB_EXPORT DwgDbStatus    GetMinDistPrecision (double& prec) const;
    DWGDB_EXPORT void           GetNativeCloudExtent (DRange3dR extent) const;
    DWGDB_EXPORT bool           IsSupported () const;
    };  // DwgDbPointCloudEx
DWGDB_DEFINE_OBJECTPTR (PointCloudEx)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbPoint : public DWGDB_EXTENDCLASS(Point)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(Point)

public:
    DWGDB_EXPORT DPoint3d   GetPosition () const;
    DWGDB_EXPORT DVec3d     GetNormal () const;
    DWGDB_EXPORT double     GetThickness () const;
    };  // DwgDbPoint
DWGDB_DEFINE_OBJECTPTR (Point)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbSolid : public DWGDB_EXTENDCLASS(Solid)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(Solid)

public:
    DWGDB_EXPORT DVec3d     GetNormal () const;
    DWGDB_EXPORT double     GetThickness () const;
    };  // DwgDbSolid
DWGDB_DEFINE_OBJECTPTR (Solid)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbShape : public DWGDB_EXTENDCLASS(Shape)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(Shape)

public:
    DWGDB_EXPORT DVec3d     GetNormal () const;
    DWGDB_EXPORT double     GetThickness () const;
    };  // DwgDbSolid
DWGDB_DEFINE_OBJECTPTR (Shape)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbTrace : public DWGDB_EXTENDCLASS(Trace)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(Trace)

public:
    DWGDB_EXPORT DVec3d     GetNormal () const;
    DWGDB_EXPORT double     GetThickness () const;
    };  // DwgDbTrace
DWGDB_DEFINE_OBJECTPTR (Trace)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbFace : public DWGDB_EXTENDCLASS(Face)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(Face)

public:
    DWGDB_EXPORT DwgDbStatus    GetVertexAt (DPoint3dR point, uint16_t index) const;
    DWGDB_EXPORT DwgDbStatus    GetTransform (TransformR ecs) const;
    DWGDB_EXPORT bool           IsPlanar () const;
    };  // DwgDbFace
DWGDB_DEFINE_OBJECTPTR (Face)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbRegion : public DWGDB_EXTENDCLASS(Region)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(Region)

public:
    DWGDB_EXPORT DVec3d     GetNormal () const;
    };  // DwgDbRegion
DWGDB_DEFINE_OBJECTPTR (Region)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbText : public DWGDB_EXTENDCLASS(Text)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(Text)

public:
    DWGDB_EXPORT DPoint3d   GetPosition () const;
    DWGDB_EXPORT DVec3d     GetNormal () const;
    DWGDB_EXPORT double     GetThickness () const;
    DWGDB_EXPORT DwgString  GetTextString () const;
    };  // DwgDbText
DWGDB_DEFINE_OBJECTPTR (Text)


END_DWGDB_NAMESPACE
//__PUBLISH_SECTION_END__
