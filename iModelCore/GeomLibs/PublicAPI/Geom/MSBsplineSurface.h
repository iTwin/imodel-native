/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*__PUBLISH_SECTION_END__*/

//! Complete data for a single patch extracted from a bspline surface.
//! May be used (in context) for either raw surface poles (with given non-uniform knots) or bezier poles.
struct BSurfPatch
    {
    //! support points
    bvector<DPoint4d> xyzw;
    //! u direction knots
    bvector<double> uKnots;
    //! v direction knots
    bvector<double> vKnots;
    //! curve order in u direction
    size_t uOrder;
    //! curve order in v direction
    size_t vOrder;
    //! patch position in u intervals
    size_t uIndex;
    //! patch position in v intervals
    size_t vIndex;
    //! low u knot represented by this patch
    double uMin;
    //! high u knot represented by this patch
    double uMax;
    //! low v knot represented by this patch
    double vMin;
    //! high v knot represented by this patch
    double vMax;
    //! flag set to true if this is a null interval (high multiplicity knot) in the u direction
    bool  isNullU;
    //! flag set to true if this is a null interval (high multiplicity knot) in the v direction
    bool  isNullV;

    bool Evaluate (DPoint2dCR uv, DPoint3dR xyz) const;
    bool Evaluate (DPoint2dCR uv, DPoint3dR xyz, DVec3dR dXdu, DVec3dR dXdv) const;
    DPoint2d PatchUVToKnotUV (DPoint2dCR patchUV) const;
    bool DoSaturate (); 

    // return estimates of how far interior control points are from midpoints of chords.
    // for each grid edge and diagonal, the deviation is point distance from chord midpoint, divided by shorter edge length, capped at 10.
    // 
    bool MidpointDeviations (double &uFraction, double &vFraction, double &twistFraction) const;
    };


typedef RefCountedPtr<struct PatchRangeData>    PatchRangeDataPtr;


//! Local Range data for each patch within a bspline surface.
//! May be used to accelerate calculations on patches.
struct PatchRangeData : RefCountedBase
    {
    //! ranges.
    bvector<TaggedLocalRange>    m_ranges;

    static PatchRangeDataPtr Create() { return new PatchRangeData(); }
    };

/*----------------------------------------------------------------------+
|                                                                       |
|   SSI Definitions                                                     |
|                                                                       |
+----------------------------------------------------------------------*/
struct boundBox                /* 120 words */
    {
    double      system[3][3];
    DPoint3d    origin;
    DPoint3d    extent;
    };

struct boundCyl                /* 64 words */
    {
    DPoint3d    org;        /* origin of cylinder */
    DPoint3d    dir;        /* direction of cylinder, unit vector */
    double      radius;     /* radius of cylinder */
    double      length;     /* length of cylinder axis */
    };

/*__PUBLISH_SECTION_START__*/

//! An entry in a double linked list of (parameteric intervals of) curves.
struct trimCurve
    {
    MSBsplineCurve      curve;
    double              intervalParams[2];
    struct trimCurve    *pNext;
    struct trimCurve    *pPrevious;
    EdgeId              trimEdgeId;
    };

//! Header for a linked list of trim curves.
struct bsurfBoundary
    {
    int32_t             numPoints;
    DPoint2d            *points;
    TrimCurve           *pFirst;
    };

typedef void*       CallbackArgP;
typedef void const* UserDataCP;

//! An entry in a double linked list of (complete) curves.
struct curveChain
    {
    MSBsplineCurve      curve;
    UserDataCP          userDataP;
    CurveChain*         nextP;
    CurveChain*         previousP;
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
typedef StatusInt   (*BSplineCallback_AskCurvePoints)
(
DPoint3dP       pointP,
DPoint3dP       derivativeP,
DPoint3dP       secondDerivativeP,
double          parameter,
CallbackArgP    userDataP
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
typedef StatusInt   (*BSplineCallback_AskSurfacePoints)
(
DPoint3dP       pointP,
DPoint3dP       uDerivativeP,
DPoint3dP       vDerivativeP,
DPoint3dP       uuDerivativeP,
DPoint3dP       vvDerivativeP,
DPoint3dP       uvDerivativeP,
double          uParameter,
double          vParameter,
CallbackArgP    userDataP
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
typedef StatusInt   (*BSplineCallback_AnnounceMeshQuadGrid)
(
DPoint3dP       pointsP,
DPoint3dP       normalsP,
DPoint2dP       parametersP,
int             nColumns,
int             nRows,
CallbackArgP    userDataP
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
typedef StatusInt   (*BSplineCallback_AnnounceMeshTriangleStrip)
(
DPoint3dP       pointsP,
DPoint3dP       normalsP,
DPoint2dP       parametersP,
int             nPoints,
CallbackArgP    userDataP
);



//! Ploles, knots, weigths, etc for a bspline surface (NOT REFCOUNTED)
//! This is the traditional C structure, instantated uninitialized on the stack.
//! @ingroup GROUP_Geometry
struct GEOMDLLIMPEXP MSBsplineSurface
    {
    int32_t             type;
    int32_t             rational;
    BsplineDisplay      display;
    BsplineParam        uParams;
    BsplineParam        vParams;
    DPoint3d            *poles;
    double              *uKnots;
    double              *vKnots;
    double              *weights;
    int32_t             holeOrigin;     /* true: bounds trim surface; false: bounds enclose holes */
    int32_t             numBounds;
    BsurfBoundary       *boundaries;
    
    //! zero out the surface    
    void Zero ();
    
    //! Allocate memory arrays to match the current counts.
    MSBsplineStatus Allocate   ();
    //! Allocate uKnots.  Prior knot buffer is freed if present (do not call on pre-zeroed surface!!)
    MSBsplineStatus AllocateUKnots ();
    //! Allocate vKnots.  Prior knot buffer is freed if present (do not call on pre-zeroed surface!!)
    MSBsplineStatus AllocateVKnots ();
    //! Free memory allocated for the poles, weights and knot vector of a B-spline surface. 
    void            ReleaseMem ();
    //! Allocate memory for the B-spline surface and copies all data from the input B-spline surface.
    MSBsplineStatus CopyFrom (MSBsplineSurfaceCR source);
    
    //! Returns a smart pointer to an MSBsplineSurface on the heap.
    static MSBsplineSurfacePtr CreatePtr ();

    //! Returns a smart pointer to an MSBsplineSurface on the heap.  Copy bits from instance, zero the instance.
    MSBsplineSurfacePtr CreateCapture ();
    
    //! Copy bits into simple structure. Caller instance zeroed.
    void ExtractTo (MSBsplineSurfaceR dest);
    
    
    //! Clone as new refcounted pointer.
    MSBsplineSurfacePtr CreateCopyTransformed (TransformCR transform) const;
/*__PUBLISH_SECTION_END__*/

    //! @DotNetMethodExclude
    static void SetAllocationFunctions (
            int  (*AllocateSurface)(MSBsplineSurface *),
            void (*FreeSurface)(MSBsplineSurface *)
            );
/*__PUBLISH_SECTION_START__*/
    
    //! Return the knot values corresponding to fraction 0 and fraction 1 about given direction...
    void GetKnotRange (double &min, double &max, int direction) const;
    //! Return the knot value at a fractional parameter about given direction...
    double FractionToKnot (double f, int direction) const;
    //! Return the fractional parameter corresponding to a knot value ...
    double KnotToFraction (double knot, int direction) const;
    //! Return the parameter region of the B-spline surface...
    void GetParameterRegion (double &uMin, double &uMax, double &vMin, double &vMax) const;
    //! Add a given knot value to the B-spline surface in given direction. newMultiplicity is the desired final 
    //! multiplicity of a knot that may already exist. 
    MSBsplineStatus AddKnot (double unnormalizedKnotValue, int newMultiplicity, int direction);

    //! Compute blending functions at KNOT value (unnormalized)
    //! knotIndex is the index of the knot to the right of u. (i.e. the leftmost knot of the upper limits of active windows)
    void KnotToBlendFunctions (double *blend, double *blendDerivatives, size_t &knotIndex, double u, int direction) const;
    //! Return the index of the knot at the left of the interval containing specified knot.
    //! When knotValue exactly matches a knot, the returned index is of the knot to the left -
    //!   i.e. knotValue appears at the RIGHT of the returned interval.
    //! (favor  knot[index] < knotValue <= knot[index + 1])
    size_t FindKnotInterval (double knotValue, int direction) const;


    //! Normalize knots to 01 (both directions)
    void NormalizeKnots ();
    
    //! Add a uv polyline trim boundary.
    bool AddTrimBoundary (bvector<DPoint2d> const &uvPoints);
    //! Add a uv polyline trim boundary, using only xyz parts
    bool AddTrimBoundary (bvector<DPoint3d> const &xyzPoints);
    //! Remove all trim boundaries.
    void DeleteBoundaries ();
    //! @param [in] active  If true, the outer boundary is active as an (implicit) trim boundary, so
    //!   the outermost explict boundary acts as a hole.
    void SetOuterBoundaryActive (bool active);

    //! @return the state of the outer boundary flag.
    bool IsOuterBoundaryActive () const;
    //! Return the area of a single boundary loop ..
    double BoundaryLoopArea (size_t boundaryIndex) const;
    //! Create B-spline surface by opening a closed B-spline surface about given direction.
    MSBsplineStatus CopyOpen (MSBsplineSurfaceCR source, double unnormalizedKnot, int edge);
    //! Create B-spline surface by closing a open B-spline surface.
    MSBsplineStatus CopyClosed (MSBsplineSurfaceCR source, int edge);
    //! Create B-spline surface by reversing the given direction of surface.
    MSBsplineStatus CopyReversed (MSBsplineSurfaceCR source, int edge);
    
    //! Check whether the B-spline surface is physically closed in either parameter direction. A B-spline surface may be 
    //! non-periodic, but still be closed in the v/u direction if its first and last rows/ columns of poles coincide. 
    void IsPhysicallyClosed (bool& uClosed, bool& vClosed);
    //! Check whether the B-spline surface encloses a valid space.
    bool    IsSolid (double tolerance);
    //! Check whether the poles are entirely within a plane.  This does not check for goofy direction changes -- just planarity.
    bool    IsPlane () const;
    //! Return a transform whose columns are along edges parallel to the principal axes and sized to include the poles from the translation point.
    bool    GetPrincipalExtents (TransformR extents) const;
    //! Check whether the B-spline surface is bilinear and each face of the control polygon is planar, with tight system tolerance (Angle::SmallAngle (
    bool    IsPlanarBilinear () const;
    //! Check whether the B-spline surface is bilinear and each face of the control polygon is planar, with caller specified tolerance
    bool    IsPlanarBilinear (double angleTol) const;
    //! Check whether the u curves are all translations of the first u curve.
    //! (If this is true, the v curves are also translations of the first v curve0)
    //! @param [in] relativeTolerance tolerance as a fraction of the largest coordinate.
    bool    IsBidirectionalTranslation(double relativeTolerance = 0.0) const;    

    //! Get the resolution of the B-spline surface.
    double Resolution () const;
    //! Get the resolution of the B-spline surface.
    double Resolution (double abstol, double reltol) const;
    //! Get the range of the poles of the B-spline surface.
    void GetPoleRange (DRange3dR range) const;
    //! Get the range of the transformed poles of the B-spline surface
    void GetPoleRange (DRange3dR range, TransformCR transform) const;
    //! Return an aligned range of (moderatly dense) points on the surface.
    //! Return range in principal system (See DPoint3dOps::PrincipalExtents)

    bool TightPrincipalExtents
    (
    TransformR originWithExtentVectors, //!< [out] (non-uniform scale) transform with origin at a corner of the aligned range, x,y,z along 3 along the entire length of the 3 perpendicular edges through that point.
    TransformR centroidAlLocalToWorld, //!< [out] unit coordinate frame with origin at an approximate centroid of points on the surface
    TransformR centroidalWorldToLocal, //!< [out] inverse of the centroidalLocalToWorld
    DRange3dR centroidalRange //!< [out] range of the surface in the centroidal frame.
    ) const;
    //! Return array of points on the surface.
    //! The points data is:
    //!<ul>
    //!<li>m_xyz = coordinates
    //!<li>m_f = index of bezier patch.
    //!<li>m_uv = surface parametric coordinate
    //!</ul>
    void EvaluatePoints (DPoint3dDoubleUVArrays &points, IFacetOptionsCP options = nullptr) const;

//! Return the true allocated size of the pole array....
    bool AreUKnotsValid (bool clampingRequired) const;
    bool AreVKnotsValid (bool clampingRequired) const;

    
    //! return the product of u and v direction poles counts.
    size_t GetNumPoles () const;
    //! return the u direction pole count.
    size_t GetNumUPoles () const;
    //! return the v direction pole count.
    size_t GetNumVPoles () const;

    //! return the u direction order.
    size_t GetUOrder () const;
    //! return the v direction order.
    size_t GetVOrder () const;
    //! return the u direction knot count.
    size_t GetNumUKnots () const;
    //! return the v direction knot count.
    size_t GetNumVKnots () const;

    //! return the product of u and v direction poles counts.
    int GetIntNumPoles () const;
    //! return the u direction pole count.
    int GetIntNumUPoles () const;
    //! return the v direction pole count.
    int GetIntNumVPoles () const;

    //! return the u direction order.
    int GetIntUOrder () const;
    //! return the v direction order.
    int GetIntVOrder () const;
    //! return the u direction knot count.
    int GetIntNumUKnots () const;
    //! return the v direction knot count.
    int GetIntNumVKnots () const;

    //! return the u direction periodic state.
    bool GetIsUClosed () const;
    //! return the v direction periodic state.
    bool GetIsVClosed () const;

    //! ask if this is a "rational" (weighted) surface.
    bool HasWeights () const;




    void SetNumRules (int numU, int numV);
    void SetSurfaceDisplay (bool display);
    void SetPolygonDisplay (bool display);
    bool GetSurfaceDisplay () const;
    bool GetPolygonDisplay () const;


    DPoint3d GetPole (size_t i) const;
    DPoint3d GetPole (int i) const;
    double   GetWeight (size_t i) const;
    double   GetWeight (int i) const;
    DPoint3d GetPole (size_t i, size_t j) const;
    DPoint3d GetUnWeightedPole (size_t i, size_t j) const;
    DPoint3d GetUnWeightedPole (size_t i) const;
    DPoint3d GetUnWeightedPole (int i) const;
    //! Transform a block of poles wtih start index i0,j0
    void TransformPoles (TransformCR transform, size_t i0, size_t j0, size_t numI, size_t numJ);
    //! Try to dereference index i,j.
    bool TryGetUnWeightedPole (size_t i, size_t j, DPoint3dR xyz) const;
    //! Try to dereference index i.
    bool TryGetUnWeightedPole (size_t i, DPoint3dR xyz) const;
    double GetWeight (size_t i, size_t j) const;
    DPoint4d GetPoleDPoint4d (size_t i) const;
    DPoint4d GetPoleDPoint4d (int i) const;
    DPoint4d GetPoleDPoint4d (size_t i, size_t j) const;
    double GetUKnot (size_t i) const;
    double GetVKnot (size_t i) const;

    void GetUKnots (bvector<double> &knots) const;
    void GetVKnots (bvector<double> &knots) const;
    //! Copy all poles out into caller array.
    void GetPoles (bvector<DPoint3d> &outData) const;
    //! Copy all weights out into caller array.
    void GetWeights (bvector<double> &outData) const;        
    //! Copy all poles out into caller array, dividing each by its weight
    void GetUnWeightedPoles (bvector<DPoint3d> &outData) const;



    //! set poles by index. returns false if any index out of range.
    //! @DotNetMethodExclude
    bool SetPoles (size_t index, DPoint3dCP value, size_t n);
    //! set poles by index. returns false if any index out of range.
    bool SetPoles (bvector<DPoint3d> const &poles);


    //! set pole by index. returns false if index out of range.
    //! @DotNetMethodExclude
    bool SetPole (size_t index, DPoint3dCR value);
    bool SetPole (int index, DPoint3dCR value);
    bool SetPole (size_t i, size_t j, DPoint3dCR value);
    //! set pole by index. returns false if index out of range.  If the surface is weighted, the current weight is multiplied
    //! into the input pole.
    bool SetReWeightedPole (size_t index, DPoint3dCR value);
    bool SetReWeightedPole (size_t i, size_t j, DPoint3dCR value);
    bool SetReWeightedPole (int index, DPoint3dCR value);
        
    //! set pole by index. returns false if index out of range.
    //! @DotNetMethodExclude
    bool SetPole (size_t index, double x, double y, double z);
    bool SetPole (int index, double x, double y, double z);

    //! set weight by index. returns false if index out of range.
    //! @DotNetMethodExclude
    bool SetWeight (size_t index, double w);
    bool SetWeight (int index, double w);

    //! set weight by index. returns false if any index out of range.
    //! @DotNetMethodExclude
    bool SetWeights (size_t index, double const * value, size_t n);    
    //! set uKnotw by index. returns false if any index out of range.
    //! @DotNetMethodExclude
    bool SetUKnots (size_t index, double const * value, size_t n);    
    //! set vKnotw by index. returns false if any index out of range.
    //! @DotNetMethodExclude
    bool SetVKnots (size_t index, double const * value, size_t n);    

    //! if the surface is rational, divide (wx,wy,wz) style poles by the weights
    void UnWeightPoles ();
    //! if the surface is rational, multiply (wx,wy,wz) style poles by the weights
    void WeightPoles ();

        

    //! Check whether an edge of the surface degenerates to a single point. 
    bool    IsDegenerateEdge (int edgeCode, double tolerance);
    
    //! Calculate the point on the B-spline surface at the input u and v parameter values.
    void EvaluatePoint (DPoint3dR xyz, double u, double v) const;

    //! Compute a grid of points uniformly spaced in each parameter direction.
    //! @param [in] numUPoint number of points in u direction.
    //! @param [in] numVPoint number of points in v direction
    //! @param [out] uParams u-direction evaluation parameters.
    //! @param [out] vParams v-direction evaluation parameters.
    //! @param [out] gridPoints 
    void EvaluateUniformGrid (size_t numUPoint, size_t numVPoint, bvector<double> &uParams, bvector<double> &vParams, bvector<DPoint3d> &gridPoints) const;

    //! Compute a grid of points uniformly spaced in each parameter direction.
    //! @param [in] numUPoint number of points in u direction.
    //! @param [in] numVPoint number of points in v direction
    //! @param [out] uvParams computed paramters.
    //! @param [out] gridPoints computed points.
    void EvaluateUniformGrid (size_t numUPoint, size_t numVPoint, bvector<DPoint2d> &uvParams, bvector<DPoint3d> &gridPoints) const;

    //! Calculate the point and unit normal on the B-spline surface at the input u and v parameter values.
    //! Return false if tangent vectors are parallel or zero.
    bool EvaluatePointAndUnitNormal (DRay3dR ray, double u, double v) const;

    //! Calculate the knot values at fractional position within a control polygon quad.
    ValidatedDPoint2d ControlPolygonFractionToKnot (size_t i, size_t j, double u, double v) const;

    //! Calculate the control polygon point at fractional position within a control polygon quad.
    ValidatedDPoint4d ControlPolygonFractionToControlPolygonDPoint4d (size_t i, size_t j, double u, double v) const;

    //! Calculate the control polygon point at fractional position within a control polygon quad.
    ValidatedDPoint3d ControlPolygonFractionToControlPolygonDPoint3d (size_t i, size_t j, double u, double v) const;


    //! Calculate a coordinate frame on the surface.
    //! @param [in] u u parameter
    //! @param [in] v v parameter.
    //! @param [out] transform transform with (a) origin at surface point, (b) x axis in u direction, (c) y axis perpendicular to x and in the surface tangent plane, (d) z axis normal to surface.
    //! Return false if tangent vectors are parallel or zero.
    bool EvaluateNormalizedFrame(TransformR transform, double u, double v) const;


    //! Calculate the point and its partial derivatives on the B-spline surface at the input u and v parameter values. 
    void EvaluatePoint (DPoint3dR xyz, DVec3dR dPdU, DVec3dR dPdV, double u, double v) const;
    //! Calculate the position and partial derivatives of a B-spline surface at a particular uv parameter value pair.
    void EvaluateAllPartials (DPoint3dR xyz, DVec3dR dPdU, DVec3dR dPdV, 
                                DVec3dR dPdUU, DVec3dR dPdVV, DVec3dR dPdUv,
                                    DVec3dR norm, double u, double v) const;
    //! Evaluate the directions and curvatures (inverse radius)                                    
    //! @return true if this is a nonsingular surface point.
    bool EvaluatePrincipalCurvature
        (
        DPoint3dR xyz,      //!< [out] point on surface
        DVec3dR unitA,      //!< [out] unit vector tangent to surface and in the direction of the curvature with larger absolute value.
        double &curvatureA,  //!< [out] curvature corresponsing to unitA.
        DVec3dR unitB,      //!< [out] unit vector tangent to surface and in the direction of the curvature with smaller absolute value
        double &curvatureB,  //!< [out] curvature corresponsing to unitB.
        double u,           //!< [in] u parameter on surface
        double v            //!< [in] v parameter on surface
        ) const;
    //! Elevate the degree (increases the order) of the B-spline surface in given direction.
    MSBsplineStatus ElevateDegree (int newDegree, int edge);
    //! Clean all unnecessary knots.
    MSBsplineStatus CleanKnots ();
    //! Remove all removable knots with the tolerance constraint.
    MSBsplineStatus RemoveKnotsBounded (int dir, double tol);
    
    //! Find closest point on surface
    void ClosestPoint (DPoint3dR surfacePoint, DPoint2dR surfaceUV, DPoint3dCR spacePoint) const;



    //! Initialize the B-spline surface from point array and U/V order.
    MSBsplineStatus InitFromPointsAndOrder (int uOrder, int vOrder,
          int uNumPoles, int vNumPoles, DPoint3dP pPoints);

    //! Copy as a refCounted surface
    MSBsplineSurfacePtr Clone () const;

    //! Allocate and initialize a refcounted surface
    static MSBsplineSurfacePtr CreateFromPolesAndOrder
        (
        bvector<DPoint3d> const &pointVector,   //!< [in] control polygon coordinates
        bvector<double> const *weightVector,   //!< [in] optinal control polygon weightsweights
        bvector<double> const *uKnotVector,   //!< [in] optional u direction knot vector 
        int uOrder,   //!< [in] order (one more than degree) in the u direction
        int numUPoints,   //!< [in] number of control points in the u direction
        bool uClosed,   //!< [in]  true if closed in the u direction
        bvector<double> const *vKnotVector,   //!< [in] optional v direction knot vector
        int vOrder,   //!< [in] order (one more than degree) in v direction
        int numVPoints,   //!< [in] number of control points in v direction
        bool vClosed,   //!< [in] true if closed in the v direction
        bool inputPolesAlreadyWeighted   //!< [in] true if points are preweighted.  False if not.
        );
        
    //! Create a linear sweep from a (single) base curve.
    //! Fails (i.e. returns NULL) if the primitive has children.
    //! @param [in] primitive base curve to be swept
    //! @param [in] delta sweep direction.
    static MSBsplineSurfacePtr CreateLinearSweep (ICurvePrimitiveCR primitive, DVec3dCR delta);
    //! Create a linear sweep from a (single) base curve.
    //! @param [in] primitive base curve to be swept
    //! @param [in] delta sweep direction.
    static MSBsplineSurfacePtr CreateLinearSweep (MSBsplineCurveCR primitive, DVec3dCR delta);
    //! Create (possbily many) bspline surfaces for linear sweeps from base curves.
    //! @param [out] surfaces growing array of surfaces.
    //! @param [in] baseCurves base curves.  All primitives within are swept.
    //! @param [in] delta sweep vector.
    static bool CreateLinearSweep (bvector<MSBsplineSurfacePtr> &surfaces, CurveVectorCR baseCurves, DVec3dCR delta);

    //! Create a linear sweep from a ruled surface between two curves.
    //! Fails (i.e. returns NULL) if the primitives have children or are not compatible.
    //! @param [in] curveA first curve
    //! @param [in] curveB second curve
    static MSBsplineSurfacePtr CreateRuled (ICurvePrimitiveCR curveA, ICurvePrimitiveCR curveB);



    //! Create a rotational sweep from a (single) base curve.
    //! Fails (i.e. returns NULL) if the primitive has children.
    //! @param [in] primitive base curve to be rotated.
    //! @param [in] center point on axis of rotation.
    //! @param [in] axis axis of rotation
    //! @param [in] sweepRadians sweep angle.
    static MSBsplineSurfacePtr CreateRotationalSweep (ICurvePrimitiveCR primitive,
        DPoint3dCR center, DVec3dCR axis, double sweepRadians);
    //! Create a rotational sweep from a (single) base curve.
    //! @param [in] primitive base curve to be rotated.
    //! @param [in] center point on axis of rotation.
    //! @param [in] axis axis of rotation
    //! @param [in] sweepRadians sweep angle.
    static MSBsplineSurfacePtr CreateRotationalSweep (MSBsplineCurveCR primitive,
        DPoint3dCR center, DVec3dCR axis, double sweepRadians);
    //! Create (possbily many) bspline surfaces for rotational sweeps from base curves.
    //! @param [out] surfaces growing array of surfaces.
    //! @param [in] baseCurves base curves.  All primitives within are swept.
    //! @param [in] center point on axis of rotation.
    //! @param [in] axis axis of rotation
    //! @param [in] sweepRadians sweep angle.
    static bool CreateRotationalSweep (bvector<MSBsplineSurfacePtr> &surfaces, CurveVectorCR baseCurves,
        DPoint3dCR center, DVec3dCR axis, double sweepRadians);

    //! Create a planar (bilinear) surface for the parallelogram around the ellipse.
    //! Insert a trim curve for the ellipse.
    //! @param [in] ellipse space ellipse
    static MSBsplineSurfacePtr CreateTrimmedDisk (DEllipse3dCR ellipse);

      //! Create a surface swept along a trace curve.
      //! @param [in] baseCurve base contour
      //! @param [in] translateBaseCurve true to translate section, false to rotate with trace
      //! @param [in] traceCurve path to sweep
      static MSBsplineSurfacePtr CreateTubeSurface (MSBsplineCurveCR baseCurve, bool translateBaseCurve, MSBsplineCurveCR traceCurve);

    //! Populate the B-spline surface with the given parameters.
    //! @DotNetMethodParameterVectorIsInputArray{pointVector}
    //! @DotNetMethodParameterVectorIsInputArray{weightVector}
    //! @DotNetMethodParameterVectorIsInputArray{uKnotVector}
    //! @DotNetMethodParameterVectorIsInputArray{vKnotVector}
    //! @DotNetMethodExclude
    MSBsplineStatus Populate
        (
        bvector<DPoint3d> const &pointVector,
        bvector<double> const *weightVector,
        bvector<double> const *uKnotVector,
        int uOrder,
        int numUPoints,
        bool uClosed,
        bvector<double> const *vKnotVector,
        int vOrder,
        int numVPoints,
        bool vClosed,
        bool inputPolesAlreadyWeighted
        );

    //! Open the closed B-spline surface about the given direction.
    MSBsplineStatus MakeOpen (double uv, int direction);
    //! Close the open B-spline surface about the given direction.
    MSBsplineStatus MakeClosed (int direction);
    //! Make an equivalent rational B-spline surface.
    MSBsplineStatus MakeRational ();
    //! Swap the U/V direction of the surface.
    MSBsplineStatus SwapUV ();
    //! Reserve the given direction of the surface.
    MSBsplineStatus MakeReversed (int direction);
    //! Create equivalent Bézier surface for the B-spline surface.
    MSBsplineStatus MakeBezier (MSBsplineSurfaceP outSurface);
    //! Create a series of Bézier surfaces for the B-spline surface.
    MSBsplineStatus MakeBeziers (bvector<MSBsplineSurface>& beziers) const;
    
    //! Extract the poles and knots that support a single bezier patch ...
    //! @param [out] outPoles {uOrder X vOrder} poles
    //! @param [out] outUKnots {2*(uOrder-1)} knots
    //! @param [out] outVKnots {2*(vOrder-1)} knots
    //! @param [in] uIndex index of bezier to extract.
    //! @param [in] vIndex index of bezier to extract.
    //! @DotNetMethodExclude
    bool GetSupport (bvector<DPoint4d>& outPoles, bvector<double>& outUKnots, bvector<double>& outVKnots, size_t uIndex, size_t vIndex) const;

    //! Return the number of u and v intervals, i.e. to use as loop limits when calling GetBezier.
    //! @remark Note that there may be "null" intervals (due to high multiplicity knots) that reduce the number of real knots.  The null interval flags
    //!    returned by GetBezier will identify these.
    //! @param [out] numU number of u intervals.
    //! @param [out] numV number of v intervals.
    bool GetIntervalCounts (size_t &numU, size_t &numV) const;

    //! Populate a single bezier patch.
    //! @param [out] patchData complete description of the patch.   Allocated (usually on stack) in caller space.  This exrtraction function begins by clearing all prior contents.
    //! @param [in] uIndex patch index in u direction.
    //! @param [in] vIndex patch index in v direction.
    //! @remark When either knot interval is null, beware of the return state.
    //! <pre>
    //! <ul>
    //! <li>The function return is TRUE.
    //! <li>There may be non-null intervals with higher uIndex and vIndex.
	//! <li>That is, the function value is expected to be used for loop control, with the understanding
	//!     that intermediate index values may be perfectly valid as "support" queries but are a non-strokable interval.
    //! <li>The xyzw values are saturated in the direction(s) of non-null knot interval.
    //! </ul>
    //! </pre>
    //! The uMin and uMax values in the patch are the ends of the requested knot interval.
    //! The uKnots and vKnots are "presaturation". That is, they carry history (i.e. may be useful for future knot removal steps?)
    bool GetPatch (BSurfPatch& patchData, size_t uIndex, size_t vIndex) const;

    //! Transform the B-spline surface.
    MSBsplineStatus TransformSurface (Transform const *transformP);
    //! Transform the B-spline surface.
    MSBsplineStatus TransformSurface (TransformCR transform);



    //! Scale and translate the parameter range of the surface and its boundary loops so all parameters are between 0 and 1. 
    void NormalizeSurface ();
    

    //! Compute uniformly spaced knots.
    //! This uses counts from params.
    //! @return false if param counts are not set.
    bool ComputeUniformUKnots ();
    //! Compute uniformly spaced knots.
    //! This uses counts from params.
    //! @return false if param counts are not set.
    bool ComputeUniformVKnots ();

    //! Install counts and set up uniform knots in u direction.
    bool SetUParamsWithUniformKnots (size_t numPoles, size_t order, bool closed);
    //! Install counts and set up uniform knots in v direction.
    bool SetVParamsWithUniformKnots (size_t numPoles, size_t order, bool closed);


//! Make a loft surface for a set of curves.
//! @param [in] curves input curve array.
//! @param [in] pStartNormal used when smoothStart is true only, or NULL.
//! @param [in] pEndNormal used when smoothEnd is true only, or NULL.
//! @param [in] approxComp true: use approximation for compitibility.
//! @param [in] closed true: consider the last curve to be the same as the first curve.
//! @param [in] smoothStart true: for degenerate end, use normal to control the tangent.
//! @param [in] smoothEnd true: for degenerate end, use normal to control the tangent.
//! @param [in] chordLength true: chord length parametrization. false: uniform.
//! @param [in] applyComp false means that input curves are already compatible.
//! @param [in] tolerance used only when approxComp is true (can be 0.0 for precise compatibility).
MSBsplineStatus InitLoftingSurface
(
bvector<MSBsplineCurvePtr> const &curves,        
DVec3dP             pStartNormal,   
DVec3dP             pEndNormal,     
bool                approxComp,     
bool                closed,         
bool                smoothStart,    
bool                smoothEnd,      
bool                chordLength,    
bool                applyComp,      
double              tolerance       
);
                                    
    //! Return the area, centroid, orientation, and principal moments, treating the surface as a shell.
    //! @param [out] area area
    //! @param [out] centroid centroid
    //! @param [out] axes columns of this matrix are the principal directions.
    //! @param [out] momentxyz moments (yy+zz,xx+zz,xx+yy) around the principal directions.
    //! @return false if (a) no points when meshed mesh
    bool ComputePrincipalAreaMoments (
                                  double &area,
                                  DVec3dR centroid,
                                  RotMatrixR axes,
                                  DVec3dR momentxyz
                                  ) const;                                    

    //! Return the integrals of products of inertia [xx xy xz x; xy yy yz y; xz yz zz 1] * dA
    bool ComputeSecondMomentAreaProducts (DMatrix4dR products) const;

/*__PUBLISH_SECTION_END__*/
    //! Return the integrals of products of inertia [xx xy xz x; xy yy yz y; xz yz zz 1] * dA.
    bool ComputeSecondMomentAreaProducts
        (
        DMatrix4dR products,
        double relativeTolerancefForFacets,
        int numGauss,
        int &numEvaluations
        ) const;      
    bool GetPatchSupport (BSurfPatch& patchData, size_t uIndex, size_t vIndex) const;
    void FastIntersectRay (DRay3dCR ray, PatchRangeDataPtr& patchRangeData, bvector<double> &rayParameters, bvector<DPoint2d> &surfaceParameters, bvector<DPoint3d> &surfaceXYZ) const;
    
//! Intesect curve, with cached range data.
void FastIntersectCurve
(
ICurvePrimitiveCR curve,                    //!< [in] curve primitive.
PatchRangeDataPtr& patchRangeData,          //!< [in,out] cached mesh details
bvector<CurveAndSolidLocationDetail> &curvePoints     //!< [out] hit points on curve
) const;


    PatchRangeDataPtr ComputePatchRangeData() const;
     
                  
/*__PUBLISH_SECTION_START__*/

    //! Convert regions in the curve vector to trimmed bspline surfaces.
    //! @param [out] surfaces array to receive bspline surfaces.
    //! @param [in] source curve vector with area regions
    //! @param [in] options optional controls for curve stroking.
    //! @return true if successful conversion.
    //! @return true if the entire curve primitive was converted.
    static bool CreateTrimmedSurfaces (bvector <MSBsplineSurfacePtr> &surfaces, CurveVectorCR source, IFacetOptionsP options = NULL);

    //! Convert surfaces of the SolidPrimitive to trimmed bspline surfaces.
    //! @param [out] surfaces array to receive bspline surfaces.
    //! @param [in] source curve vector with area regions
    //! @param [in] options optional controls for curve stroking.
    //! @return true if the entire solid primitive was converted.
    static bool CreateTrimmedSurfaces (bvector <MSBsplineSurfacePtr> &surfaces, ISolidPrimitiveCR source, IFacetOptionsP options = NULL);

    //! Return the surface boundary, optionally in cubic spline fit (rather than just sampled points).
    //! The returned CurveVectorPtr is a BOUNDARY_TYPE_None.  It contains individual loops.
    //! @remark The curves are returned as unstructured list -- no analysis of loop closure.
    //! @remark The tolerance is for curve to the points, NOT for surface tolerance.
    CurveVectorPtr GetUnstructuredBoundaryCurves (double tolerance, bool cubicFit) const;
    
    //! Return the surface boundary, optionally in cubic spline fit (rather than just sampled points).
    //! The returned CurveVectorPtr is a BOUNDARY_TYPE_None.  It contains individual loops.
    //! @param tolerance tolerance for the cubic curve fit.
    //! @param cubicFit true to fit a cubic when possible.
    //! @param addOuterLoopIfActive true to add the outer edges when they are boundary edges.
    //! @remark The curves are returned as unstructured list -- no analysis of loop closure.
    //! @remark The tolerance is for curve to the points, NOT for surface tolerance.
    CurveVectorPtr GetUnstructuredBoundaryCurves (double tolerance, bool cubicFit, bool addOuterLoopIfActive) const;


    //! Return boundary count.  This does NOT include an implied outer -- it is exactly what is stored.
    int GetIntNumBounds () const;
    size_t GetNumBounds () const;
    //! return the number of ponits in a boundary.
    size_t GetNumPointsInBoundary (size_t boundaryIndex) const;
    int GetIntNumPointsInBoundary (int boundaryIndex) const;
    //! return pointer to boundary loop data.
    DPoint2dCP GetBoundaryUVCP (size_t boundaryIndex) const;
    DPoint2dCP GetBoundaryUVCP (int boundaryIndex) const;
    //! Return a single boundary uv.   Return 00 if out of range.
    DPoint2d GetBoundaryUV (size_t boundaryIndex, size_t pointIndex) const;
    DPoint2d GetBoundaryUV (int boundaryIndex, int pointIndex) const;
    //! guarded access to a single boundary uv.
    bool TryGetBoundaryUV (size_t boundaryIndex, size_t pointIndex, DPoint2dR uv) const;
    bool TryGetBoundaryUV (int boundaryIndex, int pointIndex, DPoint2dR uv) const;    
    
    //! Return current uv boundary data.
    void GetUVBoundaryLoops (bvector< bvector<DPoint2d> > &uvBoundaries, bool addOuterLoopsIfActive) const;
    //! Return uv boundary data, optionally cleaned up by parity analysis.
    void GetUVBoundaryLoops (bvector< bvector<DPoint2d> > &uvBoundaries, bool addOuterLoopsIfActive, bool cleanupParity) const;
    //! Analyze loop parity among all boundary loops.  Replace as "complete" loop set -- no implicit 01 outer boundary.
    void FixupBoundaryLoopParity ();

    //! Return current uv boundary data.  This returns a parity region.
    CurveVectorPtr GetUVBoundaryCurves (bool addOuterLoopsIfActive, bool preferCurves) const;

    //! Delete previous trim and add new trim.
    void SetTrim (CurveVectorR curves);

    //! Copy poles from a row into a curve structure. index -1 is understood as "end"
    MSBsplineCurvePtr GetPolygonRowAsCurve (int index) const;
    //! Copy poles from a column into a curve structure. index -1 is understood as "end"
    MSBsplineCurvePtr GetPolygonColumnAsCurve (int index) const;

    //! Get the (untrimmed) v-direction curve at u
    MSBsplineCurvePtr GetIsoUCurve (double u) const;
    //! Get the (untrimmed) u-direction curve at v
    MSBsplineCurvePtr GetIsoVCurve (double v) const;
    //! Get scan line intersections at constant u.
    void GetIsoULineVIntersections (double u, bvector<double> &vParams) const;
    //! Get scan line intersections at constant v.
    void GetIsoVLineUIntersections (double v, bvector<double> &uParams) const;

    //! Get scan line intersections at constant u.
    void GetIsoUCurveSegments (double u, bvector<MSBsplineCurvePtr> &segments) const;
    //! Get scan line intersections at constant v.
    void GetIsoVCurveSegments (double v, bvector<MSBsplineCurvePtr> &segments) const;


    
    //! Collect intersections with a ray.
    void IntersectRay (
        bvector<DPoint3d> &intersectionPoints,  //!< returned intersection points
        bvector<double>   &rayParameters,       //!< returned parameters on the ray
        bvector<DPoint2d> &surfaceParameters,   //!< returned parameters on the surface
        DRay3dCR ray                            //!< ray to intesect.
        ) const;

    //! Collect intersections with an interval of a ray.
    void IntersectRay (
        bvector<DPoint3d> &intersectionPoints,  //!< returned intersection points
        bvector<double>   &rayParameters,       //!< returned parameters on the ray
        bvector<DPoint2d> &surfaceParameters,   //!< returned parameters on the surface
        DRay3dCR ray,                            //!< ray to intesect.
        DRange1dCR rayInterval                  //!< parameter range on ray.
        ) const;

    bool HasValidPoleAllocation () const;
    bool HasValidWeightAllocation () const;
    bool HasValidBoundaryAllocation () const;
    bool HasValidKnotAllocation () const;
    bool HasValidOrder () const;
    bool HasValidPoleCounts () const;
    bool HasValidCountsAndAllocations () const;

    //! Compare all non-coordinate data.
    bool IsSameStructure (MSBsplineSurfaceCR other) const;
    //! Compare all data.
    bool IsSameStructureAndGeometry (MSBsplineSurfaceCR other, double tolerance) const;

    }; //MSBsplineSurface

//! MSBsplineSurface with IRefCounted support for smart pointers.
//! Create via MSBsplineSurface::CreatePtr ();
struct RefCountedMSBsplineSurface : public MSBsplineSurface, RefCountedBase
    {
/*__PUBLISH_SECTION_END__*/
friend struct MSBsplineSurface;
/*__PUBLISH_SECTION_START__*/ 
    protected:
    RefCountedMSBsplineSurface ();
    ~RefCountedMSBsplineSurface ();
    };

/*__PUBLISH_SECTION_END__*/

void GEOMDLLIMPEXP bspsurf_setTimerControl (int select, int count);
//! Double linked list of bspline surfaces.
struct surfaceChain
    {
    MSBsplineSurface    surface;
    UserDataCP          userDataP;
    SurfaceChain*       nextP;
    SurfaceChain*       previousP;
    };
/*__PUBLISH_SECTION_START__*/
END_BENTLEY_GEOMETRY_NAMESPACE

