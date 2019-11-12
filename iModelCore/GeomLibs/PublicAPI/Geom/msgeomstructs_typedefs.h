/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

// Note: base "interop" types like DPoint3d, DPoint2d, DVec3d, DVec2d, RotMatrix, and Transform are in the Bentley namespace.
// All other geometry types are in the BentleyApi (version-specific) namespace.
#define BEGIN_BENTLEY_GEOMETRY_NAMESPACE    BEGIN_BENTLEY_NAMESPACE
#define END_BENTLEY_GEOMETRY_NAMESPACE      END_BENTLEY_NAMESPACE

#if !defined (mdl_type_resource_generator) && !defined (mdl_resource_compiler)

  #if defined(jmdlgeom_internal)
      #define GEOMDLLIMPEXP EXPORT_ATTRIBUTE
      #define GEOMDLLVIRTUAL EXPORT_ATTRIBUTE virtual
  #else
      #define GEOMDLLIMPEXP IMPORT_ATTRIBUTE
      #define GEOMDLLVIRTUAL IMPORT_ATTRIBUTE virtual
  #endif


  // struct in Bentley namespace, with struct name hidden by typedef...
  #define DEFINE_GEOM_STRUCT(_TypeDefName_,_StructName_) \
  BEGIN_BENTLEY_GEOMETRY_NAMESPACE \
  struct _StructName_;\
  typedef struct _StructName_ _TypeDefName_;\
  typedef struct _StructName_ * _TypeDefName_##P;\
  typedef struct _StructName_ const * _TypeDefName_##CP;\
  typedef struct _StructName_ & _TypeDefName_##R;\
  typedef struct _StructName_ const & _TypeDefName_##CR;\
  END_BENTLEY_GEOMETRY_NAMESPACE
  // struct in Bentley namespace, with struct name directly by user code ...
  #define DEFINE_GEOM_STRUCT1(_StructName_) \
  BEGIN_BENTLEY_GEOMETRY_NAMESPACE \
  struct _StructName_;\
  typedef struct _StructName_ * _StructName_##P;\
  typedef struct _StructName_ const * _StructName_##CP;\
  typedef struct _StructName_ & _StructName_##R;\
  typedef struct _StructName_ const & _StructName_##CR;\
  END_BENTLEY_GEOMETRY_NAMESPACE

  // struct in Bentley namespace, with struct name directly by user code ...
  #define DEFINE_GEOM_STRUCT_INTERNAL(_StructName_) \
  BEGIN_BENTLEY_GEOMETRY_INTERNAL_NAMESPACE \
  struct _StructName_;\
  typedef struct _StructName_ * _StructName_##P;\
  typedef struct _StructName_ const * _StructName_##CP;\
  typedef struct _StructName_ & _StructName_##R;\
  typedef struct _StructName_ const & _StructName_##CR;\
  END_BENTLEY_GEOMETRY_INTERNAL_NAMESPACE

  // struct within Bentley namespace, with struct name(P,CP,R,CR) as typedef names
  #define DEFINE_GEOM_STRUCT2(_ParentStructName_,_StructName_) \
  BEGIN_BENTLEY_GEOMETRY_NAMESPACE \
  struct _StructName_;\
  typedef struct _StructName_ * _StructName_##P;\
  typedef struct _StructName_ const * _StructName_##CP;\
  typedef struct _StructName_ & _StructName_##R;\
  typedef struct _StructName_ const & _StructName_##CR;\
  END_BENTLEY_GEOMETRY_NAMESPACE

  #define DEFINE_GEOM_CLASS(_ClassName_) \
  BEGIN_BENTLEY_GEOMETRY_NAMESPACE \
  class _ClassName_;\
  typedef _ClassName_* _ClassName_##P;\
  typedef _ClassName_ const * _ClassName_##CP;\
  typedef _ClassName_       & _ClassName_##R;\
  typedef _ClassName_ const & _ClassName_##CR;\
  END_BENTLEY_GEOMETRY_NAMESPACE

  BENTLEY_NAMESPACE_TYPEDEFS (DPoint2d)
  BENTLEY_NAMESPACE_TYPEDEFS (DPoint3d)

  #if defined (LEGACY_DVEC3D) && !defined (NO_LEGACY_DVEC3D)
  #define     _DVEC3D_DPOINT3D_SYNONYM_
  #define    MS_DVEC3D_DEFINED
  //#pragma message ("LEGACY_DVEC3D")
  BENTLEY_NAMESPACE_TYPEDEFS (DVec3d)
  BENTLEY_NAMESPACE_TYPEDEFS (DVec2d)
  #else
  //#pragma message ("STRONG_DVEC3D")
  BENTLEY_NAMESPACE_TYPEDEFS (DVec3d)
  BENTLEY_NAMESPACE_TYPEDEFS (DVec2d)
  #endif

#else
  #define DEFINE_GEOM_STRUCT(_TypeDefName_,_StructName_)
  #define DEFINE_GEOM_STRUCT1(_StructName_)
  #define DEFINE_GEOM_STRUCT_INTERNAL(_StructName_)
  #define DEFINE_GEOM_STRUCT2(_ParentStructName_,_StructName_)
  #define DEFINE_GEOM_CLASS(_ClassName_)

  #define GEOMDLLIMPEXP
  #define GEOMDLLVIRTUAL
    
#endif

DEFINE_GEOM_STRUCT1 (ClipPlane)
DEFINE_GEOM_STRUCT1 (ClipPlaneSet)
DEFINE_GEOM_STRUCT1 (ConvexClipPlaneSet)
DEFINE_GEOM_STRUCT1 (DPoint4d)
DEFINE_GEOM_STRUCT1 (DMatrix4d)
DEFINE_GEOM_STRUCT1 (DMap4d)
DEFINE_GEOM_STRUCT1 (DSpiral2dBase)
DEFINE_GEOM_STRUCT1 (DSpiral2dPlacement)
DEFINE_GEOM_STRUCT1 (GraphicsPoint)
DEFINE_GEOM_STRUCT1 (DRange1d)
DEFINE_GEOM_STRUCT1 (GraphicsPointArray)
/*__PUBLISH_SECTION_END__*/
DEFINE_GEOM_STRUCT (SmallSetRange1d,_smallsetrange1d)

#ifdef TestNamespaceDefinitions
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
struct eQuark;
END_BENTLEY_GEOMETRY_NAMESPACE
ADD_BENTLEY_TYPEDEFS1(Bentley,eQuark,DQuark,struct)

ADD_BENTLEY_TYPEDEFS1(Bentley,eBoson,EBoson,struct)
#endif

/*__PUBLISH_SECTION_START__*/
DEFINE_GEOM_STRUCT (Range1d,_range1d)
DEFINE_GEOM_STRUCT (DMatrix3d,_dMatrix3d)

/*__PUBLISH_SECTION_END__*/
DEFINE_GEOM_STRUCT (DTransform3d,_dTransform3d)
/*__PUBLISH_SECTION_START__*/

DEFINE_GEOM_STRUCT1 (DRange3d)
DEFINE_GEOM_STRUCT1 (LocalRange)
DEFINE_GEOM_STRUCT1 (DRange2d)

/*__PUBLISH_SECTION_END__*/
DEFINE_GEOM_STRUCT (Accumulator,_accumulator)
typedef struct _dRange3d RangeCube;
DEFINE_GEOM_STRUCT (ProximityData,_proximityData)
/*__PUBLISH_SECTION_START__*/

DEFINE_GEOM_STRUCT1 (DPlane3d)
DEFINE_GEOM_STRUCT1 (DEllipse3d)
DEFINE_GEOM_STRUCT1 (DSegment3d)
DEFINE_GEOM_STRUCT1 (DBilinearPatch3d)
DEFINE_GEOM_STRUCT1 (DTriangle3d)
DEFINE_GEOM_STRUCT1 (DPoint3dDVec3dDVec3d)
DEFINE_GEOM_STRUCT1 (DCatenary3dPlacement)
DEFINE_GEOM_STRUCT1 (BSIQuadraturePoints)
DEFINE_GEOM_STRUCT1 (DRay3d)
DEFINE_GEOM_STRUCT1 (DrapeSegment)
DEFINE_GEOM_STRUCT1 (DPoint3dDoubleUVCurveArrays)
DEFINE_GEOM_STRUCT1 (DPoint3dDoubleUVArrays)
DEFINE_GEOM_STRUCT1 (DPoint3dDoubleArrays)


DEFINE_GEOM_STRUCT (DEllipsoid3d,_dEllipsoid3d)
DEFINE_GEOM_STRUCT (DConic4d,_dConic4d)
DEFINE_GEOM_STRUCT (DCone3d,_dCone3d)
DEFINE_GEOM_STRUCT (DToroid3d,_dToroid3d)
DEFINE_GEOM_STRUCT (DDisk3d,_dDisk3d)
DEFINE_GEOM_STRUCT1 (FPoint3d)
DEFINE_GEOM_STRUCT1 (FVec3d)
DEFINE_GEOM_STRUCT (FPoint2d,_fPoint2d)
DEFINE_GEOM_STRUCT1 (FRange3d)
DEFINE_GEOM_STRUCT (FRange2d,_fRange2d)

BENTLEY_NAMESPACE_TYPEDEFS (Transform)
BENTLEY_NAMESPACE_TYPEDEFS (RotMatrix)
/*__PUBLISH_SECTION_END__*/
DEFINE_GEOM_STRUCT (DEllipse4d,_dEllipse4d)
DEFINE_GEOM_CLASS  (DMoments3d)
/*__PUBLISH_SECTION_START__*/

DEFINE_GEOM_STRUCT (GeoPoint, GeoPoint)
DEFINE_GEOM_STRUCT (GeoPoint2d, GeoPoint2d)
DEFINE_GEOM_STRUCT1 (MSBsplineCurve)
DEFINE_GEOM_STRUCT1 (RefCountedMSBsplineCurve)
DEFINE_GEOM_STRUCT1 (CurveConstraint)
DEFINE_GEOM_STRUCT1 (ConstrainedConstruction)
DEFINE_GEOM_STRUCT1 (BezierTriangleDPoint3d)
DEFINE_GEOM_STRUCT1 (BezierTriangleD)
DEFINE_GEOM_STRUCT1 (BezierTriangleOfTriangles)
DEFINE_GEOM_STRUCT1 (BezierCurveDPoint3d)
DEFINE_GEOM_STRUCT1 (DSegment1d)
DEFINE_GEOM_STRUCT1 (DSegment4d)
DEFINE_GEOM_STRUCT1 (TaggedPolygon)

DEFINE_GEOM_STRUCT1 (Angle)
DEFINE_GEOM_STRUCT1 (AngleInDegrees)
DEFINE_GEOM_STRUCT1 (YawPitchRollAngles)
DEFINE_GEOM_STRUCT1 (YawPitchRollAnglesInDegrees)

DEFINE_GEOM_STRUCT (BsplineParam, bSplineParam)
DEFINE_GEOM_STRUCT (InterpolationParam, interpolationParam)

DEFINE_GEOM_STRUCT (BsplineDisplay, bsplineDisplay)
DEFINE_GEOM_STRUCT (TrimCurve, trimCurve)

DEFINE_GEOM_STRUCT (BsurfBoundary, bsurfBoundary)
DEFINE_GEOM_STRUCT1 (MSBsplineSurface)
DEFINE_GEOM_STRUCT1 (RefCountedMSBsplineSurface)
DEFINE_GEOM_STRUCT1 (BSurfPatch)
DEFINE_GEOM_STRUCT1 (PatchRangeData)
DEFINE_GEOM_STRUCT1 (BCurveSegment)
DEFINE_GEOM_STRUCT1 (IFacetOptions)
DEFINE_GEOM_STRUCT1 (CurveGapOptions)

DEFINE_GEOM_STRUCT1 (IGeometry)
DEFINE_GEOM_STRUCT1 (GeometryNode)
DEFINE_GEOM_STRUCT1 (ICurvePrimitive)
DEFINE_GEOM_STRUCT1 (CurveLocationDetail)
DEFINE_GEOM_STRUCT1 (PathLocationDetail)
DEFINE_GEOM_STRUCT1 (CurveAndSolidLocationDetail)
DEFINE_GEOM_STRUCT1 (CurveLocationDetailPair)
DEFINE_GEOM_STRUCT1 (CurveOffsetOptions)
DEFINE_GEOM_STRUCT1 (PartialCurveDetail)
DEFINE_GEOM_STRUCT1 (CurveVector)
DEFINE_GEOM_STRUCT1 (CurveVectorWithDistanceIndex)
DEFINE_GEOM_STRUCT1 (FacetFaceData)

DEFINE_GEOM_STRUCT1 (PolyfaceVisitor)
DEFINE_GEOM_STRUCT1 (IPolyfaceVisitorFilter)
DEFINE_GEOM_STRUCT1 (PolyfaceHeader)
DEFINE_GEOM_STRUCT1 (PolyfaceQuery)
DEFINE_GEOM_STRUCT1 (PolyfaceVectors)
DEFINE_GEOM_STRUCT1 (PolyfaceCoordinateMap)
DEFINE_GEOM_STRUCT1 (FacetEdgeLocationDetail)
DEFINE_GEOM_STRUCT1 (FacetEdgeLocationDetailPair)
DEFINE_GEOM_STRUCT1 (PolyfaceCoordinateAverageContext)
DEFINE_GEOM_STRUCT1 (NeighborVector)
DEFINE_GEOM_STRUCT1 (Neighbors)
DEFINE_GEOM_STRUCT1 (ISolidPrimitive)
DEFINE_GEOM_STRUCT1 (SolidLocationDetail)
DEFINE_GEOM_STRUCT1 (ISPCone)
DEFINE_GEOM_STRUCT1 (ISPBox)
DEFINE_GEOM_STRUCT1 (ISPSphere)
DEFINE_GEOM_STRUCT1 (ISPTorus)
DEFINE_GEOM_STRUCT1 (ISPExtrude)
DEFINE_GEOM_STRUCT1 (ISPRuledSurfaceBetweenChains)
DEFINE_GEOM_STRUCT1 (ISPSurfaceOfRotation)

DEFINE_GEOM_STRUCT1 (DgnTorusPipeDetail)
DEFINE_GEOM_STRUCT1 (DgnConeDetail)
DEFINE_GEOM_STRUCT1 (DgnBoxDetail)
DEFINE_GEOM_STRUCT1 (DgnSphereDetail)
DEFINE_GEOM_STRUCT1 (DgnExtrusionDetail)
DEFINE_GEOM_STRUCT1 (DgnRotationalSweepDetail)
DEFINE_GEOM_STRUCT1 (DgnRuledSweepDetail)

/*__PUBLISH_SECTION_END__*/
DEFINE_GEOM_STRUCT (BoundBox, boundBox)
DEFINE_GEOM_STRUCT (BoundCyl, boundCyl)
DEFINE_GEOM_STRUCT (BooleanData, booleandata)
/*__PUBLISH_SECTION_START__*/
DEFINE_GEOM_STRUCT1 (MSInterpolationCurve)

DEFINE_GEOM_STRUCT (CurveChain, curveChain)
DEFINE_GEOM_STRUCT (SurfaceChain, surfaceChain)

DEFINE_GEOM_STRUCT (MTGFragmentSorter, _MTGFragmentSorter)

//! Selector for mapping of parameters
#ifndef CurveParameterMapping_DEFINED_
#define CurveParameterMapping_DEFINED_
BEGIN_BENTLEY_NAMESPACE
enum CurveParameterMapping
    {
    CURVE_PARAMETER_MAPPING_BezierFraction    = 0,
    CURVE_PARAMETER_MAPPING_CurveKnot         = 1,
    CURVE_PARAMETER_MAPPING_CurveFraction     = 2
    };
END_BENTLEY_NAMESPACE
#endif // CurveParameterMapping_DEFINED_

DEFINE_GEOM_STRUCT1 (FacetLocationDetail)
DEFINE_GEOM_STRUCT1 (FacetLocationDetailPair)
DEFINE_GEOM_STRUCT (FloatRgb,FloatRgb)
DEFINE_GEOM_STRUCT (RgbFactor,RgbFactor)

#ifndef LocalCoordinateSelect_DEFINED_
#define LocalCoordinateSelect_DEFINED_
BEGIN_BENTLEY_NAMESPACE
enum LocalCoordinateSelect {
    //! Axis vectors in the transform are unit length.  Origin is at start of data
    LOCAL_COORDINATE_SCALE_UnitAxesAtStart,
    //! Axis vectors in the transform are unit length.  Origin is at lower left of range.
    LOCAL_COORDINATE_SCALE_UnitAxesAtLowerLeft,
    //! Axis vectors in the transform are full length of data in that direction.
    LOCAL_COORDINATE_SCALE_01RangeBothAxes,
    //! Axis vectors in the transform are the full length of data in the larger direction.
    LOCAL_COORDINATE_SCALE_01RangeLargerAxis
    };
END_BENTLEY_NAMESPACE
#endif // LocalCoordinateSelect_DEFINED_
