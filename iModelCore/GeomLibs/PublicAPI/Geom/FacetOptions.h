/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

//#include <Bentley/RefCounted.h>
//#include <Bentley/NonCopyableClass.h>
//#include <Bentley/bvector.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//! Enumeration of requested range of facet parameters.
enum FacetParamMode
{
//! Facet parameters are 0 to 1 in each direction
FACET_PARAM_01BothAxes    = 0,
//! Facet parameters are 0 to 1 in longer physical direction, 0 to smaller fraction in the other direction.
FACET_PARAM_01LargerAxis  = 1,
//! Facet parameters are (as best as possible) physical distances.
FACET_PARAM_Distance = 2
};



typedef RefCountedPtr<IFacetOptions>  IFacetOptionsPtr;


//! FacetOptions carries tolerance and count data to be used in making surface facets and curve chords.
struct IFacetOptions : public RefCountedBase
{
protected:

   GEOMAPI_VIRTUAL double _GetChordTolerance () const = 0;
   GEOMAPI_VIRTUAL void _SetChordTolerance (double chordTolerance) = 0;

   GEOMAPI_VIRTUAL double _GetMaxEdgeLength () const = 0;
   GEOMAPI_VIRTUAL void _SetMaxEdgeLength (double maxEdgeLength) = 0;

   GEOMAPI_VIRTUAL double _GetAngleTolerance () const = 0;
   GEOMAPI_VIRTUAL void _SetAngleTolerance (double normalAngleTolerance) = 0;

   GEOMAPI_VIRTUAL int _GetMaxPerBezier () const = 0;
   GEOMAPI_VIRTUAL void _SetMaxPerBezier (int count) = 0;

   GEOMAPI_VIRTUAL int _GetMinPerBezier () const = 0;
   GEOMAPI_VIRTUAL void _SetMinPerBezier (int count) = 0;

   GEOMAPI_VIRTUAL int _GetMaxPerFace () const = 0;
   GEOMAPI_VIRTUAL void _SetMaxPerFace (int maxPerFace) = 0;

   GEOMAPI_VIRTUAL bool _GetEdgeHiding () const = 0;
   GEOMAPI_VIRTUAL void _SetEdgeHiding (bool edgeHiding) = 0;

   GEOMAPI_VIRTUAL bool _GetNormalsRequired () const = 0;
   GEOMAPI_VIRTUAL void _SetNormalsRequired (bool normalsRequired) = 0;

   GEOMAPI_VIRTUAL bool _GetParamsRequired () const = 0;
   GEOMAPI_VIRTUAL void _SetParamsRequired (bool paramsRequired) = 0;

   GEOMAPI_VIRTUAL CurveParameterMapping _GetCurveParameterMapping () const = 0;
   GEOMAPI_VIRTUAL void _SetCurveParameterMapping (CurveParameterMapping mapping) = 0;

   GEOMAPI_VIRTUAL bool _GetVertexColorsRequired () const = 0;
   GEOMAPI_VIRTUAL void _SetVertexColorsRequired (bool vertexColorsRequired) = 0;

   GEOMAPI_VIRTUAL DVec3d _GetSilhouetteDirection () const = 0;
   GEOMAPI_VIRTUAL void _SetSilhouetteDirection (DVec3d silhouetteDirection) = 0;

   GEOMAPI_VIRTUAL DPoint3d _GetSilhouetteOrigin () const = 0;
   GEOMAPI_VIRTUAL void _SetSilhouetteOrigin (DPoint3d silhouetteOrigin) = 0;

   GEOMAPI_VIRTUAL int _GetSilhouetteType () const = 0;
   GEOMAPI_VIRTUAL void _SetSilhouetteType (int silhouetteType) = 0;

   GEOMAPI_VIRTUAL double _GetSilhouetteToleranceDivisor () const = 0;
   GEOMAPI_VIRTUAL void _SetSilhouetteToleranceDivisor (double silhouetteToleranceDivisor) = 0;

   GEOMAPI_VIRTUAL int _GetCurvedSurfaceMaxPerFace () const = 0;
   GEOMAPI_VIRTUAL void _SetCurvedSurfaceMaxPerFace (int curveSurfaceMaxPerFace) = 0;

   GEOMAPI_VIRTUAL bool _GetCombineFacets () const = 0;
   GEOMAPI_VIRTUAL void _SetCombineFacets (bool combineFacets) = 0;

   GEOMAPI_VIRTUAL bool _GetConvexFacetsRequired () const = 0;
   GEOMAPI_VIRTUAL void _SetConvexFacetsRequired (bool convexFacetsRequired) = 0;

   GEOMAPI_VIRTUAL FacetParamMode _GetParamMode () const = 0;
   GEOMAPI_VIRTUAL void _SetParamMode (FacetParamMode paramMode) = 0;

   GEOMAPI_VIRTUAL double _GetParamDistanceScale () const = 0;
   GEOMAPI_VIRTUAL void _SetParamDistanceScale (double paramDistanceScale) = 0;

   GEOMAPI_VIRTUAL double _GetToleranceDistanceScale () const = 0;
   GEOMAPI_VIRTUAL void _SetToleranceDistanceScale (double toleranceDistanceScale) = 0;

   GEOMAPI_VIRTUAL bool _GetEdgeChainsRequired () const = 0;
   GEOMAPI_VIRTUAL void _SetEdgeChainsRequired (bool edgeChainsRequired) = 0;

   GEOMAPI_VIRTUAL bool _GetSmoothTriangleFlowRequired () const = 0;
   GEOMAPI_VIRTUAL void _SetSmoothTriangleFlowRequired (bool smoothTrianglesRequired) = 0;

   GEOMAPI_VIRTUAL bool _GetBSurfSmoothTriangleFlowRequired () const = 0;
   GEOMAPI_VIRTUAL void _SetBSurfSmoothTriangleFlowRequired (bool smoothBsurfTrianglesRequired) = 0;

   GEOMAPI_VIRTUAL bool _GetDoSpatialLaplaceSmoothing () const = 0;
   GEOMAPI_VIRTUAL void _SetDoSpatialLaplaceSmoothing (bool smoothBsurfTrianglesRequired) = 0;

   GEOMAPI_VIRTUAL double _GetCurvatureWeightFactor () const = 0;
   GEOMAPI_VIRTUAL void _SetCurvatureWeightFactor (double fration = 0.5) = 0;

   GEOMAPI_VIRTUAL bool _GetIgnoreFaceMaterialAttachments () const = 0;
   GEOMAPI_VIRTUAL void _SetIgnoreFaceMaterialAttachments (bool ignoreFaceAttachments) = 0;

   GEOMAPI_VIRTUAL bool _GetHideSmoothEdgesWhenGeneratingNormals () const = 0;
   GEOMAPI_VIRTUAL void _SetHideSmoothEdgesWhenGeneratingNormals (bool ignoreFaceAttachments) = 0;

   GEOMAPI_VIRTUAL int _GetBsplineSurfaceEdgeHiding () const = 0;
   GEOMAPI_VIRTUAL void _SetBsplineSurfaceEdgeHiding (int edgeHiding) = 0;

   GEOMAPI_VIRTUAL bool _GetIgnoreHiddenBRepEntities () const = 0;
   GEOMAPI_VIRTUAL void _SetIgnoreHiddenBRepEntities (bool ignoreHiddenBRepEntities) = 0;

   GEOMAPI_VIRTUAL bool _GetOmitBRepEdgeChainIds () const = 0;
   GEOMAPI_VIRTUAL void _SetOmitBRepEdgeChainIds (bool doOmit) = 0;

   GEOMAPI_VIRTUAL bool _GetBRepConcurrentFacetting() const = 0;
   GEOMAPI_VIRTUAL void _SetBRepConcurrentFacetting(bool doConcurrentFacet) = 0;

   GEOMAPI_VIRTUAL IFacetOptionsPtr  _Clone() const = 0;

// Protected signatures of public/protected methods

   GEOMAPI_VIRTUAL void _SetDefaults () = 0;
   GEOMAPI_VIRTUAL void _SetCurveDefaults () = 0;

public:

//! Set the ChordTolerance facet control. 
GEOMDLLIMPEXP void SetChordTolerance (double chordTolerance);
//! Get the ChordTolerance facet control. 
GEOMDLLIMPEXP double GetChordTolerance () const;

//! Set the MaxEdgeLength facet control. 
GEOMDLLIMPEXP void SetMaxEdgeLength (double maxEdgeLength);
//! Get the MaxEdgeLength facet control. 
GEOMDLLIMPEXP double GetMaxEdgeLength () const;

//! Set the AngleTolerance facet control. 
GEOMDLLIMPEXP void SetAngleTolerance (double normalAngleTolerance);
//! Get the AngleTolerance facet control. 
GEOMDLLIMPEXP double GetAngleTolerance () const;

//! Set the MaxPerBezier facet control. 
GEOMDLLIMPEXP void SetMaxPerBezier (int maxPerBezier);
//! Get the MaxPerBezier facet control. 
GEOMDLLIMPEXP int GetMaxPerBezier () const;

//! Set the MinPerBezier facet control. 
GEOMDLLIMPEXP void SetMinPerBezier (int minPerBezier);
//! Get the MinPerBezier facet control. 
GEOMDLLIMPEXP int GetMinPerBezier () const;

//! Set the MaxPerFace facet control. 
GEOMDLLIMPEXP void SetMaxPerFace (int maxPerFace);
//! Get the MaxPerFace facet control. 
GEOMDLLIMPEXP int GetMaxPerFace () const;

//! Set the EdgeHiding facet control. 
GEOMDLLIMPEXP void SetEdgeHiding (bool edgeHiding);
//! Get the EdgeHiding facet control. 
GEOMDLLIMPEXP bool GetEdgeHiding () const;

//! Set edge hiding on bspline surface mesh.
//!<ul>
//!<li>0: Leave original edge hiding from the mesher.  This is probably (but not certainly) all edges visible.
//!<li>1: (default) Exterior edges and edges with crease of 40 degrees or more are visible
//!<li>2: All edges visible
//!</ul>
GEOMDLLIMPEXP void SetBsplineSurfaceEdgeHiding (int edgeHiding);
//! Get the bspline surface edge Hiding control (see SetBsplineSurfaceEdgeHiding)
GEOMDLLIMPEXP int GetBsplineSurfaceEdgeHiding () const;


//! Set the NormalsRequired facet control. 
GEOMDLLIMPEXP void SetNormalsRequired (bool normalsRequired);
//! Get the NormalsRequired facet control. 
GEOMDLLIMPEXP bool GetNormalsRequired () const;

//! Set the ParamsRequired facet control. 
GEOMDLLIMPEXP void SetParamsRequired (bool paramsRequired);
//! Get the ParamsRequired facet control. 
GEOMDLLIMPEXP bool GetParamsRequired () const;

//! Set the EdgeChainsRequired facet control. 
GEOMDLLIMPEXP void SetEdgeChainsRequired (bool edgeChainsRequired);
//! Get the EdgeChainsRequired facet control. 
GEOMDLLIMPEXP bool GetEdgeChainsRequired () const;

//! Set the SmoothTriangleFlow facet control.  (This can apply to both bspline and non-bspline.  See 
GEOMDLLIMPEXP void SetSmoothTriangleFlowRequired (bool value);
//! Get the SmoothTriangleFlow facet control.
GEOMDLLIMPEXP bool GetSmoothTriangleFlowRequired () const;

//! Set the SmoothTriangleFlow facet control -- bspline surfaces only.  A true unqualified SetSmoothTriangleFlowRequired overrides false for the Bsurf setting!!
GEOMDLLIMPEXP void SetBSurfSmoothTriangleFlowRequired (bool value);
//! Get the SmoothTriangleFlow facet control -- bspline surfaces only.
GEOMDLLIMPEXP bool GetBSurfSmoothTriangleFlowRequired () const;

//! Set the SmoothTriangleFlow facet control -- bspline surfaces only.  A true unqualified SetSmoothTriangleFlowRequired overrides false for the Bsurf setting!!
GEOMDLLIMPEXP void SetDoSpatialLaplaceSmoothing (bool value);
//! Get the SmoothTriangleFlow facet control -- bspline surfaces only.
GEOMDLLIMPEXP bool GetDoSpatialLaplaceSmoothing () const;

//! Set the CurvatureWeightFactor facet control.
GEOMDLLIMPEXP void SetCurvatureWeightFactor (double value);
//! Get the CurvatureWeightFactor control.
GEOMDLLIMPEXP double  GetCurvatureWeightFactor () const;

//! Set the CurveParameterMapping facet control. 
GEOMDLLIMPEXP void SetCurveParameterMapping (CurveParameterMapping curveParameterMapping);
//! Get the CurveParameterMapping facet control. 
GEOMDLLIMPEXP CurveParameterMapping GetCurveParameterMapping () const;

//! Set the VertexColorsRequired facet control. 
GEOMDLLIMPEXP void SetVertexColorsRequired (bool vertexColorsRequired);
//! Get the VertexColorsRequired facet control. 
GEOMDLLIMPEXP bool GetVertexColorsRequired () const;

//! Set the SilhouetteDirection facet control. 
GEOMDLLIMPEXP void SetSilhouetteDirection (DVec3d silhouetteDirection);
//! Get the SilhouetteDirection facet control. 
GEOMDLLIMPEXP DVec3d GetSilhouetteDirection () const;

//! Set the SilhouetteOrigin facet control. 
GEOMDLLIMPEXP void SetSilhouetteOrigin (DPoint3d silhouetteOrigin);
//! Get the SilhouetteOrigin facet control. 
GEOMDLLIMPEXP DPoint3d GetSilhouetteOrigin () const;

//! Set the SilhouetteType facet control. 
GEOMDLLIMPEXP void SetSilhouetteType (int silhouetteType);
//! Get the SilhouetteType facet control. 
GEOMDLLIMPEXP int GetSilhouetteType () const;

//! Set the SilhouetteToleranceDivisor facet control. 
GEOMDLLIMPEXP void SetSilhouetteToleranceDivisor (double silhouetteToleranceDivisor);
//! Get the SilhouetteToleranceDivisor facet control. 
GEOMDLLIMPEXP double GetSilhouetteToleranceDivisor () const;

//! Set the CurvedSurfaceMaxPerFace facet control. 
GEOMDLLIMPEXP void SetCurvedSurfaceMaxPerFace (int curveSurfaceMaxPerFace);
//! Get the CurvedSurfaceMaxPerFace facet control. 
GEOMDLLIMPEXP int GetCurvedSurfaceMaxPerFace () const;

//! Set the CombineFacets facet control. 
GEOMDLLIMPEXP void SetCombineFacets (bool combineFacets);
//! Get the CombineFacets facet control. 
GEOMDLLIMPEXP bool GetCombineFacets () const;

//! Set the ConvexFacetsRequired facet control. 
GEOMDLLIMPEXP void SetConvexFacetsRequired (bool convexFacetsRequired);
//! Get the ConvexFacetsRequired facet control. 
GEOMDLLIMPEXP bool GetConvexFacetsRequired () const;

//! Set the ParamMode facet control. 
GEOMDLLIMPEXP void SetParamMode (FacetParamMode paramMode);
//! Get the ParamMode facet control. 
GEOMDLLIMPEXP FacetParamMode GetParamMode () const;

//! Set the ParamDistanceScale facet control. 
GEOMDLLIMPEXP void SetParamDistanceScale (double paramDistanceScale);
//! Get the ParamDistanceScale facet control. 
GEOMDLLIMPEXP double GetParamDistanceScale () const;

//! Set the ToleranceDistanceScale facet control. 
GEOMDLLIMPEXP void SetToleranceDistanceScale (double toleranceDistanceScale);
//! Get the ToleranceDistanceScale facet control. 
GEOMDLLIMPEXP double GetToleranceDistanceScale () const;

//! Set whether facets returned for BReps are separated by color/material when there is per-face symbology attachments.
GEOMDLLIMPEXP void SetIgnoreFaceMaterialAttachments (bool ignoreFaceAttachments);
//! Get whether facets returned for BReps are separated by color/material when there is per-face symbology attachments.
GEOMDLLIMPEXP bool GetIgnoreFaceMaterialAttachments () const;


//! Set whether smooth edges between facets are marked as hidden when normals are genereted for a polyface without normals.
GEOMDLLIMPEXP void SetHideSmoothEdgesWhenGeneratingNormals (bool hideSmoothEdgesWhenGeneratingNormals);
//! Get whether mooth edges between facets are marked as hidden when normals are genereted for a polyface without normals.
GEOMDLLIMPEXP bool GetHideSmoothEdgesWhenGeneratingNormals () const;

//! Set whether to ignore hidden entities (edges and faces from unification) in BRep entities (optimization for tile generation).
GEOMDLLIMPEXP bool GetIgnoreHiddenBRepEntities () const;
//! Get whether to ignore hidden entities (edges and faces from unification) in BRep entities (optimization for tile generation).k
GEOMDLLIMPEXP void SetIgnoreHiddenBRepEntities (bool ignoreHiddenBRepEntities);

//! Get whether to omit edges chain ids. ((optimization for tile generation).
GEOMDLLIMPEXP bool GetOmitBRepEdgeChainIds () const;
//! Set whether to omit edges chain ids. ((optimization for tile generation).
GEOMDLLIMPEXP void SetOmitBRepEdgeChainIds (bool doOmit);

//! Get whether PK_TOPOL_facet_2 runs in concurrent or exclusive mode.
GEOMDLLIMPEXP bool GetBRepConcurrentFacetting() const;
//! Set whether PK_TOPOL_facet_2 runs in concurrent or exclusive mode.
GEOMDLLIMPEXP void SetBRepConcurrentFacetting(bool doConcurrentFacet);

//! Set all parameters to default values
GEOMDLLIMPEXP void SetDefaults ();

//! Set all parameters to default values for curves.
//! (This will have finer angle tolerance than surfaces)
GEOMDLLIMPEXP void SetCurveDefaults ();

//! Create a copy.
GEOMDLLIMPEXP IFacetOptionsPtr  Clone() const;

//! Return a (smart pointer) implementation of the interface.  Depreated -- use Create() or CreateForCurves ()
static GEOMDLLIMPEXP IFacetOptionsPtr New ();

//! Return a (smart pointer) implementation of the interface, with tolerances set for typical surfaces.
static GEOMDLLIMPEXP IFacetOptionsPtr Create ();

//! Return a (smart pointer) implementation of the interface
static GEOMDLLIMPEXP IFacetOptionsPtr CreateForSurfaces
(
double chordTol = 0.0,        //!< [in] target deviation between facet and true surface.
double angleRadians = msGeomConst_piOver12,    //!< [in] target angle between adjacent facets
double maxEdgeLength = 0.0,    //!< [in] target max length of any facet edge.
bool triangulate = false,       //!< [in] true to triangulate
bool normals = false,           //!< [in] true to construct facet normals
bool params = false             //!< [in] true to construct facet texture parameters
);


//! Return a (smart pointer) implementation of the interface, with tolerances set for typical curves.
static GEOMDLLIMPEXP IFacetOptionsPtr CreateForCurves ();

//! Compute the number of strokes needed for a (weighted) bezier.
GEOMDLLIMPEXP bool BezierStrokeCount (DPoint4dCP poles, size_t order, size_t &count) const;
//! Compute the number of strokes needed for a (nonweighted) bezier.
GEOMDLLIMPEXP bool BezierStrokeCount (bvector<DPoint4d> const &poles, size_t index0, int order, size_t &count) const;
//! Compute the number of strokes needed for a line segment.
GEOMDLLIMPEXP size_t SegmentStrokeCount (DSegment3dCR segment) const;
//! Compute the number of strokes needed for a distance.
GEOMDLLIMPEXP size_t DistanceStrokeCount (double distance) const;

//! Compute the number of strokes needed for distance with turn, and optional max curvature.
GEOMDLLIMPEXP size_t DistanceAndTurnStrokeCount (double distance, double turnRadians, double maxCurvature = 0.0) const;

//! Compute the number of strokes needed for distance with turn and maxCurvature.
GEOMDLLIMPEXP static size_t DistanceAndTurnStrokeCount
(
double distance,
double turnRadians,
double maxCurvature,    // curve property
double chordTol,
double angleTol,
double maxEdgeLength
);

//! Compute the number of strokes needed for a complete ellipse.
GEOMDLLIMPEXP size_t FullEllipseStrokeCount (DEllipse3dCR ellipse) const;
//! Compute the number of strokes needed for a partial ellipse.
GEOMDLLIMPEXP size_t EllipseStrokeCount (DEllipse3dCR ellipse) const;
//! Comptue the number of strokes needed for a bspline curve.
GEOMDLLIMPEXP size_t BsplineCurveStrokeCount (MSBsplineCurveCR curve) const;
//! Comptue the number of strokes needed for a linestring.  This is the sum of counts on individual segments.
GEOMDLLIMPEXP size_t LineStringStrokeCount (bvector<DPoint3d> const &points) const;

}; // IFacetOptions




struct IPolyfaceConstruction;
typedef RefCountedPtr<IPolyfaceConstruction>  IPolyfaceConstructionPtr;

typedef struct IPolyfaceConstruction *IPolyfaceConstructionP;
typedef struct IPolyfaceConstruction &IPolyfaceConstructionR;


//! Helper object for constructing polyface mesh. 
//! The construction object carries (a) facet options (b) the growing mesh (c) coordinate maps so duplicate points can be assigned a shared index.
struct  IPolyfaceConstruction : public RefCountedBase
{

protected:



private:
struct ConstructionState
    {
    bool      m_reverseNewFacetIndexOrder;
    bool      m_reverseNewNormals;

    Transform m_worldToLocal;
    Transform m_localToWorld;
    // These are derived from the transforms .... must be updated when the transforms change ...
    bool      m_isTransformed;
    double    m_worldToLocalScale;
    double    m_localToWorldScale;

    // This maps normals from local to world.
    //  This is the INVERSE TRANPSOSE of the local to world matrix part --
    //   this makes normals transform properly even if there is skew in the local to world
    // This does NOT preserve length -- normalize if you need it.
    RotMatrix m_localToWorldNormals;



    ConstructionState ();
    };

bvector<ConstructionState> m_stateStack;
ConstructionState m_state;

bool m_needNormals;
bool m_needParams;
bool m_needEdgeChains;



// Make sure the options, polyface, and instance vars are consistent.
void SynchOptions ();

protected:
//! Constructor -- must initialize transform state.
IPolyfaceConstruction ();

GEOMAPI_VIRTUAL void _Clear ( ) = 0;
GEOMAPI_VIRTUAL size_t _FindOrAddPoint (DPoint3dCR point) = 0;
GEOMAPI_VIRTUAL size_t _FindOrAddNormal (DVec3dCR normal) = 0;
GEOMAPI_VIRTUAL size_t _FindOrAddParam (DPoint2dCR param) = 0;
GEOMAPI_VIRTUAL size_t _FindOrAddIntColor (uint32_t color) = 0;
GEOMAPI_VIRTUAL size_t _AddPointIndex (size_t zeroBasedIndex, bool visible) = 0;
GEOMAPI_VIRTUAL size_t _AddNormalIndex (size_t zeroBasedIndex) = 0;
GEOMAPI_VIRTUAL size_t _AddParamIndex (size_t zeroBasedIndex) = 0;
GEOMAPI_VIRTUAL size_t _AddColorIndex (size_t zeroBasedIndex) = 0;
GEOMAPI_VIRTUAL size_t _AddFaceIndex (size_t zeroBasedIndex) = 0;

GEOMAPI_VIRTUAL size_t _AddSignedOneBasedPointIndex (int zeroBasedIndex) = 0;
GEOMAPI_VIRTUAL size_t _AddSignedOneBasedNormalIndex (int zeroBasedIndex) = 0;
GEOMAPI_VIRTUAL size_t _AddSignedOneBasedParamIndex (int zeroBasedIndex) = 0;
GEOMAPI_VIRTUAL size_t _AddSignedOneBasedColorIndex (int zeroBasedIndex) = 0;

GEOMAPI_VIRTUAL size_t _AddPointIndexTerminator ( ) = 0;
GEOMAPI_VIRTUAL size_t _AddNormalIndexTerminator ( ) = 0;
GEOMAPI_VIRTUAL size_t _AddParamIndexTerminator ( ) = 0;
GEOMAPI_VIRTUAL size_t _AddColorIndexTerminator ( ) = 0;
GEOMAPI_VIRTUAL PolyfaceHeaderR _GetClientMeshR ( ) = 0;
GEOMAPI_VIRTUAL PolyfaceHeaderPtr _GetClientMeshPtr ( ) = 0;
GEOMAPI_VIRTUAL IFacetOptionsR _GetFacetOptionsR ( ) = 0;

GEOMAPI_VIRTUAL size_t _IncrementFaceIndex ( ) = 0;
GEOMAPI_VIRTUAL void  _SetFaceIndex (size_t index) = 0;
GEOMAPI_VIRTUAL void  _SetFaceData (FacetFaceDataCR data) = 0;
GEOMAPI_VIRTUAL FacetFaceData _GetFaceData () const = 0;
GEOMAPI_VIRTUAL size_t _GetFaceIndex () const = 0;
GEOMAPI_VIRTUAL void _CollectCurrentFaceRanges () = 0;
GEOMAPI_VIRTUAL void _EndFace () = 0;

GEOMAPI_VIRTUAL size_t _AddPointIndexTriangle (size_t index0, bool visible0, size_t index1, bool visible1, size_t index2, bool visible2) = 0;
GEOMAPI_VIRTUAL size_t _AddNormalIndexTriangle (size_t index0, size_t index1, size_t index2) = 0;
GEOMAPI_VIRTUAL size_t _AddParamIndexTriangle (size_t index0, size_t index1, size_t index2) = 0;
GEOMAPI_VIRTUAL size_t _AddColorIndexTriangle (size_t index0, size_t index1, size_t index2) = 0;
GEOMAPI_VIRTUAL size_t _AddPointIndexQuad (size_t index0, bool visible0, size_t index1, bool visible1, size_t index2, bool visible2, size_t index3, bool visible3) = 0;
GEOMAPI_VIRTUAL size_t _AddNormalIndexQuad (size_t index0, size_t index1, size_t index2, size_t index3) = 0;
GEOMAPI_VIRTUAL size_t _AddParamIndexQuad (size_t index0, size_t index1, size_t index2, size_t index3) = 0;
GEOMAPI_VIRTUAL size_t _AddColorIndexQuad (size_t index0, size_t index1, size_t index2, size_t index3) = 0;
GEOMAPI_VIRTUAL bool   _AddPolyface (PolyfaceQueryCR polyface, size_t drawMethodIndex) = 0;
public:
// To be called (instead of EndFace) by BSI internal callers until confirmed that they do parameter scaling options correctly.  Then call EndFace.
void EndFace_internal ();

public:

//! Set the (modal) face index. (And clear the modal parameter range)
GEOMDLLIMPEXP void SetFaceIndex (size_t index);
//! Get the (modal) face index.
GEOMDLLIMPEXP size_t GetFaceIndex () const;
//! Increment the (modal) face index. (And clear the modal parameter range)
//! return the (incremented) index.
GEOMDLLIMPEXP size_t IncrementFaceIndex ();
//! Return the current face data.
GEOMDLLIMPEXP FacetFaceData GetFaceData () const;
//! Set the current face data.
GEOMDLLIMPEXP void SetFaceData (FacetFaceDataCR data);
//! Set (only) the paramDistance range part of the current FacetFacetData.
GEOMDLLIMPEXP void SetCurrentFaceParamDistanceRange (DRange2dCR range);

//! Collect point, param, and normal range data in the current face.
GEOMDLLIMPEXP void CollectCurrentFaceRanges ();

//! Finalize data for the current face.
GEOMDLLIMPEXP void EndFace ();

//! Clear client mesh and all construction support
GEOMDLLIMPEXP void Clear ( );

//! Find or add a point.  Return the (0-based) index.
GEOMDLLIMPEXP size_t FindOrAddPoint (DPoint3dCR point);

//! Find or add a normal.  Return the (0-based) index.
GEOMDLLIMPEXP size_t FindOrAddNormal (DVec3dCR normal);

//! Find or add a param.  Return the (0-based) index.
GEOMDLLIMPEXP size_t FindOrAddParam (DPoint2dCR param);

//! Find or add a color.  Return the (0-based) index.
GEOMDLLIMPEXP size_t FindOrAddIntColor (uint32_t color);

//! Add a point index, adjusted to 1-based indexing with visibility in sign.  Return the (0-based) position in the PointIndex array.
GEOMDLLIMPEXP size_t AddPointIndex (size_t zeroBasedIndex, bool visible);

//! Add a normal index, adjusted to 1-based indexing.  Return the (0-based) position in the NormalIndex array.
GEOMDLLIMPEXP size_t AddNormalIndex (size_t zeroBasedIndex);

//! Add a param index, adjusted to 1-based indexing  Return the (0-based) position in the ParamIndex array.
GEOMDLLIMPEXP size_t AddParamIndex (size_t zeroBasedIndex);

//! Add a color index, adjusted to 1-based indexing  Return the (0-based) position in the ParamIndex array.
GEOMDLLIMPEXP size_t AddColorIndex (size_t zeroBasedIndex);

//! Add a point index directly, i.e. caller is responsible for providing a one based index with optional negation for hidden edges.
GEOMDLLIMPEXP size_t AddSignedOneBasedPointIndex (int zeroBasedIndex);

//! Add a normal index, directly, i.e. caller is responsible for providing a one based index
GEOMDLLIMPEXP size_t AddSignedOneBasedNormalIndex (int zeroBasedIndex);

//! Add a param index, directly, i.e. caller is responsible for providing a one based index
GEOMDLLIMPEXP size_t AddSignedOneBasedParamIndex (int zeroBasedIndex);

//! Add a color index, directly, i.e. caller is responsible for providing a one based index
GEOMDLLIMPEXP size_t AddSignedOneBasedColorIndex (int zeroBasedIndex);


//! Add a face index, adjusted to 1-based indexing  Return the (0-based) position in the FaceIndex array.
GEOMDLLIMPEXP size_t AddFaceIndex (size_t zeroBasedIndex);

//! Add a terminator to the point index table.  Return the (0-based) position in the PointIndex array.
GEOMDLLIMPEXP size_t AddPointIndexTerminator ( );

//! Add a terminator to the normal index table.  Return the (0-based) position in the NormalIndex array.
GEOMDLLIMPEXP size_t AddNormalIndexTerminator ( );

//! Add a terminator to the param index table.  Return the (0-based) position in the ParamIndex array.
GEOMDLLIMPEXP size_t AddParamIndexTerminator ( );

//! Add a terminator to the color index table.  Return the (0-based) position in the ColorIndex array.
GEOMDLLIMPEXP size_t AddColorIndexTerminator ( );

//! Return the client polyface mesh.  DEPRECATED -- use GetClientMeshPtr to get the smart pointer.
GEOMDLLIMPEXP PolyfaceHeaderR GetClientMeshR ( );

//! Return the client polyface mesh.
GEOMDLLIMPEXP PolyfaceHeaderPtr GetClientMeshPtr ();

//! Return the facet controls
GEOMDLLIMPEXP IFacetOptionsR GetFacetOptionsR ( );

//! Add a triangle to the point index table.  Return the (0-based) position in the PointIndex array.
GEOMDLLIMPEXP size_t AddPointIndexTriangle (size_t index0, bool visible0, size_t index1, bool visible1, size_t index2, bool visible2);

//! Add a triangle to the normal index table.  Return the (0-based) position in the NormalIndex array.
GEOMDLLIMPEXP size_t AddNormalIndexTriangle (size_t index0, size_t index1, size_t index2);

//! Add a triangle to the param index table.  Return the (0-based) position in the ParamIndex array.
GEOMDLLIMPEXP size_t AddParamIndexTriangle (size_t index0, size_t index1, size_t index2);

//! Add a triangle to the color index table.  Return the (0-based) position in the ColorIndex array.
GEOMDLLIMPEXP size_t AddColorIndexTriangle (size_t index0, size_t index1, size_t index2);

//! Add a quad to the point index table.  Return the (0-based) position in the PointIndex array.
GEOMDLLIMPEXP size_t AddPointIndexQuad (size_t index0, bool visible0, size_t index1, bool visible1, size_t index2, bool visible2, size_t index3, bool visible3);

//! Add a quad to the normal index table.  Return the (0-based) position in the NormalIndex array.
GEOMDLLIMPEXP size_t AddNormalIndexQuad (size_t index0, size_t index1, size_t index2, size_t index3);

//! Add a quad to the param index table.  Return the (0-based) position in the ParamIndex array.
GEOMDLLIMPEXP size_t AddParamIndexQuad (size_t index0, size_t index1, size_t index2, size_t index3);

//! Add a quad to the color index table.  Return the (0-based) position in the ColorIndex array.
GEOMDLLIMPEXP size_t AddColorIndexQuad (size_t index0, size_t index1, size_t index2, size_t index3);

//! Add polyface mesh.
GEOMDLLIMPEXP bool AddPolyface (PolyfaceQueryCR polyface, size_t drawMethodIndex=0);


//! Ask if normals are needed. 
GEOMDLLIMPEXP bool NeedNormals ();
//! Ask if params are needed.
GEOMDLLIMPEXP bool NeedParams ();



//! Clear the construction state stack and set current state.
GEOMDLLIMPEXP void InitializeConstructionStateAndStack ();
//! Clear the current construction state, but leave the stack unchanged.
GEOMDLLIMPEXP void InitializeCurrentConstructionState ();


//! Ask if facets are to be reversed as received.
//! @remarks This is affected by PushState/PopState operations.
GEOMDLLIMPEXP bool GetReverseNewFacetIndexOrder () const;
//! Get the current normal vector reversal state.
GEOMDLLIMPEXP bool GetReverseNewNormals () const;


//! Get the world to local placement transform.
//! @return false if the transform is an identity.
//! @remarks This is affected by PushState/PopState operations.
GEOMDLLIMPEXP bool GetWorldToLocal (TransformR transform) const;
//! Ask if the local to world transform is nontrivial
//! @remarks This is affected by PushState/PopState operations.
GEOMDLLIMPEXP bool IsTransformed () const;
//! Get the local to world placement transform.
//! @return false if the transform is an identity.
//! @remarks This is affected by PushState/PopState operations.
GEOMDLLIMPEXP bool GetLocalToWorld (TransformR transform) const;
//! Set the local to world transform.
//! @return false if the given transform is not invertible.
//! @remarks This is affected by PushState/PopState operations.
GEOMDLLIMPEXP bool SetLocalToWorld (TransformCR transform);
//! Apply (right multiply) the local to world transform.
//! @return false if the given transform is not invertible.
GEOMDLLIMPEXP bool ApplyLocalToWorld (TransformCR relativeTransform);

//! Multiply the local to world transform times the input point.
//! @return transformed point.
GEOMDLLIMPEXP DPoint3d MultiplyByLocalToWorld (DPoint3dCR localPoint) const;

//! Multiply a surface normal by the local to world effects and renormalize.
//!  This also applies the normal reversal flag.
//! @return transformed ponit.
GEOMDLLIMPEXP DVec3d MultiplyNormalByLocalToWorld (DVec3dCR localNormal) const;

//! Push the current transform and revesal state.
//! @param [in] initializeCurrentState controls whether current state is 
//!     reinitialized (true) or left unchanged (false)
GEOMDLLIMPEXP void PushState (bool initializeCurrentState = false);
//! Pop the current transform and revesal state.
//! @return true if the stack had a state to pop.
GEOMDLLIMPEXP bool PopState ();

//! Set the current facet index reversal state.
GEOMDLLIMPEXP void SetReverseNewFacetIndexOrder (bool reverse);
//! Set the current normal vector reversal state.
GEOMDLLIMPEXP void SetReverseNewNormals (bool reverse);

//! Get the local to world matrix for surface normals.
//! @param [out] matrix returned matrix 
//! @return false if no local to world transform is in effect.
GEOMDLLIMPEXP bool GetLocalToWorldNormals (RotMatrixR matrix) const;


//! Toggle both index order and normal
GEOMDLLIMPEXP void ToggleIndexOrderAndNormalReversal ();

//! Get the (average) scale factor of the local to world transform.
//! @remarks This is affected by PushState/PopState operations.
GEOMDLLIMPEXP double GetLocalToWorldScale () const;
//! Get the (average) scale factor of the world to local transform.
//! @remarks This is affected by PushState/PopState operations.
GEOMDLLIMPEXP double GetWorldToLocalScale () const;

//! Add ruled facets between ellipses.
GEOMDLLIMPEXP void AddRuled (DEllipse3dR ellipse0, DEllipse3dR ellipse1, bool cap);
//! Add bspline surface mesh
GEOMDLLIMPEXP void Add (MSBsplineSurfaceCR surface);

//! Add bspline surface mesh, with smoothing effects.
GEOMDLLIMPEXP void AddSmoothed (MSBsplineSurfaceCR surface);


//! Add square grid with normal, param at each point.
GEOMDLLIMPEXP void AddRowMajorQuadGrid (DPoint3dCP points, DVec3dCP normals, DPoint2dCP params, size_t numPerRow, size_t numRow, bool forceTriangles = false);
//! Add square grid with normal, param at each point.
//! @param [in] points coordinates alternating between bottom and top of strip.
//! @param [in] params corresponding parameters
//! @param [in] normals corresponding normals
//! @param [in] numPoint number of points (2 more than number of triangles)
//! @param [in] firstTriangle012 true if 012 is leading triangle order, false if 021 is leading triangle order.
GEOMDLLIMPEXP void AddTriStrip(DPoint3dCP points, DVec3dCP normals, DPoint2dCP params, size_t numPoint, bool firstTriangle012);

//! Add facets to a mesh.
//! Facets approximate a tube around a centerline.
//! The centerline curve (bspline) should be planar or nearly so.
//!  (If it is not, the successive circular sections may pinch in strange ways)
//! @param [in] centerlineCurve tube centerline
//! @param [in] radius tube radius
//! @param [in] numEdgePerSection number of edges around each section circle.
//!             If zero, determined from builder's facet options.
//! @param [in] numSectionEdge number of edges along curve.
//!             If zero, determined from builder's facet options.
GEOMDLLIMPEXP void AddTubeMesh
(
MSBsplineCurveCR centerlineCurve,
double radius,
int numEdgePerSection,
int numSectionEdge
);

/*__PUBLISH_SECTION_END__*/


/*__PUBLISH_SECTION_START__*/

//! Make a linear sweep from base points.
//! To indicate a sharp corner, duplicate the point, using incoming tangent on the first, outgoing on the second.
//! Any zero-length edge will be skipped, and the sweep edge will be marked visible.
GEOMDLLIMPEXP void AddLinearSweep (bvector<DPoint3d> &pointA, bvector<DVec3d> &tangentA, DVec3dCR step);
//! Make a linear sweep with optional cap from base points.
//! To indicate a sharp corner, duplicate the point, using incoming tangent on the first, outgoing on the second.
//! Any zero-length edge will be skipped, and the sweep edge will be marked visible.
GEOMDLLIMPEXP void AddLinearSweep (bvector<DPoint3d> &pointA, bvector<DVec3d> *tangentA, DVec3dCR step, bool capped);

//! Add ruled mesh between each pair of successive contours.
GEOMDLLIMPEXP bool AddRuledBetweenCorrespondingCurves (bvector<CurveVectorPtr> const &contours, bool capped);
//! Add rotational sweep from curves
GEOMDLLIMPEXP void AddRotationalSweep (CurveVectorPtr, DPoint3dCR center, DVec3dCR axis, double totalSweep, bool capped);

//! Add (triangulation of) the region bounded by a curve vector.
GEOMDLLIMPEXP void AddRegion (CurveVectorCR region);

//! Stroke with facet options from the PolyfaceConstruction.
//! Return false if not a simple loop.
//! @remark points are doubled at hard corners (so the incoming and outgoing tangents can be distinguished)
GEOMDLLIMPEXP bool StrokeWithDoubledPointsAtCorners(CurveVectorCR curves, bvector<DPoint3d> &points, bvector<DVec3d>&tangents, size_t &numLoop);


//! Stroke with facet options from the PolyfaceConstruction.
//! Return false if non-loop structure found
//! @remark points are doubled at hard corners (so the incoming and outgoing tangents can be distinguished)
GEOMDLLIMPEXP bool StrokeWithDoubledPointsAtCorners(
CurveVectorCR curves,
bvector<bvector<DPoint3d> >&points,
bvector<bvector<DVec3d> >&tangents,
bvector<double> &curveLengths
);

//! Stroke with facet options from the PolyfaceConstruction.
//! Return false if not a simple loop.
//! @remark points are doubled at hard corners (so the incoming and outgoing tangents can be distinguished)
GEOMDLLIMPEXP bool StrokeWithDoubledPointsAtCorners
(
CurveVectorCR curves,                                     //!< [in] path(s) to stroke
bvector<DPoint3dDoubleUVCurveArrays> &pointTangentCurve,  //!< out] array of loops with point, tangent, and curve.
bvector<double> &curveLength                              //!< [out] array with total of real curve lengths in each entry of the pointTangentCurve array.
);


//! Stroke with facet options from the PolyfaceConstruction.
//! Return false if not a simple loop.
//! @remark points are doubled at hard corners (so the incoming and outgoing tangents can be distinguished)
GEOMDLLIMPEXP bool Stroke (CurveVectorCR curves, bvector<DPoint3d> &points, size_t &numLoop);
 
//! Triangulate a space polygon and add to mesh. Disconnect points separate multiple loops.
GEOMDLLIMPEXP bool AddTriangulation (bvector<DPoint3d> const &points);

//! Add all triangles to mesh.   Optionally reverse orientations.
GEOMDLLIMPEXP void AddTriangles (
bvector<DTriangle3d> const &triangles,                  //!< [in] xyz coordinates of triangles.
bool reverse = false,                                   //!< [in] true to reveres orientation of all triangles (and params)
bvector<DTriangle3d> const *paramTriangles = nullptr    //!< [in] optional vertex params.
);

//! AddTriangulation on 2 sets of points, optionally reversing each.
//! edgeChains are created only if enabled by both edgeChainsPermitted and GetEdgeChainsRequired ().
GEOMDLLIMPEXP void AddTriangulationPair
(
bvector<DPoint3d> &pointA,      //!< [in] points on first cap
bool reverseA,                  //!< [in] true to reverse pointA
bvector<DPoint3d> &pointB,      //!< [in] points on second cap.
bool reverseB,                  //!< [in] true to reverse pointA
bool enableTriangulation = true,//!< [in] true to enable triangulation step
bool edgeChainsPermitted = false,//!< [in] true to enable chain step
CurveTopologyId::Type chainType = CurveTopologyId::Type::Unknown //!< [in] chainType
);
//! add edge chain(s) for points. (multiple chains if there are disconnects)
GEOMDLLIMPEXP void AddEdgeChains (CurveTopologyId::Type type, uint32_t chainIndex, bvector <DPoint3d> const &points, bool addClosure = false);

//! add a single edge chain with 0-based indices
GEOMDLLIMPEXP void AddEdgeChainZeroBased (CurveTopologyId::Type type, uint32_t chainIndex, bvector <size_t> const &pointIndices);

//! Add a polygon
GEOMDLLIMPEXP bool AddPolygon(bvector<DPoint3d> &points);

//! Make a Rotational sweep from base points.
//! To indicate a sharp corner, duplicate the point, using incoming tangent on the first, outgoing on the second.
//! The zero-length edge will be skipped, and the sweep edge will be marked visible.
//! @param [in] pointA base curve points
//! @param [in] tangentA base curve tangents
//! @param [in] center center of rotation
//! @param [in] rotationAxis rotation axis
//! @param [in] totalSweepRadians sweep angle
//! @param [in] reverse true to reverse facet orientations.
//! @param [in] nominalBaseCurveLength if nonzero, parameter distances
//!     along the base curve are scaled to this length
//! @param [in] startCapPointAccumulator optional array to receive fully transformed endcap points.  This array is NOT cleared (so caller can combine over multiple calls.)
//! @param [in] endCapPointAccumulator optional array to receive fully transformed endcap points.  This array is NOT cleared (so caller can combine over multiple calls.)
//! @param [in] curve optional array of parent curves for use in edge visibility tests
GEOMDLLIMPEXP void AddRotationalSweepLoop
(
bvector<DPoint3d> &pointA,
bvector<DVec3d> &tangentA,
DPoint3dCR center,
DVec3dCR rotationAxis,
double   totalSweepRadians,
bool     reverse,
double   nominalBaseCurveLength,
bvector<DPoint3d> *startCapPointAccumulator,
bvector<DPoint3d> *endCapPointAccumulator,
bvector<ICurvePrimitiveP> *curve = nullptr
);


//! Add complete disk.
//! When options specify maxEdgeLength, radial lines go full distance from center to edge. (See AddFulLDiskTriangles)
//! @param [in] ellipse arc to stroke
//! @param [in] numPerQuadrant overrides all options controls.
GEOMDLLIMPEXP void AddFullDisk (DEllipse3dCR ellipse, size_t numPerQuadrant = 0);

//! Add complete disk.
//! When options specify maxEdgeLength, triangles are created without imposed radial edges.
//! @param [in] ellipse arc to stroke
//! @param [in] numPerQuadrant overrides all options controls.
GEOMDLLIMPEXP void AddFullDiskTriangles (DEllipse3dCR ellipse, size_t numPerQuadrant = 0);


//! Add complete sphere.
//! @param [in] center ellipse center
//! @param [in] radius ellipse radius
//! @param [in] numPerQuadrantNS if nonzero overrides all option controls.
//! @param [in] numPerQuadrantEW if nonzero overrides all option controls.
GEOMDLLIMPEXP void AddFullSphere (DPoint3dCR center, double radius, size_t numPerQuadrantEW = 0, size_t numPerQuadrantNS = 0);

//! Add sphere patch with latitude, longitude sweeps.
//! @param [in] center sphere center
//! @param [in] radiusX equator radius at 0 degrees longitude
//! @param [in] radiusY equator radius at 90 degrees longitude
//! @param [in] radiusPole radius at pole.
//! @param [in] numEastWestEdge overrides all option controls. 
//! @param [in] numNorthSouthEdge overrides all option controls.
//! @param [in] longitudeStart in radians
//! @param [in] longitudeSweep in radinas
//! @param [in] latitudeStart in radians
//! @param [in] latitudeSweep in radians
//! @param [in] capped true to include planar caps if latitudes are other than at poles.  This is ignored if longitude sweep is incomplete.
//! @param [in] orientationSelect 0==> ("Meractor map" parameter space bottom U edge is south pole.)   (1==> parameter space U edge is date line)
GEOMDLLIMPEXP void AddEllipsoidPatch (DPoint3dCR center,
            double radiusX,
            double radiusY,
            double radiusPole,
            size_t numEastWestEdge = 0,
            size_t numNorthSouthEdge = 0,
            double longitudeStart = 0.0,
            double longitudeSweep = msGeomConst_2pi,
            double latitudeStart = -msGeomConst_piOver2,
            double latitudeSweep = msGeomConst_pi,
            bool capped = false,
            int orientationSelect = 1
            );

//! Sweep a regular polygon (parallel to xy plane) from z0 to z1.
//! @param [in] n number of edges.  Must be 3 or greater.
//! @param [in] rOuter outer radius
//! @param [in] z0 start z
//! @param [in] z1 end z
//! @param [in] bottomCap true to include bottom cap
//! @param [in] topCap true to include top cap.
GEOMDLLIMPEXP bool AddSweptNGon (size_t n, double rOuter, double z0, double z1,
    bool bottomCap,
    bool topCap
    );

//! Find or add n indices.  Return the n indices plus numWrap additional wraparounds.
GEOMDLLIMPEXP void FindOrAddPoints (bvector<DPoint3d> &point, size_t n, size_t numWrap, bvector<size_t> &index);
//! Find or add n indices.  Return the n indices plus numWrap additional wraparounds.
GEOMDLLIMPEXP void FindOrAddNormals (bvector<DVec3d> &normal, size_t n, size_t numWrap, bvector<size_t> &index);
//! Find or add n indices.  Return the n indices plus numWrap additional wraparounds.
GEOMDLLIMPEXP void FindOrAddParams (bvector<DPoint2d> &param, size_t n, size_t numWrap, bvector<size_t> &index);
//! Find or add n indices.  Return all the indices.
GEOMDLLIMPEXP void FindOrAddPoints (DPoint3dCP points,  size_t n, bvector<size_t> &index);
//! Find or add n indices.  Return all the indices.
GEOMDLLIMPEXP void FindOrAddNormals (DVec3dCP  normals, size_t n, bvector<size_t> &index);
//! Find or add n indices.  Return all the indices.
GEOMDLLIMPEXP void FindOrAddParams (DPoint2dCP params,  size_t n, bvector<size_t> &index);
//! Add point triangles from a center index to edges of a polyline.
GEOMDLLIMPEXP void AddPointIndexFan         (size_t centerIndex, bvector<size_t> &index, size_t numChord, bool reverse);
//! Add point quads between EQUAL LENGTH index vectors.
GEOMDLLIMPEXP void AddPointIndexStrip
(
bool visibleLeft,
bvector<size_t> &indexA,
bool visibleA,
bvector<size_t> &indexB,
bool visibleB,
bool visibleRight,
size_t numQuad,
bool reverse);

//! Add normal triangles from a center index to multiple edges.
GEOMDLLIMPEXP void AddNormalIndexFan        (size_t centerIndex, bvector<size_t> &index, size_t numChord, bool reverse);
//! Add triangles that share index to the a new normal whose coordinates are the cross product of given vectors.
GEOMDLLIMPEXP void AddNormalIndexPlanarFan  (DVec3dCR vectorA, DVec3dCR vectorB, bool reverse, size_t numChord);
//! Add triangles that share index to the a single normal.
GEOMDLLIMPEXP void AddNormalIndexPlanarFan  (size_t normalIndex, size_t numChord);
//! Add quads that share index to the a single normal.
GEOMDLLIMPEXP void AddNormalIndexPlanarStrip (size_t index, size_t numQuad);


//! Add parameter triangles from a center index to edges of a polyline.
GEOMDLLIMPEXP void AddParamIndexFan         (size_t centerIndex, bvector<size_t> &index, size_t numChord, bool reverse);
//! Add parameter quads between same-size indices.
GEOMDLLIMPEXP void AddParamIndexStrip         (bvector<size_t> &indexA, bvector<size_t> &indexB, size_t numQuad, bool reverse);

//! Add facets for all faces of a solid primitive.
GEOMDLLIMPEXP bool AddSolidPrimitive (ISolidPrimitiveCR primitive);
//! Add facets for all faces of a cone
GEOMDLLIMPEXP bool Add (DgnConeDetailCR cone);
//! Add facets for all faces of a sphere
GEOMDLLIMPEXP bool Add (DgnSphereDetailCR sphere);
//! Add facets for all faces of a box
GEOMDLLIMPEXP bool Add (DgnBoxDetailCR box);
//! Add facets for all faces of a TorusPipe
GEOMDLLIMPEXP bool Add (DgnTorusPipeDetailCR torus);
//! Add facets for all faces of a extrusion.
GEOMDLLIMPEXP bool Add (DgnExtrusionDetailCR extrusion);
//! Add facets for all faces of a rotational sweep.
GEOMDLLIMPEXP bool Add (DgnRotationalSweepDetailCR sweep);
//! Add facets for all faces of a ruled sweep.
GEOMDLLIMPEXP bool Add (DgnRuledSweepDetailCR sweep);

//! Apply the FacetParamMode to an array of parameters.
//! On input, params are in coordinates that can be scaled independently in x and y to obtain distances.
//! On ouptut, params are in PARAM_MODE_01BothAxes, PARAM_MODE_01LargerAxis, or PARAM_MODE_Distance as requested by the facet options in effect.
//! @param [in,out] params parameters to remap.
//! @param [out] distanceRange range of parameters when scaled to distance (whether or not params are returned as distances)
//! @param [out] paramRange range of parameters as actually returned.
//! @param [in] xDistanceFactor scale factor to turn input x coordinates to distance (if distance requested either as final result or for larger axis scaling)
//! @param [in] yDistanceFactor scale factor to turn input y coordinates to distance (if distance requested either as final result or for larger axis scaling)
//! @param [out] transform optional transform (e.g. to be applied later to more params in the same parameter space) (May be nullptr)
GEOMDLLIMPEXP bool RemapPseudoDistanceParams
(
bvector<DPoint2d>&params,
DRange2dR distanceRange,
DRange2dR paramRange,
double xDistanceFactor,
double yDistanceFactor,
Transform *transform
);

GEOMDLLIMPEXP bool RemapPseudoDistanceParams
(
bvector<DPoint2d>&params,
DRange2dR distanceRange,
DRange2dR paramRange,
double xDistanceFactor = 1.0,
double yDistanceFactor = 1.0
)
    {
    return RemapPseudoDistanceParams (params, distanceRange, paramRange, xDistanceFactor, yDistanceFactor, nullptr);
    }



//! Add complete facets.
GEOMDLLIMPEXP bool Add (PolyfaceHeaderR polyface);
//! Return a system default facet construction object.  DEPRECATED -- use Create(options)
//! @param [in] options facet options
//! @param [in] pointMatchTolerance tolerance for testing for identical points (0.0 to use relative tolerance)
//! @param [in] paramMatchTolerance tolerance for testing for identical params (0.0 to use relative tolerance)
//! @param [in] normalMatchTolerance tolerance for testing for identical normals (0.0 to use relative tolerance)
GEOMDLLIMPEXP static IPolyfaceConstructionPtr New (IFacetOptionsR options, double pointMatchTolerance = 0.0, double paramMatchTolerance = 1.0E-12, double normalMatchTolerance = 1.0E-12);

//! Return a system default facet construction object.
//! @param [in] options facet options
//! @param [in] pointMatchTolerance tolerance for testing for identical points (0.0 to use relative tolerance)
//! @param [in] paramMatchTolerance tolerance for testing for identical params (0.0 to use relative tolerance)
//! @param [in] normalMatchTolerance tolerance for testing for identical normals (0.0 to use relative tolerance)
GEOMDLLIMPEXP static IPolyfaceConstructionPtr Create (IFacetOptionsR options, double pointMatchTolerance = 0.0, double paramMatchTolerance = 1.0E-12, double normalMatchTolerance = 1.0E-12);

};

//! Helper object for constructing facets from coordinate sources.
struct PolyfaceConstruction : IPolyfaceConstruction
{
protected:


bool m_triangulateQuads;

PolyfaceHeaderPtr m_polyfacePtr;
PolyfaceCoordinateMapPtr m_coordinateMapPtr;
IFacetOptionsPtr m_facetOptions;

// modal face index.
// used as index saved (with one-base offset) in facets.
// used a z component of parameters in parameter map, to prevent sharing of parameter values across faces.
size_t m_currentFaceIndex;
// modal face data.
FacetFaceData m_currentFaceData;

// Internal AddPolyface --- expects source to have all data matched and ready to load.
bool AddPolyface_matched (PolyfaceQueryCR source);

void _Clear ( ) override;
size_t _FindOrAddPoint (DPoint3dCR point) override;
size_t _FindOrAddNormal (DVec3dCR normal) override;
size_t _FindOrAddParam (DPoint2dCR param) override;
size_t _FindOrAddIntColor (uint32_t color) override;
size_t _AddPointIndex (size_t zeroBasedIndex, bool visible) override;
size_t _AddNormalIndex (size_t zeroBasedIndex) override;
size_t _AddParamIndex (size_t zeroBasedIndex) override;
size_t _AddColorIndex (size_t zeroBasedIndex) override;

size_t _AddSignedOneBasedPointIndex (int zeroBasedIndex) override;
size_t _AddSignedOneBasedNormalIndex (int zeroBasedIndex) override;
size_t _AddSignedOneBasedParamIndex (int zeroBasedIndex) override;
size_t _AddSignedOneBasedColorIndex (int zeroBasedIndex) override;

size_t _AddFaceIndex (size_t zeroBasedIndex) override;
size_t _AddPointIndexTerminator ( ) override;
size_t _AddNormalIndexTerminator ( ) override;
size_t _AddParamIndexTerminator ( ) override;
size_t _AddColorIndexTerminator ( ) override;
PolyfaceHeaderR _GetClientMeshR ( ) override;
PolyfaceHeaderPtr _GetClientMeshPtr ( ) override;
IFacetOptionsR _GetFacetOptionsR ( ) override;

size_t _AddPointIndexTriangle (size_t index0, bool visible0, size_t index1, bool visible1, size_t index2, bool visible2) override;
size_t _AddNormalIndexTriangle (size_t index0, size_t index1, size_t index2) override;
size_t _AddParamIndexTriangle (size_t index0, size_t index1, size_t index2) override;
size_t _AddColorIndexTriangle (size_t index0, size_t index1, size_t index2) override;
size_t _AddPointIndexQuad (size_t index0, bool visible0, size_t index1, bool visible1, size_t index2, bool visible2, size_t index3, bool visible3) override;
size_t _AddNormalIndexQuad (size_t index0, size_t index1, size_t index2, size_t index3) override;
size_t _AddParamIndexQuad (size_t index0, size_t index1, size_t index2, size_t index3) override;
size_t _AddColorIndexQuad (size_t index0, size_t index1, size_t index2, size_t index3) override;
bool   _AddPolyface (PolyfaceQueryCR polyface, size_t drawMethodIndex) override;


size_t _IncrementFaceIndex ( ) override;
void   _SetFaceIndex (size_t index) override;
void  _SetFaceData (FacetFaceDataCR data) override;
FacetFaceData _GetFaceData () const override;
size_t _GetFaceIndex () const override;
void _CollectCurrentFaceRanges () override;
void  _EndFace () override;




public:
//! Prefered construction object ...
PolyfaceConstruction (IFacetOptionsR options, double pointMatchTolerance, double paramMatchTolerance, double normalMatchTolerance);
};
END_BENTLEY_GEOMETRY_NAMESPACE
