/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/SolidPrimitive.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
// ISolidPrimtivePtr is a pointer to a reference counted object that is a carrier for a solid primitive.
//
// Each solid primitive Xxxxx has a corresponding XxxxDetail and SolidPrimitiveType_Xxxx:
//      DgnTorusPipe DgnTorusPipeDetail   SolidPrimitiveType_DgnTorusPipe
//      DgnCone DgnConeDetail   SolidPrimitiveType_DgnCone
//      DgnBox DgnBoxDetail   SolidPrimitiveType_DgnBox
//      DgnSphere DgnSphereDetail   SolidPrimitiveType_DgnSphere
//      DgnExtrusion DgnExtrusionDetail   SolidPrimitiveType_DgnExtrusion
//      DgnRotationalSweep DgnRotationalSweepDetail   SolidPrimitiveType_DgnRotationalSweep
//      DgnRuledSweep DgnRuledSweepDetail   SolidPrimitiveType_DgnRuledSweep
//
// Each XxxxxDetail struct exposes its contents as public members and has a constructor that takes
//     the complete list of individual members as parameters.
//
// The ISolidPrimitivePtr interface provides a corresponding static Create method to allocate
//   a reference counted object of respective type.  The create method takes the corresponding
//   detail structure as it sone input parameter:
//      ISolidPrimitivePtr myDgnTorusPipe = ISolidPrimitive::CreateDgnTorusPipe(DgnTorusPipeDetailCR data);
//      ISolidPrimitivePtr myDgnCone = ISolidPrimitive::CreateDgnCone(DgnConeDetailCR data);
//      ISolidPrimitivePtr myDgnBox = ISolidPrimitive::CreateDgnBox(DgnBoxDetailCR data);
//      ISolidPrimitivePtr myDgnSphere = ISolidPrimitive::CreateDgnSphere(DgnSphereDetailCR data);
//      ISolidPrimitivePtr myDgnExtrusion = ISolidPrimitive::CreateDgnExtrusion(DgnExtrusionDetailCR data);
//      ISolidPrimitivePtr myDgnRotationalSweep = ISolidPrimitive::CreateDgnRotationalSweep(DgnRotationalSweepDetailCR data);
//      ISolidPrimitivePtr myDgnRuledSweep = ISolidPrimitive::CreateDgnRuledSweep(DgnRuledSweepDetailCR data);
//
//! Enumerated type code for various solid primitive types.
enum SolidPrimitiveType
{
SolidPrimitiveType_None,
SolidPrimitiveType_DgnTorusPipe,
SolidPrimitiveType_DgnCone,
SolidPrimitiveType_DgnBox,
SolidPrimitiveType_DgnSphere,
SolidPrimitiveType_DgnExtrusion,
SolidPrimitiveType_DgnRotationalSweep,
SolidPrimitiveType_DgnRuledSweep,
};

//! @ingroup GROUP_Geometry
typedef RefCountedPtr<ISolidPrimitive> ISolidPrimitivePtr;

//! A DgnTorusPipeDetail represents a pipe elbow as a torus with partial sweep in the major circle and full circle of pipe.
struct DgnTorusPipeDetail : ZeroInit<DgnTorusPipeDetail>
{
DPoint3d        m_center;       //!< Center of primary circle 
DVec3d          m_vectorX;      //!< X vector of primary circle 
DVec3d          m_vectorY;      //!< Y vector of primary circle 
double          m_majorRadius;  //!< radius of elbow 
double          m_minorRadius;  //!< radius of pipe section 
double          m_sweepAngle;   //!< major circle sweep angle 
bool            m_capped;       //!< cap surface present (e.g. for incomplete sweep)

//! Detail consructor with complete field list as parameters ...
//! @param [in] center Center of primary circle
//! @param [in] vectorX X vector of primary circle
//! @param [in] vectorY Y vector of primary circle
//! @param [in] majorRadius radius of elbow
//! @param [in] minorRadius radius of pipe section
//! @param [in] sweepAngle major circle sweep angle
//! @param [in] capped true for solid
GEOMDLLIMPEXP DgnTorusPipeDetail 
    (
    DPoint3dCR  center,
    DVec3dCR    vectorX,
    DVec3dCR    vectorY,
    double      majorRadius,
    double      minorRadius,
    double      sweepAngle, 
    bool        capped
    );

//! Detail consructor with complete field list as parameters ...
//! @param [in] arc primary arc.
//! @param [in] minorRadius radius of pipe section
//! @param [in] capped true for solid
GEOMDLLIMPEXP DgnTorusPipeDetail 
    (
    DEllipse3dCR arc,
    double      minorRadius,
    bool        capped
    );


//! Default value constructor.
GEOMDLLIMPEXP DgnTorusPipeDetail ();

//! Return range.
GEOMDLLIMPEXP bool GetRange (DRange3dR range) const;
//! Return range when transformed
GEOMDLLIMPEXP bool GetRange (DRange3dR range, TransformCR transform) const;
//! Return coordinate system with origin at major hoop center, X axis towards starting minor circle center.
GEOMDLLIMPEXP bool TryGetConstructiveFrame (TransformR localToWorld, TransformR worldToLocal) const;

//! Test for same type and structure (but no coordinate comparison)
 GEOMDLLIMPEXP bool IsSameStructure (ISolidPrimitiveCR other) const;
//! Test for same type, structure and coordinates.
GEOMDLLIMPEXP bool IsSameStructureAndGeometry (ISolidPrimitiveCR other, double tolerance) const;

//! Test if this is a closed volume.
GEOMDLLIMPEXP bool IsClosedVolume () const;
//! Return +1 or -1 that matches the volume sign after integration over the parameter space.
GEOMDLLIMPEXP double ParameterizationSign () const;


//! Return true if capped and incomplete major sweep.
GEOMDLLIMPEXP bool HasRealCaps () const;
//! Return full ellipse on minor hoop at fractional position along major circle.
GEOMDLLIMPEXP DEllipse3d UFractionToVSectionDEllipse3d (double fraction) const;

//! Return full ellipse on minor hoop at fractional position along minor circle.
GEOMDLLIMPEXP DEllipse3d VFractionToUSectionDEllipse3d (double fraction) const;

//! @param center OUT center of rotation.
//! @param axis OUT axis of rotation.
//! @param sweepRadians OUT angle of rotation.
GEOMDLLIMPEXP bool TryGetRotationAxis (DPoint3dR center, DVec3dR axis, double &sweepRadians) const;

//! @param [out] center center of rotation.
//! @param [out] axes coordinate axes, xy in major plane, z through hole.
//! @param [out] radiusA major radius (elbow radius)
//! @param [out] radiusB minor radius (pipe diameter)
//! @param [out] sweepRadians angle of rotation.
GEOMDLLIMPEXP bool TryGetFrame
    (
    DPoint3dR   center,
    RotMatrixR  axes,
    double&     radiusA,
    double &    radiusB,
    double &    sweepRadians
    ) const;

GEOMDLLIMPEXP bool TryGetFrame
(
TransformR localToWorld,
TransformR worldToLocal,
double &radiusA,
double &radiusB,
double &sweepRadians
) const;

//! Compute intersections with a ray and add to the pickData.
//! @param [in] ray origin and direction
//! @param [in] minParameter smallest parameter of pick.  (e.g. 0 for picking
//!     along a positive ray from an eyepoint)
//! @param [in,out] pickData growing array of picks.
//! @param [in] parentId caller's id for this solid.  Will be placed
//!               in each pickData entry has parentId.
GEOMDLLIMPEXP void AddRayIntersections
    (
    bvector<SolidLocationDetail> &pickData,
    DRay3dCR ray,
    int parentId,
    double minParameter
    ) const;

//! Compute intersections with curves and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void AddCurveIntersections
    (
    CurveVectorCR curves,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const;

//! Compute intersections with a single curve and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void AddCurveIntersections
    (
    ICurvePrimitiveCR curves,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const;


//! @param [in] spacePoint search for point close to here and on the solid surface
//! @param [out] pickDetail closest point data.
//! @return false if not supported.
GEOMDLLIMPEXP bool ClosestPoint
    (
    DPoint3dCR spacePoint,
    SolidLocationDetail &pickDetail
    ) const;

//! convert u,v fraction on specified face to xyz and derivatives.
//! @param [in] faceIndices face selection indices
//! @param [in] uFraction fractional coordinate in u direction.
//! @param [in] vFraction fractional coordinate in v direction.
//! @param [out] xyz point on face.
//! @param [out] dXdu derivative of face point wrt uFraction.
//! @param [out] dXdv derivative of face point wrt vFraction.
GEOMDLLIMPEXP bool TryUVFractionToXYZ
    (
    SolidLocationDetail::FaceIndices const &faceIndices, 
    double uFraction,
    double vFraction,
    DPoint3dR xyz,
    DVec3dR dXdu,
    DVec3dR dXdv
    ) const;

//! Return a single face of the solid primitive
//! @param indices integer selectors for the face.
GEOMDLLIMPEXP IGeometryPtr GetFace (SolidLocationDetail::FaceIndices const &indices) const;

//! Fill a list of all possible face indices.
//! @param [out] indices array of selectors for the faces.
GEOMDLLIMPEXP void GetFaceIndices (bvector<SolidLocationDetail::FaceIndices> &indices) const;

//! Copy a section curve at constant v of a single face to a single ICurvePrimitive.
//! @param [in] indices selects face
//! @param [in] fraction v fraction for section curve
GEOMDLLIMPEXP ICurvePrimitivePtr GetConstantVSection (SolidLocationDetail::FaceIndices const & indices, double fraction) const;

//! Copy a section at constant u of a single face to a single ICurvePrimitive.
//! @param [in] indices selects face
//! @param [in] fraction u fraction for section curve
GEOMDLLIMPEXP ICurvePrimitivePtr GetConstantUSection (SolidLocationDetail::FaceIndices const & indices, double fraction) const;

//! Compute the lengths of the longest u and v direction sections of a single face of a solid primitive.
//! @param [in] indices selects face.
//! @param [out] uvLength sizes in u, v directions.
GEOMDLLIMPEXP bool TryGetMaxUVLength (SolidLocationDetail::FaceIndices const & indices, DVec2dR uvLength) const;
//! Copy to a new (allocated) solid primitive.
GEOMDLLIMPEXP ISolidPrimitivePtr Clone () const;
//! Transform in place.
GEOMDLLIMPEXP bool TransformInPlace (TransformCR transform);

//! Return the various integrated products for area moment calculations.   The primitive is treated as a thin shell.
//! @param [out] localToWorld transform from preferred system where the products are most easily computed to world.
//! @param [out] localProducts integrated [xx xy xz x; xy yy yz y; xz yz zz z; x y z 1] dA
//! @return false if unable to compute.
GEOMDLLIMPEXP bool ComputeSecondMomentAreaProducts (TransformR localToWorld, DMatrix4dR localProducts) const;

//! Return the various integrated products for moment calculations.  The primitive is treated as a volume
//! @param [out] localToWorld transform from preferred system where the products are most easily computed to world.
//! @param [out] localProducts integrated [xx xy xz x; xy yy yz y; xz yz zz z; x y z 1] dA
//! @return false if unable to compute.
GEOMDLLIMPEXP bool ComputeSecondMomentVolumeProducts (TransformR localToWorld, DMatrix4dR localProducts) const;

//! Return curves which are silhoutte curves OTHER than hard edges
//! @return return false if not implemented.   return true if implemented -- but curves may still be empty.
GEOMDLLIMPEXP bool SilhouetteCurves(DPoint4dCR eyePoint, CurveVectorPtr &curves) const;
//! Return all intersection points of a curve with the pipe body
//! Returned data is the detailed local coordinates, with additional data to relate it back to world.
GEOMDLLIMPEXP void IntersectCurveLocal
(
ICurvePrimitiveCR curve,               //!< [in] curve to intersect with instance cone.
bvector<double> &curveFractions,  //!< [out] intersection fractions along the curve
bvector<DPoint3d> &normalizedConePoints,    //!< [out] intersection points, in local coordinates
TransformR localToWorld,    //!< [out] local to world transformation
TransformR worldToLocal,    //!< [out] world to local transformation
double &rMajor,            //!< [out] major radius
double &rMinor,            //!< [out] minor radius
double &sweepRadians,       //!< [out] pipe sweep
bool boundedConeZ           //!< [in] if true, skip intersections outside the cone caps.
) const;                              

//! set point, uv coordinates, and uv derivatives vectors 
static GEOMDLLIMPEXP void SetDetailCoordinatesFromLocalPipeCoordinates
(
SolidLocationDetail &detail,//!< [in,out] detail to update
DPoint3dCR  localuvw,       //!< [in] coordinates in local fraction space.
TransformCR localToWorld,   //!< [in] transform to world coordinates
double &rMajor,            //!< [in] major radius
double &rMinor,             //!< [in] minor radius
double &sweepRadians        //!< [in] sweep angle
);
//! Return the (constant !!) sign for the vector90 direction of the minor ellipse.
static GEOMDLLIMPEXP double GetVector90Sign ();
//! Return the (constant !!) flag for the vector90 direction of the minor ellipse.  (Equivalent to GetVector90Sign () < 0.0)
static GEOMDLLIMPEXP bool GetReverseVector90 ();
};


//! A DgnConeDetail represents a (frustum of a) cone.
struct DgnConeDetail : ZeroInit<DgnConeDetail>
{
DPoint3d    m_centerA;  // Center of base circle 
DPoint3d    m_centerB;  // Center of top circle 
DVec3d      m_vector0;  // 0 degree vector of base circle 
DVec3d      m_vector90; // 0 degree vector of base circle 
double      m_radiusA;  // radius at base (centerA) 
double      m_radiusB;  // radius at top (centerB) 
bool        m_capped;   // true if end cap is enabled 

//! Detail consructor with complete field list as parameters ...
//! @param [in] centerA Center of base circle
//! @param [in] centerB Center of top circle
//! @param [in] radiusA radius at base (centerA)
//! @param [in] radiusB radius at top (centerB)
//! @param [in] capped true if end cap is enabled
GEOMDLLIMPEXP DgnConeDetail 
    (
    DPoint3dCR  centerA,
    DPoint3dCR  centerB,
    double      radiusA,
    double      radiusB,
    bool        capped
    );

//! Detail consructor with complete field list as parameters ...
//! @param [in] centerA Center of base circle
//! @param [in] centerB Center of top circle
//! @param [in] axes x and y columns give base, top circle parameterization.
//! @param [in] radiusA radius at base (centerA)
//! @param [in] radiusB radius at top (centerB)
//! @param [in] capped true if end cap is enabled
GEOMDLLIMPEXP DgnConeDetail 
    (
    DPoint3dCR      centerA,
    DPoint3dCR      centerB,
    RotMatrixCR     axes,
    double          radiusA,
    double          radiusB,
    bool            capped
    );

//! Detail consructor with complete field list as parameters ...
//! @param [in] centerA Center of base circle
//! @param [in] centerB Center of top circle
//! @param [in] vectorX
//! @param [in] vectorY
//! @param [in] radiusA radius at base (centerA)
//! @param [in] radiusB radius at top (centerB)
//! @param [in] capped true if end cap is enabled
GEOMDLLIMPEXP DgnConeDetail 
    (
    DPoint3dCR      centerA,
    DPoint3dCR      centerB,
    DVec3dCR        vectorX,
    DVec3dCR        vectorY,
    double          radiusA,
    double          radiusB,
    bool            capped
    );


//! Default value constructor.
GEOMDLLIMPEXP DgnConeDetail ();

//! Return range.
GEOMDLLIMPEXP bool GetRange (DRange3dR range) const;
//! Return range when transformed
GEOMDLLIMPEXP bool GetRange (DRange3dR range, TransformCR transform) const;
//! Return coordinate system with
//!   1) XY plane of base circle, origin at center.
//!   2) Z perpendicular
GEOMDLLIMPEXP bool TryGetConstructiveFrame (TransformR localToWorld, TransformR worldToLocal) const;

//! Test for same type and structure (but no coordinate comparison)
 GEOMDLLIMPEXP bool IsSameStructure (ISolidPrimitiveCR other) const;
//! Test for same type, structure and coordinates.
GEOMDLLIMPEXP bool IsSameStructureAndGeometry (ISolidPrimitiveCR other, double tolerance) const;
//! Test if this is a closed volume.
GEOMDLLIMPEXP bool IsClosedVolume () const;
//! Return +1 or -1 that matches the volume sign after integration over the parameter space.
GEOMDLLIMPEXP double ParameterizationSign () const;


//! Try to set up a nonsingular coordinate frame.
//! Returns false if centerB is in base plane !!!!
GEOMDLLIMPEXP bool GetTransforms (
//! Frame with origin at base, z=1 at top.   This frame can be skewed.
TransformR localToWorld,
//! Inverse frame.  Undefined for false return.
TransformR worldToLocal,
//! radius at z=0 of local frame.
double &radiusA,
//! radius at z=1 of local frame.
double &radiusB,
//! If false, x and y have legngth 1, z has length along the axis, and the radii are physical bottom and top
//! If true, x and y have length of the largest radius and z has length along the axis, and the radii are fractions of the larger radius.
bool   fractionalRadii = false
) const;

//! Return the ellipse cross section at a fraction along the rotation axis.
//! @param [in] fraction fractional position alont the z axis.
//! @param [out] ellipse the section ellipse.
GEOMDLLIMPEXP bool FractionToSection (double fraction, DEllipse3dR ellipse) const;

//! Return the rule line section at a fraction around the circular sections.
//! @param [in] fraction fractional position around the cone.
//! @param [out] segment rule line
GEOMDLLIMPEXP bool FractionToRule (double fraction, DSegment3dR segment) const;

//! Return the rule lines which are silhouettes as viewed.
//! @param [out] segmentA first silhouette line
//! @param [out] segmentB second silhouette line
//! @param [in] viewToLocal matrix that positions the cone for viewing along the z axis.
GEOMDLLIMPEXP bool GetSilhouettes (DSegment3dR segmentA, DSegment3dR segmentB, DMatrix4dCR viewToLocal) const;

//! Test if caps are active and the indicated cap has nonzero radius.
GEOMDLLIMPEXP bool IsRealCap (int select01) const;

//! Return true (with supporting data) if 
//! the cone is circular.
GEOMDLLIMPEXP bool IsCircular (
    DPoint3dR centerA,
    DPoint3dR centerB,
    RotMatrixR rotMatrix,
    double &radiusA,
    double &radiusB,
    bool &capped
    ) const;
//! Return true (with supporting data) if the cone is a (constant radius) cylinder.
GEOMDLLIMPEXP bool IsCylinder
(
DPoint3dR centerA,
DPoint3dR centerB,
double &radius,
bool &capped
) const;

//! Compute intersections with a ray and add to the pickData.
//! @param [in] ray origin and direction
//! @param [in] minParameter smallest parameter of pick.  (e.g. 0 for picking
//!     along a positive ray from an eyepoint)
//! @param [in,out] pickData growing array of picks.
//! @param [in] parentId caller's id for this solid.  Will be placed
//!               in each pickData entry has parentId.
GEOMDLLIMPEXP void AddRayIntersections
    (
    bvector<SolidLocationDetail> &pickData,
    DRay3dCR ray,
    int parentId,
    double minParameter
    ) const;

//! Compute intersections with curves and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void AddCurveIntersections
    (
    CurveVectorCR curves,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const;

//! Compute intersections with a single curve and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void AddCurveIntersections
    (
    ICurvePrimitiveCR curves,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const;

//! @param [in] spacePoint search for point close to here and on the solid surface
//! @param [out] pickDetail closest point data.
//! @return false if not supported.
GEOMDLLIMPEXP bool ClosestPoint
    (
    DPoint3dCR spacePoint,
    SolidLocationDetail &pickDetail
    ) const;
    
//! convert u,v fraction on specified face to xyz and derivatives.
//! @param [in] faceIndices face selection indices
//! @param [in] uFraction fractional coordinate in u direction.
//! @param [in] vFraction fractional coordinate in v direction.
//! @param [out] xyz point on face.
//! @param [out] dXdu derivative of face point wrt uFraction.
//! @param [out] dXdv derivative of face point wrt vFraction.
GEOMDLLIMPEXP bool TryUVFractionToXYZ
    (
    SolidLocationDetail::FaceIndices const &faceIndices,
    double uFraction,
    double vFraction,
    DPoint3dR xyz,
    DVec3dR dXdu,
    DVec3dR dXdv
    ) const;

//! Return a single face of the solid primitive
//! @param indices integer selectors for the face.
GEOMDLLIMPEXP IGeometryPtr GetFace (SolidLocationDetail::FaceIndices const &indices) const;

//! Fill a list of all possible face indices.
//! @param [out] indices array of selectors for the faces.
GEOMDLLIMPEXP void GetFaceIndices (bvector<SolidLocationDetail::FaceIndices> &indices) const;

//! Copy a section curve at constant v of a single face to a single ICurvePrimitive.
//! @param [in] indices selects face
//! @param [in] fraction v fraction for section curve
GEOMDLLIMPEXP ICurvePrimitivePtr GetConstantVSection (SolidLocationDetail::FaceIndices const & indices, double fraction) const;

//! Copy a section at constant u of a single face to a single ICurvePrimitive.
//! @param [in] indices selects face
//! @param [in] fraction u fraction for section curve
GEOMDLLIMPEXP ICurvePrimitivePtr GetConstantUSection (SolidLocationDetail::FaceIndices const & indices, double fraction) const;

//! Compute the lengths of the longest u and v direction sections of a single face of a solid primitive.
//! @param [in] indices selects face.
//! @param [in] uvLength sizes in u, v directions.
GEOMDLLIMPEXP bool TryGetMaxUVLength (SolidLocationDetail::FaceIndices const & indices, DVec2dR uvLength) const;
//! Copy to a new (allocated) solid primitive.
GEOMDLLIMPEXP ISolidPrimitivePtr Clone () const;
//! Transform in place.
GEOMDLLIMPEXP bool TransformInPlace (TransformCR transform);
//! Return the various integrated products for area moment calculations.   The primitive is treated as a thin shell.
//! @param [out] localToWorld transform from preferred system where the products are most easily computed to world.
//! @param [out] localProducts integrated [xx xy xz x; xy yy yz y; xz yz zz z; x y z 1] dA
//! @return false if unable to compute.
GEOMDLLIMPEXP bool ComputeSecondMomentAreaProducts (TransformR localToWorld, DMatrix4dR localProducts) const;

//! Return the various integrated products for moment calculations.  The primitive is treated as a volume
//! @param [out] localToWorld transform from preferred system where the products are most easily computed to world.
//! @param [out] localProducts integrated [xx xy xz x; xy yy yz y; xz yz zz z; x y z 1] dA
//! @return false if unable to compute.
GEOMDLLIMPEXP bool ComputeSecondMomentVolumeProducts (TransformR localToWorld, DMatrix4dR localProducts) const;
                              
//! Return curves which are silhoutte curves OTHER than hard edges
//! @return return false if not implemented.   return true if implemented -- but curves may still be empty.
GEOMDLLIMPEXP bool SilhouetteCurves(DPoint4dCR eyePoint, CurveVectorPtr &curves) const;

//! Return all intersection points of an arc with the cone
//! Returned data is the detailed local coordinates, with additional data to relate it back to world.
GEOMDLLIMPEXP void IntersectBoundedArc
(
DEllipse3dCR arc,               //!< [in] arc to intersect with instance cone.
bvector<double> &arcFractions,  //!< [out] intersection fractions along the arc.
bvector<DPoint3d> &normalizedConePoints,    //!< [out] intersection points, in local coordinates
TransformR localToWorld,    //!< [out] local to world transformation
TransformR worldToLocal,    //!< [out] world to local transformation
double &radius0,            //!< [out] local arc radius at z=0
double &radius1,            //!< [out] local arc radius at z=1
bool boundedConeZ           //!< [in] if true, skip intersections outside the cone caps.
) const;

//! Return all intersection points of a curve with a cone.
//! Returned data is the detailed local coordinates, with additional data to relate it back to world.
GEOMDLLIMPEXP void IntersectCurveLocal
(
ICurvePrimitiveCR curve,               //!< [in] curve to intersect with instance cone.
bvector<double> &curveFractions,  //!< [out] intersection fractions along the curve
bvector<DPoint3d> &normalizedConePoints,    //!< [out] intersection points, in local coordinates
TransformR localToWorld,    //!< [out] local to world transformation
TransformR worldToLocal,    //!< [out] world to local transformation
double &radius0,            //!< [out] local arc radius at z=0
double &radius1,            //!< [out] local arc radius at z=1
bool boundedConeZ           //!< [in] if true, skip intersections outside the cone caps.
) const;

//! set point, uv coordinates, and uv derivatives vectors 
static GEOMDLLIMPEXP void SetDetailCoordinatesFromFractionalizedConeCoordinates
(
SolidLocationDetail &detail,//!< [in,out] detail to update
DPoint3dCR  localuvw,       //!< [in] coordinates in local fraction space.
TransformCR localToWorld,   //!< [in] transform to world coordinates
double      r0,             //!< [in] cone radius at w=0
double      r1             //!< [in] cone radius at w=1
);
};


//! A DgnBoxDetail represents a boxlike surface with two paralell rectangular faces (bottom and top) and ruled side surfaces.
struct DgnBoxDetail : ZeroInit<DgnBoxDetail>
{
DPoint3d m_baseOrigin; // origin of base rectangle 
DPoint3d m_topOrigin; // origin of Top rectangle 
DVec3d m_vectorX; // X vector of base plane 
DVec3d m_vectorY; // Y vector of base plane 
double m_baseX; // x size at base 
double m_baseY; // y size at base 
double m_topX; // X size at top 
double m_topY; // y size at top 
bool m_capped; // true if end cap is enabled 

//! Detail constructor with complete field list as parameters ...
//! @param [in] baseOrigin origin of base rectangle
//! @param [in] topOrigin origin of Top rectangle
//! @param [in] vectorX X vector of base plane
//! @param [in] vectorY Y vector of base plane
//! @param [in] baseX x size at base
//! @param [in] baseY y size at base
//! @param [in] topX X size at top
//! @param [in] topY y size at top
//! @param [in] capped true if closed top and bottom.
GEOMDLLIMPEXP DgnBoxDetail (
    DPoint3dCR baseOrigin,
    DPoint3dCR topOrigin,
    DVec3dCR vectorX,
    DVec3dCR vectorY,
    double baseX,
    double baseY,
    double topX,
    double topY,
    bool capped);

//! Default value constructor.
GEOMDLLIMPEXP DgnBoxDetail ();

//! Initialize box detail fields specifying top/base centers instead of origins...
//! @param [in] baseCenter center of base rectangle
//! @param [in] topCenter center of Top rectangle
//! @param [in] vectorX X vector of base plane
//! @param [in] vectorY Y vector of base plane
//! @param [in] baseX x size at base
//! @param [in] baseY y size at base
//! @param [in] topX X size at top
//! @param [in] topY y size at top
//! @param [in] capped true if closed top and bottom.
static GEOMDLLIMPEXP DgnBoxDetail InitFromCenters (DPoint3dCR baseCenter, DPoint3dCR topCenter, DVec3dCR vectorX, DVec3dCR vectorY, double baseX, double baseY, double topX, double topY, bool capped);

//! Initialize box detail fields from center and size.
//! @param [in] center center of box in XYZ
//! @param [in] size total range in XYZ
//! @param [in] capped true if closed top and bottom.
static GEOMDLLIMPEXP DgnBoxDetail InitFromCenterAndSize (DPoint3dCR center, DPoint3dCR size, bool capped);

//! Return (nonuniform) placement and rectangle sizes.   (ax,ay) rectangle is on z=0.  (bx,by) is on z=1;
// @param [out] localToWorld placement transform.   Base rectangel is from origin to (ax,ay,0).  Top is from (0,0,1) to (ax,ay,1)
// @param [out] ax base rectangle x size.
// @param [out] ay top rectangle y size.
// @param [out] bx top rectangle x size.
// @param [out] by top rectangle y size.
GEOMDLLIMPEXP void GetNonUniformTransform (TransformR localToWorld, double &ax, double &ay, double &bx, double &by) const;

//! Return range.
GEOMDLLIMPEXP bool GetRange (DRange3dR range) const;
//! Return range when transformed
GEOMDLLIMPEXP bool GetRange (DRange3dR range, TransformCR transform) const;
//! Return coordinate system with
//!   1) XY in box base plane, origin at nominal lower left.
//!   2) Z perpendicular
GEOMDLLIMPEXP bool TryGetConstructiveFrame (TransformR localToWorld, TransformR worldToLocal) const;

//! Test for same type and structure (but no coordinate comparison)
 GEOMDLLIMPEXP bool IsSameStructure (ISolidPrimitiveCR other) const;
//! Test for same type, structure and coordinates.
GEOMDLLIMPEXP bool IsSameStructureAndGeometry (ISolidPrimitiveCR other, double tolerance) const;
//! Test if this is a closed volume.
GEOMDLLIMPEXP bool IsClosedVolume () const;
//! Return +1 or -1 that matches the volume sign after integration over the parameter space.
GEOMDLLIMPEXP double ParameterizationSign () const;


//! Return 8 corners of the box.  x varies fastest, then y then z.
//! @param [out] corners 8 corner coordinates.
GEOMDLLIMPEXP void GetCorners (bvector<DPoint3d> &corners) const;

//! Return 8 corners of the box.  x varies fastest, then y then z.
//! @param [out] corners 8 corner coordinates.   Caller must allocate to size 8 or mre.
GEOMDLLIMPEXP void GetCorners (DPoint3dP corners) const;

//! Test if the box is an orthogonal box.
//! @param [out] centeredFrame coordinate axes (normalized) at the center of the box.
//! @param [out] diagonal diagonal vector.
GEOMDLLIMPEXP void IsCenteredBox (Transform centeredFrame, DVec3dR diagonal) const;


//! Compute intersections with a ray and add to the pickData.
//! @param [in] ray origin and direction
//! @param [in] minParameter smallest parameter of pick.  (e.g. 0 for picking
//!     along a positive ray from an eyepoint)
//! @param [in,out] pickData growing array of picks.
//! @param [in] parentId caller's id for this solid.  Will be placed
//!               in each pickData entry has parentId.
GEOMDLLIMPEXP void AddRayIntersections
    (
    bvector<SolidLocationDetail> &pickData,
    DRay3dCR ray,
    int parentId,
    double minParameter
    ) const;

//! Compute intersections with curves and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void AddCurveIntersections
    (
    CurveVectorCR curves,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const;

//! Compute intersections with a single curve and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void AddCurveIntersections
    (
    ICurvePrimitiveCR curves,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const;

//! @param [in] spacePoint search for point close to here and on the solid surface
//! @param [out] pickDetail closest point data.
//! @return false if not supported.
GEOMDLLIMPEXP bool ClosestPoint
    (
    DPoint3dCR spacePoint,
    SolidLocationDetail &pickDetail
    ) const;
    
//! convert u,v fraction on specified face to xyz and derivatives.
//! @param [in] faceIndices face selection indices
//! @param [in] uFraction fractional coordinate in u direction.
//! @param [in] vFraction fractional coordinate in v direction.
//! @param [out] xyz point on face.
//! @param [out] dXdu derivative of face point wrt uFraction.
//! @param [out] dXdv derivative of face point wrt vFraction.
GEOMDLLIMPEXP bool TryUVFractionToXYZ
    (
    SolidLocationDetail::FaceIndices const &faceIndices,
    double uFraction,
    double vFraction,
    DPoint3dR xyz,
    DVec3dR dXdu,
    DVec3dR dXdv
    ) const;
//! Return a single face of the solid primitive
//! @param indices integer selectors for the face.
GEOMDLLIMPEXP IGeometryPtr GetFace (SolidLocationDetail::FaceIndices const &indices) const;

//! Fill a list of all possible face indices.
//! @param [out] indices array of selectors for the faces.
GEOMDLLIMPEXP void GetFaceIndices (bvector<SolidLocationDetail::FaceIndices> &indices) const;

//! Copy a section curve at constant v of a single face to a single ICurvePrimitive.
//! @param [in] indices selects face
//! @param [in] fraction v fraction for section curve
GEOMDLLIMPEXP ICurvePrimitivePtr GetConstantVSection (SolidLocationDetail::FaceIndices const & indices, double fraction) const;

//! Copy a section at constant u of a single face to a single ICurvePrimitive.
//! @param [in] indices selects face
//! @param [in] fraction u fraction for section curve
GEOMDLLIMPEXP ICurvePrimitivePtr GetConstantUSection (SolidLocationDetail::FaceIndices const & indices, double fraction) const;

//! Compute the lengths of the longest u and v direction sections of a single face of a solid primitive.
//! @param [in] indices selects face.
//! @param [in] uvLength sizes in u, v directions.
GEOMDLLIMPEXP bool TryGetMaxUVLength (SolidLocationDetail::FaceIndices const & indices, DVec2dR uvLength) const;
//! Copy to a new (allocated) solid primitive.
GEOMDLLIMPEXP ISolidPrimitivePtr Clone () const;
//! Transform in place.
GEOMDLLIMPEXP bool TransformInPlace (TransformCR transform);

//! Return the various integrated products for area moment calculations.   The primitive is treated as a thin shell.
//! @param [out] localToWorld transform from preferred system where the products are most easily computed to world.
//! @param [out] localProducts integrated [xx xy xz x; xy yy yz y; xz yz zz z; x y z 1] dA
//! @return false if unable to compute.
GEOMDLLIMPEXP bool ComputeSecondMomentAreaProducts (TransformR localToWorld, DMatrix4dR localProducts) const;

//! Return the various integrated products for moment calculations.  The primitive is treated as a volume
//! @param [out] localToWorld transform from preferred system where the products are most easily computed to world.
//! @param [out] localProducts integrated [xx xy xz x; xy yy yz y; xz yz zz z; x y z 1] dA
//! @return false if unable to compute.
GEOMDLLIMPEXP bool ComputeSecondMomentVolumeProducts (TransformR localToWorld, DMatrix4dR localProducts) const;

//! Return curves which are silhoutte curves OTHER than hard edges
//! @return return false if not implemented.   return true if implemented -- but curves may still be empty.
GEOMDLLIMPEXP bool SilhouetteCurves(DPoint4dCR eyePoint, CurveVectorPtr &curves) const;

//! Test if the DgnBox is really a box (aka slab).  Return orientation and size data, using caller-specifed fractional coordinates to indicate position of origin in reference system.
//! @param [out] origin local coordinates origin
//! @param [out] unitAxes transform (with orthogonal axes)
//! @param [out] localDiagonal the box edge lengths.
//! @param [in] originXFraction fractional position of the unitAxes origin along the x edge.
//! @param [in] originYFraction fractional position of the unitAxes origin along the y edge.
//! @param [in] originZFraction fractional position of the unitAxes origin along the z edge.
GEOMDLLIMPEXP bool IsBlock (DPoint3dR origin, RotMatrixR unitAxes, DVec3dR localDiagonal, double originXFraction, double originYFraction, double originZFraction ) const;
};

//! A DgnSphereDetail represents an ellipsoid, possibly truncated on two planes parallel to the equator.
struct DgnSphereDetail : ZeroInit<DgnSphereDetail>
{
Transform m_localToWorld;   // origin is sphere center.  columns x,y to equator at 0 and 90 degrees latitude.  column z is to north pole.
double m_startLatitude; // latitude for truncation plane parallel to the equator 
double m_latitudeSweep; // latitude difference from start truncation plane to end truncation plane 
bool m_capped;  // cap surface present (e.g. for partial latitude sweep)
//! Detail consructor with complete field list as parameters ...
//! @param [in] center Sphere center
//! @param [in] vectorX X vector of base plane
//! @param [in] vectorZ Vector towards north pole
//! @param [in] radiusXY x and y radius (equator circle)
//! @param [in] radiusZ north pole radius.
//! @param [in] startLatitude latitude for truncation plane parallel to the equator
//! @param [in] latitudeSweep latitude difference from start truncation plane to end truncation plane
//! @param [in] capped cap flag, applicable for partial latitude range
GEOMDLLIMPEXP DgnSphereDetail 
    (
    DPoint3dCR      center,
    DVec3dCR        vectorX,
    DVec3dCR        vectorZ,
    double          radiusXY,
    double          radiusZ,
    double          startLatitude,
    double          latitudeSweep,
    bool            capped
    );

//! Detail consructor for complete true sphere
//! @param [in] center Sphere center
//! @param [in] axes x,y,z axes
//! @param [in] radius radius
GEOMDLLIMPEXP DgnSphereDetail 
    (
    DPoint3dCR      center,
    RotMatrixCR     axes,
    double          radius
    );

//! Detail constructor for complete true sphere
//! @param [in] center Sphere center
//! @param [in] radius radius
GEOMDLLIMPEXP DgnSphereDetail (DPoint3dCR center, double radius);

//! Default value constructor.
GEOMDLLIMPEXP DgnSphereDetail ();

//! Return range.
GEOMDLLIMPEXP bool GetRange (DRange3dR range) const;
//! Return range when transformed
GEOMDLLIMPEXP bool GetRange (DRange3dR range, TransformCR transform) const;
//! Return coordinate system with
//!   1) XY plane in equatorial plane, origin at sphere center.
//!   2) Z perpendicular
GEOMDLLIMPEXP bool TryGetConstructiveFrame (TransformR localToWorld, TransformR worldToLocal) const;

//! Test for same type and structure (but no coordinate comparison)
 GEOMDLLIMPEXP bool IsSameStructure (ISolidPrimitiveCR other) const;
//! Test for same type, structure and coordinates.
GEOMDLLIMPEXP bool IsSameStructureAndGeometry (ISolidPrimitiveCR other, double tolerance) const;
//! Test if this is a closed volume.
GEOMDLLIMPEXP bool IsClosedVolume () const;
//! Return +1 or -1 that matches the volume sign after integration over the parameter space.
GEOMDLLIMPEXP double ParameterizationSign () const;


//! Return true (with supporting data) iff this is a (complete) true sphere.
GEOMDLLIMPEXP bool IsTrueSphere
(
DPoint3dR center,
RotMatrixR axes,
double &radius
) const;

//! Return true if the tracing the parameterization produces reverse (inward facing) orientation
GEOMDLLIMPEXP bool IsReverseOrientation ();

//! Return true (with supporting local coordinate frame) iff this is a rotation around the Z axis.
GEOMDLLIMPEXP bool IsTrueRotationAroundZ
(
DPoint3dR center,
DVec3dR unitX,
DVec3dR unitY,
DVec3dR unitZ,
double     &equatorRadius,
double     &poleRadius
) const;

//! @param [out] latitude0 latitude at start of sweep.
//! @param [out] latitude1 latitude at end of sweep.
//! @param [out] z0 z coordinate at start of sweep.
//! @param [out] z1 z coordinate at end of sweep.
//! @param [in] forceSweepNorth true to exchange if necessary to make sweep south to north.
//! @returns false if no sweep limits (full sphere)
GEOMDLLIMPEXP bool GetSweepLimits (double &latitude0, double &latitude1, double &z0, double &z1, bool forceSweepNorth = false) const;


//! Test if caps are active and the indicated cap is not at a pole
GEOMDLLIMPEXP bool IsRealCap (int select01) const;

//! Compute the v fraction of latitude
GEOMDLLIMPEXP double LatitudeToVFraction (double latitude) const;

//! Compute the u fraction of longitude
GEOMDLLIMPEXP double LongitudeToUFraction (double longitude) const;
//! Compute the latitude at v fraction.
GEOMDLLIMPEXP double VFractionToLatitude (double v) const;

//! Return transforms for unit-axis system.
GEOMDLLIMPEXP bool GetTransforms (TransformR localToWorld, TransformR worldToLocal) const;

//! Return (nonuniform) transforms for normalized system where sphere radius is 1.
GEOMDLLIMPEXP bool GetNonUniformTransforms (TransformR localToWorld, TransformR worldToLocal) const;

//! @param center OUT center of rotation.
//! @param axis OUT axis of rotation.
//! @param sweepRadians OUT angle of rotation.
GEOMDLLIMPEXP bool TryGetRotationAxis (DPoint3dR center, DVec3dR axis, double &sweepRadians) const;

//! Return the ellipse on the meridian at fractional position.
GEOMDLLIMPEXP DEllipse3d UFractionToVSectionDEllipse3d (double fraction) const;

//! Return the ellipse on the parallel at fractional position.
GEOMDLLIMPEXP DEllipse3d VFractionToUSectionDEllipse3d (double fraction) const;

//! Compute intersections with a ray and add to the pickData.
//! @param [in] ray origin and direction
//! @param [in] minParameter smallest parameter of pick.  (e.g. 0 for picking
//!     along a positive ray from an eyepoint)
//! @param [in,out] pickData growing array of picks.
//! @param [in] parentId caller's id for this solid.  Will be placed
//!               in each pickData entry has parentId.
GEOMDLLIMPEXP void AddRayIntersections
    (
    bvector<SolidLocationDetail> &pickData,
    DRay3dCR ray,
    int parentId,
    double minParameter
    ) const;
    
//! Compute intersections with curves and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void AddCurveIntersections
    (
    CurveVectorCR curves,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const;

//! Compute intersections with a single curve and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void AddCurveIntersections
    (
    ICurvePrimitiveCR curves,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const;

//! @param [in] spacePoint search for point close to here and on the solid surface
//! @param [out] pickDetail closest point data.
//! @return false if not supported.
GEOMDLLIMPEXP bool ClosestPoint
    (
    DPoint3dCR spacePoint,
    SolidLocationDetail &pickDetail
    ) const;
    
//! convert u,v fraction on specified face to xyz and derivatives.
//! @param [in] faceIndices face selection indices
//! @param [in] uFraction fractional coordinate in u direction.
//! @param [in] vFraction fractional coordinate in v direction.
//! @param [out] xyz point on face.
//! @param [out] dXdu derivative of face point wrt uFraction.
//! @param [out] dXdv derivative of face point wrt vFraction.
GEOMDLLIMPEXP bool TryUVFractionToXYZ
    (
    SolidLocationDetail::FaceIndices const &faceIndices,
    double uFraction,
    double vFraction,
    DPoint3dR xyz,
    DVec3dR dXdu,
    DVec3dR dXdv
    ) const;
//! Return a single face of the solid primitive
//! @param indices integer selectors for the face.
GEOMDLLIMPEXP IGeometryPtr GetFace (SolidLocationDetail::FaceIndices const &indices) const;

//! Fill a list of all possible face indices.
//! @param [out] indices array of selectors for the faces.
GEOMDLLIMPEXP void GetFaceIndices (bvector<SolidLocationDetail::FaceIndices> &indices) const;

//! Copy a section curve at constant v of a single face to a single ICurvePrimitive.
//! @param [in] indices selects face
//! @param [in] fraction v fraction for section curve
GEOMDLLIMPEXP ICurvePrimitivePtr GetConstantVSection (SolidLocationDetail::FaceIndices const & indices, double fraction) const;

//! Copy a section at constant u of a single face to a single ICurvePrimitive.
//! @param [in] indices selects face
//! @param [in] fraction u fraction for section curve
GEOMDLLIMPEXP ICurvePrimitivePtr GetConstantUSection (SolidLocationDetail::FaceIndices const & indices, double fraction) const;

//! Compute the lengths of the longest u and v direction sections of a single face of a solid primitive.
//! @param [in] indices selects face.
//! @param [in] uvLength sizes in u, v directions.
GEOMDLLIMPEXP bool TryGetMaxUVLength (SolidLocationDetail::FaceIndices const & indices, DVec2dR uvLength) const;
//! Copy to a new (allocated) solid primitive.
GEOMDLLIMPEXP ISolidPrimitivePtr Clone () const;
//! Transform in place.
GEOMDLLIMPEXP bool TransformInPlace (TransformCR transform);

//! Return the various integrated products for area moment calculations.   The primitive is treated as a thin shell.
//! @param [out] localToWorld transform from preferred system where the products are most easily computed to world.
//! @param [out] localProducts integrated [xx xy xz x; xy yy yz y; xz yz zz z; x y z 1] dA
//! @return false if unable to compute.
GEOMDLLIMPEXP bool ComputeSecondMomentAreaProducts (TransformR localToWorld, DMatrix4dR localProducts) const;

//! Return the various integrated products for moment calculations.  The primitive is treated as a volume
//! @param [out] localToWorld transform from preferred system where the products are most easily computed to world.
//! @param [out] localProducts integrated [xx xy xz x; xy yy yz y; xz yz zz z; x y z 1] dA
//! @return false if unable to compute.
GEOMDLLIMPEXP bool ComputeSecondMomentVolumeProducts (TransformR localToWorld, DMatrix4dR localProducts) const;

//! Return curves which are silhoutte curves OTHER than hard edges
//! @return return false if not implemented.   return true if implemented -- but curves may still be empty.
GEOMDLLIMPEXP bool SilhouetteCurves(DPoint4dCR eyePoint, CurveVectorPtr &curves) const;

//! Return all intersection points of an (unbounded) arc with the sphere.
//! Returned data is the detailed local coordinates, with additional data to relate it back to world.
GEOMDLLIMPEXP void IntersectBoundedArc
(
DEllipse3dCR arc,               //!< [in] arc to intersect with instance cone.
bvector<double> &arcFractions,  //!< [out] intersection fractions along the arc.
bvector<DPoint3d> &normalizedConePoints,    //!< [out] intersection points, in local coordinates
TransformR localToWorld,    //!< [out] local to world transformation
TransformR worldToLocal,    //!< [out] world to local transformation
bool boundedZ           //!< [in] if true, skip intersections outside the latitude sweep
) const;


//! set point, uv coordinates, and uv derivatives vectors 
static GEOMDLLIMPEXP void SetDetailUVFromUnitSphereCoordinates
(
SolidLocationDetail &detail,//!< [in,out] detail to update
DPoint3dCR  localuvw,       //!< [in] coordinates in local fraction space.
TransformCR localToWorld,   //!< [in] transform to world coordinates
double startLatitude,       //!< [in] start latitude for partial sphere
double sweepLatitude        //!< [in] latitude sweep for partial sphere.
);
};


//! A DgnExtrusionDetail is a linear sweep of a base CurveVector.  All points on the base are swept by the same vector.
struct DgnExtrusionDetail : ZeroInit<DgnExtrusionDetail>
{
CurveVectorPtr m_baseCurve; // Curve to be swept. 
DVec3d m_extrusionVector; // Vector from base to target curve 
bool m_capped; // true if end cap is enabled 

//! Detail consructor with complete field list as parameters ...
//! @param [in] baseCurve Curve to be swept. This pointer is captured (not cloned) into the extrusion.
//! @param [in] extrusionVector Vector from base to target curve
//! @param [in] capped true if end cap is enabled
GEOMDLLIMPEXP DgnExtrusionDetail (
    CurveVectorPtr const &baseCurve,
    DVec3dCR extrusionVector,
    bool capped);


//! Default value constructor.
GEOMDLLIMPEXP DgnExtrusionDetail ();

//! Fractional profile curve
GEOMDLLIMPEXP CurveVectorPtr FractionToProfile (double fraction) const;

//! Return range.
GEOMDLLIMPEXP bool GetRange (DRange3dR range) const;
//! Return range when transformed
GEOMDLLIMPEXP bool GetRange (DRange3dR range, TransformCR transform) const;
//! Return coordinate system based on any frenet frame for base curve vector, with yz reversed if necessary to make
//!  z sense match exgtusion vector.
GEOMDLLIMPEXP bool TryGetConstructiveFrame (TransformR localToWorld, TransformR worldToLocal) const;

//! Return coordinate system with Z in the extrusion direction, and origin at start point, xy vectors from zVector.GetNormalizedTriad()
//! If this fails, return any coordinate frame with the extrusion vector Z up.
GEOMDLLIMPEXP bool TryGetZExtrusionFrame (TransformR localToWorld, TransformR worldToLocal) const;


//! Test for same type and structure (but no coordinate comparison)
 GEOMDLLIMPEXP bool IsSameStructure (ISolidPrimitiveCR other) const;
//! Test for same type, structure and coordinates.
GEOMDLLIMPEXP bool IsSameStructureAndGeometry (ISolidPrimitiveCR other, double tolerance) const;
//! Test if this is a closed volume.
GEOMDLLIMPEXP bool IsClosedVolume () const;


//! Compute intersections with a ray and add to the pickData.
//! @param [in] ray origin and direction
//! @param [in] minParameter smallest parameter of pick.  (e.g. 0 for picking
//!     along a positive ray from an eyepoint)
//! @param [in,out] pickData growing array of picks.
//! @param [in] parentId caller's id for this solid.  Will be placed
//!               in each pickData entry has parentId.
GEOMDLLIMPEXP void AddRayIntersections
    (
    bvector<SolidLocationDetail> &pickData,
    DRay3dCR ray,
    int parentId,
    double minParameter
    ) const;

//! Compute intersections with curves and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void AddCurveIntersections
    (
    CurveVectorCR curves,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const;

//! Compute intersections with a single curve and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void AddCurveIntersections
    (
    ICurvePrimitiveCR curves,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const;

//! @param [in] spacePoint search for point close to here and on the solid surface
//! @param [out] pickDetail closest point data.
//! @return false if not supported.
GEOMDLLIMPEXP bool ClosestPoint
    (
    DPoint3dCR spacePoint,
    SolidLocationDetail &pickDetail
    ) const;
    
//! convert u,v fraction on specified face to xyz and derivatives.
//! @param [in] faceIndices face selection indices
//! @param [in] uFraction fractional coordinate in u direction.
//! @param [in] vFraction fractional coordinate in v direction.
//! @param [out] xyz point on face.
//! @param [out] dXdu derivative of face point wrt uFraction.
//! @param [out] dXdv derivative of face point wrt vFraction.
GEOMDLLIMPEXP bool TryUVFractionToXYZ
    (
    SolidLocationDetail::FaceIndices const &faceIndices,
    double uFraction,
    double vFraction,
    DPoint3dR xyz,
    DVec3dR dXdu,
    DVec3dR dXdv
    ) const;

//! Compute coordinate frames aligned with the extrusion vector.
//! @param [out] localToWorld transform with orign at first point of base curve,
//!            z vector along extrusion direction, xy vectors arbitrary perpendiculars.
//! @param [out] worldToLocal inverse of local to world
GEOMDLLIMPEXP bool TryGetExtrusionFrame (TransformR localToWorld, TransformR worldToLocal) const;

//! Return a single face of the solid primitive
//! @param indices integer selectors for the face.
GEOMDLLIMPEXP IGeometryPtr GetFace (SolidLocationDetail::FaceIndices const &indices) const;

//! Fill a list of all possible face indices.
//! @param [out] indices array of selectors for the faces.
GEOMDLLIMPEXP void GetFaceIndices (bvector<SolidLocationDetail::FaceIndices> &indices) const;

//! Copy a section curve at constant v of a single face to a single ICurvePrimitive.
//! @param [in] indices selects face
//! @param [in] fraction v fraction for section curve
GEOMDLLIMPEXP ICurvePrimitivePtr GetConstantVSection (SolidLocationDetail::FaceIndices const & indices, double fraction) const;

//! Copy a section at constant u of a single face to a single ICurvePrimitive.
//! @param [in] indices selects face
//! @param [in] fraction u fraction for section curve
GEOMDLLIMPEXP ICurvePrimitivePtr GetConstantUSection (SolidLocationDetail::FaceIndices const & indices, double fraction) const;

//! Compute the lengths of the longest u and v direction sections of a single face of a solid primitive.
//! @param [in] indices selects face.
//! @param [in] uvLength sizes in u, v directions.
GEOMDLLIMPEXP bool TryGetMaxUVLength (SolidLocationDetail::FaceIndices const & indices, DVec2dR uvLength) const;
//! Copy to a new (allocated) solid primitive.
GEOMDLLIMPEXP ISolidPrimitivePtr Clone () const;
//! Transform in place.
GEOMDLLIMPEXP bool TransformInPlace (TransformCR transform);

//! Return the various integrated products for area moment calculations.   The primitive is treated as a thin shell.
//! @param [out] localToWorld transform from preferred system where the products are most easily computed to world.
//! @param [out] localProducts integrated [xx xy xz x; xy yy yz y; xz yz zz z; x y z 1] dA
//! @return false if unable to compute.
GEOMDLLIMPEXP bool ComputeSecondMomentAreaProducts (TransformR localToWorld, DMatrix4dR localProducts) const;

//! Return the various integrated products for moment calculations.  The primitive is treated as a volume
//! @param [out] localToWorld transform from preferred system where the products are most easily computed to world.
//! @param [out] localProducts integrated [xx xy xz x; xy yy yz y; xz yz zz z; x y z 1] dA
//! @return false if unable to compute.
GEOMDLLIMPEXP bool ComputeSecondMomentVolumeProducts (TransformR localToWorld, DMatrix4dR localProducts) const;
                              
//! Return curves which are silhoutte curves OTHER than hard edges
//! @return return false if not implemented.   return true if implemented -- but curves may still be empty.
GEOMDLLIMPEXP bool SilhouetteCurves(DPoint4dCR eyePoint, CurveVectorPtr &curves) const;

};


//! A DgnExtrusionDetail is a rotational sweep of a base CurveVector.
struct DgnRotationalSweepDetail : ZeroInit<DgnRotationalSweepDetail>
{
CurveVectorPtr m_baseCurve; // Curve to be swept. 
DRay3d  m_axisOfRotation;
double m_sweepAngle; // major circle sweep angle 
bool m_capped; // true if end cap is enabled 
size_t m_numVRules; // Number of v rules (radial around) to display in wireframe.

//! Detail consructor with complete field list as parameters ...
//! @param [in] baseCurve Curve to be swept. This pointer is captured (not cloned) into the extrusion.
//! @param [in] center Center of rotation
//! @param [in] axis axis of rotation.
//! @param [in] sweepAngle major circle sweep angle
//! @param [in] capped true if end cap is enabled
GEOMDLLIMPEXP DgnRotationalSweepDetail (
    CurveVectorPtr const &baseCurve,
    DPoint3dCR center,
    DVec3dCR  axis,
    double sweepAngle,
    bool capped
    );

//! Default value constructor.
GEOMDLLIMPEXP DgnRotationalSweepDetail ();

//! Return range.
GEOMDLLIMPEXP bool GetRange (DRange3dR range) const;
//! Return range when transformed
GEOMDLLIMPEXP bool GetRange (DRange3dR range, TransformCR transform) const;
//! Return coordinate system of
//! 1) Z axis along rotation
//! 2) X towards base curve extreme point.
GEOMDLLIMPEXP bool TryGetConstructiveFrame (TransformR localToWorld, TransformR worldToLocal) const;

//! Test for same type and structure (but no coordinate comparison)
 GEOMDLLIMPEXP bool IsSameStructure (ISolidPrimitiveCR other) const;
//! Test for same type, structure and coordinates.
GEOMDLLIMPEXP bool IsSameStructureAndGeometry (ISolidPrimitiveCR other, double tolerance) const;
//! Test if this is a closed volume.
GEOMDLLIMPEXP bool IsClosedVolume () const;


//! return transforms for the coordinate frame with z axis on the line of rotation.
//! @param [out] localToWorld translation part is is a point on the axis;  z direction is the rotation axis.
//! @param [out] worldToLocal inverse transform.
GEOMDLLIMPEXP bool GetTransforms (TransformR localToWorld, TransformR worldToLocal) const;

//! Get transform from base cap to fractional v position.
//! @param [out] transform transform to rotate a point.
//! @param [out] derivativeTransform Derivative with respect to vFraction.
//!         The output of derivativeTransform * xyz is a vector, not a point !!!
//!         (If viewed as a 4x4 matrix, its ww entry is zero)
//! @param [in] vFraction fraction of sweep angle.
GEOMDLLIMPEXP bool GetVFractionTransform
(
double vFraction,
TransformR transform,
TransformR derivativeTransform
) const;

//! Fractional profile curve
GEOMDLLIMPEXP CurveVectorPtr VFractionToProfile (double fraction) const;

//! Get the axis of rotation, negated if necessary to force a positive sweep angle.
//! @param center OUT center of rotation.
//! @param axis OUT axis of rotation.
//! @param sweepRadians OUT angle of rotation.
GEOMDLLIMPEXP bool TryGetRotationAxis (DPoint3dR center, DVec3dR axis, double &sweepRadians) const;
//! Compute intersections with a ray and add to the pickData.
//! @param [in] ray origin and direction
//! @param [in] minParameter smallest parameter of pick.  (e.g. 0 for picking
//!     along a positive ray from an eyepoint)
//! @param [in,out] pickData growing array of picks.
//! @param [in] parentId caller's id for this solid.  Will be placed
//!               in each pickData entry has parentId.
GEOMDLLIMPEXP void AddRayIntersections
    (
    bvector<SolidLocationDetail> &pickData,
    DRay3dCR ray,
    int parentId,
    double minParameter
    ) const;

//! Compute intersections with curves and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void AddCurveIntersections
    (
    CurveVectorCR curves,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const;

//! Compute intersections with a single curve and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void AddCurveIntersections
    (
    ICurvePrimitiveCR curves,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const;

//! @param [in] spacePoint search for point close to here and on the solid surface
//! @param [out] pickDetail closest point data.
//! @return false if not supported.
GEOMDLLIMPEXP bool ClosestPoint
    (
    DPoint3dCR spacePoint,
    SolidLocationDetail &pickDetail
    ) const;
    
//! convert u,v fraction on specified face to xyz and derivatives.
//! @param [in] faceIndices face selection indices
//! @param [in] uFraction fractional coordinate in u direction.
//! @param [in] vFraction fractional coordinate in v direction.
//! @param [out] xyz point on face.
//! @param [out] dXdu derivative of face point wrt uFraction.
//! @param [out] dXdv derivative of face point wrt vFraction.
GEOMDLLIMPEXP bool TryUVFractionToXYZ
    (
    SolidLocationDetail::FaceIndices const &faceIndices,
    double uFraction,
    double vFraction,
    DPoint3dR xyz,
    DVec3dR dXdu,
    DVec3dR dXdv
    ) const;

//! Return a single face of the solid primitive
//! @param indices integer selectors for the face.
GEOMDLLIMPEXP IGeometryPtr GetFace (SolidLocationDetail::FaceIndices const &indices) const;

//! Fill a list of all possible face indices.
//! @param [out] indices array of selectors for the faces.
GEOMDLLIMPEXP void GetFaceIndices (bvector<SolidLocationDetail::FaceIndices> &indices) const;

//! Copy a section curve at constant v of a single face to a single ICurvePrimitive.
//! @param [in] indices selects face
//! @param [in] fraction v fraction for section curve
GEOMDLLIMPEXP ICurvePrimitivePtr GetConstantVSection (SolidLocationDetail::FaceIndices const & indices, double fraction) const;

//! Copy a section at constant u of a single face to a single ICurvePrimitive.
//! @param [in] indices selects face
//! @param [in] fraction u fraction for section curve
GEOMDLLIMPEXP ICurvePrimitivePtr GetConstantUSection (SolidLocationDetail::FaceIndices const & indices, double fraction) const;

//! Compute the lengths of the longest u and v direction sections of a single face of a solid primitive.
//! @param [in] indices selects face.
//! @param [in] uvLength sizes in u, v directions.
GEOMDLLIMPEXP bool TryGetMaxUVLength (SolidLocationDetail::FaceIndices const & indices, DVec2dR uvLength) const;
//! Copy to a new (allocated) solid primitive.
GEOMDLLIMPEXP ISolidPrimitivePtr Clone () const;
//! Transform in place.
GEOMDLLIMPEXP bool TransformInPlace (TransformCR transform);

//! Return the various integrated products for area moment calculations.   The primitive is treated as a thin shell.
//! @param [out] localToWorld transform from preferred system where the products are most easily computed to world.
//! @param [out] localProducts integrated [xx xy xz x; xy yy yz y; xz yz zz z; x y z 1] dA
//! @return false if unable to compute.
GEOMDLLIMPEXP bool ComputeSecondMomentAreaProducts (TransformR localToWorld, DMatrix4dR localProducts) const;

//! Return the various integrated products for moment calculations.  The primitive is treated as a volume
//! @param [out] localToWorld transform from preferred system where the products are most easily computed to world.
//! @param [out] localProducts integrated [xx xy xz x; xy yy yz y; xz yz zz z; x y z 1] dA
//! @return false if unable to compute.
GEOMDLLIMPEXP bool ComputeSecondMomentVolumeProducts (TransformR localToWorld, DMatrix4dR localProducts) const;

//! Return curves which are silhoutte curves OTHER than hard edges
//! @return return false if not implemented.   return true if implemented -- but curves may still be empty.
GEOMDLLIMPEXP bool SilhouetteCurves(DPoint4dCR eyePoint, CurveVectorPtr &curves) const;

//! Return true if capped and incomplete sweep.
GEOMDLLIMPEXP bool HasRealCaps () const;

//! Set the number of v rules to display in wireframe.
//! @note Default when not explicitly set is to show 4 v rules for a full sweep, this is the minimum number allowed.
//! @remarks You are not required to set this and it is better not to. This is primarily intended for Type 18/19 surfaces/solids of revolution display compatibility.
GEOMDLLIMPEXP void SetVRuleCount (size_t numVRules);

//! Get the number of v rules that will be displayed in wireframe.
GEOMDLLIMPEXP size_t GetVRuleCount () const;

//! Compute v rule count for a given sweep angle and desired number of v rules for a full sweep.
static GEOMDLLIMPEXP size_t ComputeVRuleCount (double sweepRadians, size_t numVRulesFullSweep = 4);
                          
enum class RadiusType
{
Minimum,                // Radius at closest point on profile   
Maximum,                 // Radius at farthest point on profile
Centroidal              // Radius at centroid (area centroid if closed,  wire centroid if open).
};


//! Return the radius
//! @param [out] radius 
//! @param [in] type of radius.
GEOMDLLIMPEXP bool   GetRadius (double& radius, RadiusType type) const;

//! Set the radius (by translating the profile).

//! @param [in] radius 
//! @param [in] type of radius.
GEOMDLLIMPEXP bool   SetRadius (double radius, RadiusType type);

 
}; // DgnRotationalSweepDetail



//! A DgnRuledSweepDetail is a ruled surface between corresponding points of CurveVectors.
struct DgnRuledSweepDetail  : ZeroInit<DgnRuledSweepDetail>
{
bvector<CurveVectorPtr> m_sectionCurves; // Successive section curves 
bool m_capped; // true if end cap is enabled 

//! Detail consructor with complete field list as parameters ...
//! @param [in] sectionCurves Successive section curves
//! @param [in] capped true if end cap is enabled
GEOMDLLIMPEXP DgnRuledSweepDetail (
    bvector<CurveVectorPtr> const & sectionCurves,
    bool capped);


//! Detail consructor for common case with exactly 2 sections.
//! @param [in] sectionA First section curve
//! @param [in] sectionB Second section curve
//! @param [in] capped true if end cap is enabled
GEOMDLLIMPEXP DgnRuledSweepDetail (
    CurveVectorPtr const & sectionA,
    CurveVectorPtr const & sectionB,
    bool capped);

//! Default value constructor.
GEOMDLLIMPEXP DgnRuledSweepDetail ();


//! Add (capture) a section.
GEOMDLLIMPEXP void AddSection (CurveVectorP section);

//! Get the lower and upper curves of a specified face.
//! @param [in] indices face indices
//! @param [out] curveA lower curve of face
//! @param [out] curveB upper curve of face
//! @return false if indices is not a valid ruled face.  (Caps return false)
GEOMDLLIMPEXP bool TryGetCurvePair (
SolidLocationDetail::FaceIndices const &indices,
ICurvePrimitivePtr &curveA,
ICurvePrimitivePtr &curveB
) const;

//! Return range.
GEOMDLLIMPEXP bool GetRange (DRange3dR range) const;
//! Return range when transformed
GEOMDLLIMPEXP bool GetRange (DRange3dR range, TransformCR transform) const;
//! Return coordinate system (LOCAL_COORDINATES_UnitAxesAtStart) for base curve vector.
GEOMDLLIMPEXP bool TryGetConstructiveFrame (TransformR localToWorld, TransformR worldToLocal) const;

//! Test for same type and structure (but no coordinate comparison)
 GEOMDLLIMPEXP bool IsSameStructure (ISolidPrimitiveCR other) const;
//! Test for same type, structure and coordinates.
GEOMDLLIMPEXP bool IsSameStructureAndGeometry (ISolidPrimitiveCR other, double tolerance) const;
//! Test if this is a closed volume.
GEOMDLLIMPEXP bool IsClosedVolume () const;


//! Compute intersections with a ray and add to the pickData.
//! @param [in] ray origin and direction
//! @param [in] minParameter smallest parameter of pick.  (e.g. 0 for picking
//!     along a positive ray from an eyepoint)
//! @param [in,out] pickData growing array of picks.
//! @param [in] parentId caller's id for this solid.  Will be placed
//!               in each pickData entry has parentId.
GEOMDLLIMPEXP void AddRayIntersections
    (
    bvector<SolidLocationDetail> &pickData,
    DRay3dCR ray,
    int parentId,
    double minParameter
    ) const;


//! Compute intersections with curves and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void AddCurveIntersections
    (
    CurveVectorCR curves,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const;

//! Compute intersections with a single curve and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void AddCurveIntersections
    (
    ICurvePrimitiveCR curves,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const;
//! @param [in] spacePoint search for point close to here and on the solid surface
//! @param [out] pickDetail closest point data.
//! @return false if not supported.
GEOMDLLIMPEXP bool ClosestPoint
    (
    DPoint3dCR spacePoint,
    SolidLocationDetail &pickDetail
    ) const;
    
//! convert u,v fraction on specified face to xyz and derivatives.
//! @param [in] faceIndices face selection indices
//! @param [in] uFraction fractional coordinate in u direction.
//! @param [in] vFraction fractional coordinate in v direction.
//! @param [out] xyz point on face.
//! @param [out] dXdu derivative of face point wrt uFraction.
//! @param [out] dXdv derivative of face point wrt vFraction.
GEOMDLLIMPEXP bool TryUVFractionToXYZ
    (
    SolidLocationDetail::FaceIndices const &faceIndices,
    double uFraction,
    double vFraction,
    DPoint3dR xyz,
    DVec3dR dXdu,
    DVec3dR dXdv
    ) const;
//! Return a single face of the solid primitive
//! @param indices integer selectors for the face.
//! <remarks>Face orientation convention
//!<pre>
//!<ul>
//!<li>There is a natural "forward" direction along section curves.
//!<li>There is a natural "direction" to the (swept) side faces. (e.g. extrusion vector, positive rotation, rule line direction)
//!<li>The cross product of the curve direction and sweep direction is the outward normal for side faces.
//!<li>Hence base section is REVERSED when used for the Cap0 face.
//!<li>Hence the base section is NOT reversed when used for the Cap1 face.
//!</ul>
//!</pre>
//!</remarks>
GEOMDLLIMPEXP IGeometryPtr GetFace (SolidLocationDetail::FaceIndices const &indices) const;

//! Fill a list of all possible face indices.
//! @param [out] indices array of selectors for the faces.
GEOMDLLIMPEXP void GetFaceIndices (bvector<SolidLocationDetail::FaceIndices> &indices) const;

//! Copy a section curve at constant v of a single face to a single ICurvePrimitive.
//! @param [in] indices selects face
//! @param [in] fraction v fraction for section curve
GEOMDLLIMPEXP ICurvePrimitivePtr GetConstantVSection (SolidLocationDetail::FaceIndices const & indices, double fraction) const;

//! Copy a section at constant u of a single face to a single ICurvePrimitive.
//! @param [in] indices selects face
//! @param [in] fraction u fraction for section curve
GEOMDLLIMPEXP ICurvePrimitivePtr GetConstantUSection (SolidLocationDetail::FaceIndices const & indices, double fraction) const;

//! Compute the lengths of the longest u and v direction sections of a single face of a solid primitive.
//! @param [in] indices selects face.
//! @param [in] uvLength sizes in u, v directions.
GEOMDLLIMPEXP bool TryGetMaxUVLength (SolidLocationDetail::FaceIndices const & indices, DVec2dR uvLength) const;
//! Copy to a new (allocated) solid primitive.
GEOMDLLIMPEXP ISolidPrimitivePtr Clone () const;
//! Transform in place.
GEOMDLLIMPEXP bool TransformInPlace (TransformCR transform);

/*__PUBLISH_SECTION_END__*/
//! Compute translation between specified section curves. Returns true if no scale/twist/tilt.
GEOMDLLIMPEXP bool GetSectionCurveTranslation (DVec3dR translation, size_t section0, size_t section1) const;
/*__PUBLISH_SECTION_START__*/

//! Return the various integrated products for area moment calculations.   The primitive is treated as a thin shell.
//! @param [out] localToWorld transform from preferred system where the products are most easily computed to world.
//! @param [out] localProducts integrated [xx xy xz x; xy yy yz y; xz yz zz z; x y z 1] dA
//! @return false if unable to compute.
GEOMDLLIMPEXP bool ComputeSecondMomentAreaProducts (TransformR localToWorld, DMatrix4dR localProducts) const;

//! Return the various integrated products for moment calculations.  The primitive is treated as a volume
//! @param [out] localToWorld transform from preferred system where the products are most easily computed to world.
//! @param [out] localProducts integrated [xx xy xz x; xy yy yz y; xz yz zz z; x y z 1] dA
//! @return false if unable to compute.
GEOMDLLIMPEXP bool ComputeSecondMomentVolumeProducts (TransformR localToWorld, DMatrix4dR localProducts) const;

//! Return curves which are silhoutte curves OTHER than hard edges
//! @return return false if not implemented.   return true if implemented -- but curves may still be empty.
GEOMDLLIMPEXP bool SilhouetteCurves(DPoint4dCR eyePoint, CurveVectorPtr &curves) const;

};


//! ISolidPrimitive is an interface around the 7 solid primitive types -- DgnConeDetail, DgnTorusDetail, DgnSphereDetail, DgnBoxDetail, DgnExtrusionDetail, DgnRotationalSweepDetail, DgnRuledSurfaceDetail.
//! @ingroup GROUP_Geometry
struct ISolidPrimitive : public RefCountedBase
{
private:
protected:
    ISolidPrimitive ();

// Protected side of VPP wrappers.  Implementations in IsolidPrimitive return false. Concrete classes implement the one relevant to them.
    GEOMAPI_VIRTUAL SolidPrimitiveType _GetSolidPrimitiveType () const = 0;
    GEOMAPI_VIRTUAL bool _GetRange (DRange3dR range) const = 0;
    GEOMAPI_VIRTUAL bool _GetRange (DRange3dR range, TransformCR transform) const = 0;
    GEOMAPI_VIRTUAL bool _TryGetConstructiveFrame (TransformR localToWorld, TransformR worldToLocal) const = 0;
    GEOMAPI_VIRTUAL bool _IsSameStructure (ISolidPrimitiveCR other) const = 0;
    GEOMAPI_VIRTUAL bool _IsSameStructureAndGeometry (ISolidPrimitiveCR other, double tolerance) const = 0;
    GEOMAPI_VIRTUAL bool _IsClosedVolume () const = 0;
    GEOMAPI_VIRTUAL IGeometryPtr _GetFace (SolidLocationDetail::FaceIndices const & indices) const = 0;
    GEOMAPI_VIRTUAL void _GetFaceIndices (bvector<SolidLocationDetail::FaceIndices> &indices) const = 0;
    GEOMAPI_VIRTUAL ICurvePrimitivePtr _GetConstantVSection (SolidLocationDetail::FaceIndices const & indices, double fraction) const = 0;
    GEOMAPI_VIRTUAL ICurvePrimitivePtr _GetConstantUSection (SolidLocationDetail::FaceIndices const & indices, double fraction) const = 0;
    GEOMAPI_VIRTUAL bool _TryGetMaxUVLength (SolidLocationDetail::FaceIndices const & indices, DVec2dR uvLength) const = 0;

    GEOMAPI_VIRTUAL bool _TryUVFractionToXYZ (
            SolidLocationDetail::FaceIndices const &faceIndices,
            double uFraction, double vFraction,
            DPoint3dR xyz, DVec3dR dXdu, DVec3dR dXdv
            ) const = 0;

    GEOMAPI_VIRTUAL void _AddRayIntersections (
            bvector<SolidLocationDetail> &pickData, DRay3dCR ray,
                    int parentId, double minParameter) const = 0;


    GEOMAPI_VIRTUAL void _AddCurveIntersections (
        CurveVectorCR curves,
        bvector<CurveLocationDetail> &curvePoints,
        bvector<SolidLocationDetail> &solidPoints,
        MeshAnnotationVector &messages
        ) const = 0;

    GEOMAPI_VIRTUAL void _AddCurveIntersections (
        ICurvePrimitiveCR curves,
        bvector<CurveLocationDetail> &curvePoints,
        bvector<SolidLocationDetail> &solidPoints,
        MeshAnnotationVector &messages
        ) const = 0;


    GEOMAPI_VIRTUAL bool _ClosestPoint (DPoint3dCR spacePoint, SolidLocationDetail &pickDetail) const = 0;
    
    GEOMAPI_VIRTUAL bool _TryGetDgnTorusPipeDetail (DgnTorusPipeDetailR data) const;
    GEOMAPI_VIRTUAL bool _TrySetDgnTorusPipeDetail (DgnTorusPipeDetailCR data);

    GEOMAPI_VIRTUAL bool _TryGetDgnConeDetail (DgnConeDetailR data) const;
    GEOMAPI_VIRTUAL bool _TrySetDgnConeDetail (DgnConeDetailCR data);

    GEOMAPI_VIRTUAL bool _TryGetDgnBoxDetail (DgnBoxDetailR data) const;
    GEOMAPI_VIRTUAL bool _TrySetDgnBoxDetail (DgnBoxDetailCR data);

    GEOMAPI_VIRTUAL bool _TryGetDgnSphereDetail (DgnSphereDetailR data) const;
    GEOMAPI_VIRTUAL bool _TrySetDgnSphereDetail (DgnSphereDetailCR data);

    GEOMAPI_VIRTUAL bool _TryGetDgnExtrusionDetail (DgnExtrusionDetailR data) const;
    GEOMAPI_VIRTUAL bool _TrySetDgnExtrusionDetail (DgnExtrusionDetailCR data);

    GEOMAPI_VIRTUAL bool _TryGetDgnRotationalSweepDetail (DgnRotationalSweepDetailR data) const;
    GEOMAPI_VIRTUAL bool _TrySetDgnRotationalSweepDetail (DgnRotationalSweepDetailCR data);

    GEOMAPI_VIRTUAL bool _TryGetDgnRuledSweepDetail (DgnRuledSweepDetailR data) const;
    GEOMAPI_VIRTUAL bool _TrySetDgnRuledSweepDetail (DgnRuledSweepDetailCR data);

    GEOMAPI_VIRTUAL bool _GetCapped () const = 0;
    GEOMAPI_VIRTUAL void _SetCapped (bool value) = 0;
    GEOMAPI_VIRTUAL ISolidPrimitivePtr _Clone () const = 0;
    GEOMAPI_VIRTUAL bool _TransformInPlace (TransformCR transform) = 0;
    GEOMAPI_VIRTUAL bool _ComputeSecondMomentAreaProducts (TransformR localToWorld, DMatrix4dR localProducts) const = 0;
    GEOMAPI_VIRTUAL bool _ComputeSecondMomentVolumeProducts (TransformR localToWorld, DMatrix4dR localProducts) const = 0;
    
    GEOMAPI_VIRTUAL bool _SilhouetteCurves(DPoint4dCR eyePoint, CurveVectorPtr &curves) const = 0;

public:
    //! Query the primitive type.
    GEOMDLLIMPEXP SolidPrimitiveType GetSolidPrimitiveType () const;

    //! Return a new ISolidPrimtive which is a DgnTorusPipe.
    static GEOMDLLIMPEXP ISolidPrimitivePtr CreateDgnTorusPipe (DgnTorusPipeDetailCR data);

    //! Ask if this is a DgnTorusPipe
    //! If so, return true with copy of detail data.
    //! If not, return false, leave data untouched.
    bool GEOMDLLIMPEXP TryGetDgnTorusPipeDetail (DgnTorusPipeDetailR data) const;

    //! Set DgnTorusPipeDetail content.
    bool GEOMDLLIMPEXP TrySetDgnTorusPipeDetail (DgnTorusPipeDetailCR data);


    //! Return a new ISolidPrimtive which is a DgnCone.
    static GEOMDLLIMPEXP ISolidPrimitivePtr CreateDgnCone (DgnConeDetailCR data);

    //! Ask if this is a DgnCone
    //! If so, return true with copy of detail data.
    //! If not, return false, leave data untouched.
    bool GEOMDLLIMPEXP TryGetDgnConeDetail (DgnConeDetailR data) const;

    //! Set DgnConeDetail content.
    bool GEOMDLLIMPEXP TrySetDgnConeDetail (DgnConeDetailCR data);


    //! Return a new ISolidPrimtive which is a DgnBox.
    static GEOMDLLIMPEXP ISolidPrimitivePtr CreateDgnBox (DgnBoxDetailCR data);

    //! Ask if this is a DgnBox
    //! If so, return true with copy of detail data.
    //! If not, return false, leave data untouched.
    bool GEOMDLLIMPEXP TryGetDgnBoxDetail (DgnBoxDetailR data) const;

    //! Set DgnBoxDetail content.
    bool GEOMDLLIMPEXP TrySetDgnBoxDetail (DgnBoxDetailCR data);


    //! Return a new ISolidPrimtive which is a DgnSphere.
    static GEOMDLLIMPEXP ISolidPrimitivePtr CreateDgnSphere (DgnSphereDetailCR data);

    //! Ask if this is a DgnSphere
    //! If so, return true with copy of detail data.
    //! If not, return false, leave data untouched.
    bool GEOMDLLIMPEXP TryGetDgnSphereDetail (DgnSphereDetailR data) const;

    //! Set DgnSphereDetail content.
    bool GEOMDLLIMPEXP TrySetDgnSphereDetail (DgnSphereDetailCR data);


    //! Return a new ISolidPrimtive which is a DgnExtrusion.
    static GEOMDLLIMPEXP ISolidPrimitivePtr CreateDgnExtrusion (DgnExtrusionDetailCR data);

    //! Ask if this is a DgnExtrusion
    //! If so, return true with copy of detail data.
    //! If not, return false, leave data untouched.
    bool GEOMDLLIMPEXP TryGetDgnExtrusionDetail (DgnExtrusionDetailR data) const;

    //! Set DgnExtrusionDetail content.
    bool GEOMDLLIMPEXP TrySetDgnExtrusionDetail (DgnExtrusionDetailCR data);




    //! Return a new ISolidPrimtive which is a DgnRotationalSweep.
    static GEOMDLLIMPEXP ISolidPrimitivePtr CreateDgnRotationalSweep (DgnRotationalSweepDetailCR data);

    //! Ask if this is a DgnRotationalSweep
    //! If so, return true with copy of detail data.
    //! If not, return false, leave data untouched.
    bool GEOMDLLIMPEXP TryGetDgnRotationalSweepDetail (DgnRotationalSweepDetailR data) const;

    //! Set DgnRotationalSweepDetail content.
    bool GEOMDLLIMPEXP TrySetDgnRotationalSweepDetail (DgnRotationalSweepDetailCR data);




    //! Return a new ISolidPrimtive which is a DgnRuledSweep.
    static GEOMDLLIMPEXP ISolidPrimitivePtr CreateDgnRuledSweep (DgnRuledSweepDetailCR data);

    //! Ask if this is a DgnRuledSweep
    //! If so, return true with copy of detail data.
    //! If not, return false, leave data untouched.
    bool GEOMDLLIMPEXP TryGetDgnRuledSweepDetail (DgnRuledSweepDetailR data) const;

    //! Set DgnRuledSweepDetail content.
    bool GEOMDLLIMPEXP TrySetDgnRuledSweepDetail (DgnRuledSweepDetailCR data);



    //! Compute range of primitive
    bool GEOMDLLIMPEXP GetRange (DRange3dR range) const;
    //! Return range when transformed
    GEOMDLLIMPEXP bool GetRange (DRange3dR range, TransformCR transform) const;
    //! Return a typical coordinate frame
    GEOMDLLIMPEXP bool TryGetConstructiveFrame (TransformR localToWorld, TransformR worldToLocal) const;

    //! Test for same type and structure (but no coordinate comparison)
     GEOMDLLIMPEXP bool IsSameStructure (ISolidPrimitiveCR other) const;
    //! Test for same type, structure and coordinates.
    GEOMDLLIMPEXP bool IsSameStructureAndGeometry (ISolidPrimitiveCR other, double tolerance = 0.0) const;
    //! Test if this is a closed volume.
    GEOMDLLIMPEXP bool IsClosedVolume () const;


    //! return flag for cap surface.  Note that sphere and torus can be closed
    //! without having a cap surface, so this is not a closure test, just a test
    //! if the field for capping is set.
    bool GEOMDLLIMPEXP GetCapped () const;

    //! Set flag for cap surface.
    GEOMDLLIMPEXP void SetCapped (bool value);

//! Compute intersections with a ray and add to the pickData.
//! @param [in] ray origin and direction
//! @param [in] minParameter smallest parameter of pick.  (e.g. 0 for picking
//!     along a positive ray from an eyepoint)
//! @param [in,out] pickData growing array of picks.
//! @param [in] parentId caller's id for this solid.  Will be placed
//!               in each pickData entry has parentId.
    GEOMDLLIMPEXP  void AddRayIntersections
    (
    bvector<SolidLocationDetail> &pickData,
    DRay3dCR ray,
    int parentId = 0,
    double minParameter = -DBL_MAX
    ) const;

//! Compute intersections with curves and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void AddCurveIntersections
    (
    CurveVectorCR curves,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const;

//! Compute intersections with a single curve and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void AddCurveIntersections
    (
    ICurvePrimitiveCR curves,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const;

//! convert u,v fraction on specified face to xyz and derivatives.
//! @param [in] faceIndices face selection indices
//! @param [in] uFraction fractional coordinate in u direction.
//! @param [in] vFraction fractional coordinate in v direction.
//! @param [out] xyz point on face.
//! @param [out] dXdu derivative of face point wrt uFraction.
//! @param [out] dXdv derivative of face point wrt vFraction.
    GEOMDLLIMPEXP bool TryUVFractionToXYZ
        (
        SolidLocationDetail::FaceIndices const &faceIndices,
        double uFraction,
        double vFraction,
        DPoint3dR xyz,
        DVec3dR dXdu,
        DVec3dR dXdv
        ) const;

//! @param [in] spacePoint search for point close to here and on the solid surface
//! @param [out] pickDetail closest point data.
//! @return false if not supported.
    GEOMDLLIMPEXP bool ClosestPoint
        (
        DPoint3dCR spacePoint,
        SolidLocationDetail &pickDetail
        ) const;

//! Copy a single face to a new object.
//! @param [in] indices indices of face.
//! @return IGeometryPtr with representing a single face.
//! End caps will always return as CurveVectorPtr.
//! Planar side faces will return as CurveVectorPtr.
//! Cylindrical side faces will return as DgnConeDetail with no cap.
//! Other ruled side faces will appear as DgnRuledSweepDetail with one swept primitive and no cap.
//! Rotational sweeps of arcs will appear as (preferably) DgnTorusPipeDetail or DgnRotationalRotationalSweep.
    GEOMDLLIMPEXP IGeometryPtr GetFace (SolidLocationDetail::FaceIndices const & indices) const;

//! Fill an array containing all the valid face indices for this primitive.
//! @param [out] indices array of FaceIndices of all faces of the solid.
    GEOMDLLIMPEXP void GetFaceIndices (bvector <SolidLocationDetail::FaceIndices> &indices) const;

//! Copy a section curve at constant v of a single face to a single ICurvePrimitive.
//! This is usually a single primitive, but can have a disjoint, multi-component child for cut of non-convex cap.
//! @param [in] indices selects face
//! @param [in] fraction v fraction for section curve
    GEOMDLLIMPEXP ICurvePrimitivePtr GetConstantVSection (SolidLocationDetail::FaceIndices const & indices, double fraction) const;

//! Copy a section at constant u of a single face to a single ICurvePrimitive.
//! This is usually a single primitive, but can have a disjoint, multi-component child for cut of non-convex cap,
//! @param [in] indices selects face
//! @param [in] fraction u fraction for section curve
    GEOMDLLIMPEXP ICurvePrimitivePtr GetConstantUSection (SolidLocationDetail::FaceIndices const & indices, double fraction) const;

//! Compute the lengths of the longest u and v direction sections of a single face of a solid primitive.
//! @param [in] indices selects face.
//! @param [in] uvLength sizes in u, v directions.
    GEOMDLLIMPEXP bool TryGetMaxUVLength (SolidLocationDetail::FaceIndices const & indices, DVec2dR uvLength) const;

//! Deep copy.
    GEOMDLLIMPEXP ISolidPrimitivePtr Clone () const;
//! Transform in place.
    GEOMDLLIMPEXP bool TransformInPlace (TransformCR transform);

//! Detect a solid primitive that could be represented as a simpler type and change it to the simple type.
//! Tests for rotational sweeps that are a sphere or torus as well as extrusions/ruled sweeps that are a box, cone, or extrusion.
//! @return true if input primitive was simplified.
    static GEOMDLLIMPEXP bool Simplify (ISolidPrimitivePtr& primitive);

//! Return true if solid primitve has a non-linear edge or non-planar face.
    GEOMDLLIMPEXP bool HasCurvedFaceOrEdge () const;

//! Return the volume, centroid, orientation, and principal moments using most accurate (possibly exact) formulas.
//! @remark This method only attempts "exact" computations.  Use ComputeFacetedPrincipalMoments to get approximate moments.
//! @param [out] volume volume
//! @param [out] centroid centroid
//! @param [out] axes columns of this matrix are the principal directions.
//! @param [out] momentxyz moments (yy+zz,xx+zz,xx+yy) around the principal directions.
//! @return false if (a) solid primitive is not capped or (b) high accuracy moments are not supported for this type.
    GEOMDLLIMPEXP bool ComputePrincipalMoments (
                              double &volume,
                              DVec3dR centroid,
                              RotMatrixR axes,
                              DVec3dR momentxyz
                              ) const;

//! Return the volume, centroid, orientation, and principal moments, using a faceted approximation.
//! @remark This method only attempts faceted computations.  Use ComputePrincipalMoments to get exact moments.
//! @param [in] options optional controls for facets
//! @param [in] options optional controls for facet density.
//! @param [out] volume volume
//! @param [out] centroid centroid
//! @param [out] axes columns of this matrix are the principal directions.
//! @param [out] momentxyz moments (yy+zz,xx+zz,xx+yy) around the principal directions.
//! @return false if (a) solid primitive is not capped or (b) facets failed.
    GEOMDLLIMPEXP bool ComputeFacetedPrincipalMoments (
                              IFacetOptionsP options,
                              double &volume,
                              DVec3dR centroid,
                              RotMatrixR axes,
                              DVec3dR momentxyz
                              ) const;


//! Return the volume, centroid, orientation, and principal moments of the surface area (shell) using most accurate (possibly exact) formulas.
//! @remark This method only attempts "exact" computations.  Use ComputeFacetedPrincipalMoments to get approximate moments.
//! @param [out] area surface area
//! @param [out] centroid centroid
//! @param [out] axes columns of this matrix are the principal directions.
//! @param [out] momentxyz moments (yy+zz,xx+zz,xx+yy) around the principal directions.
//! @return false if (a) solid primitive is not capped or (b) high accuracy moments are not supported for this type.
    GEOMDLLIMPEXP bool ComputePrincipalAreaMoments (
                              double &area,
                              DVec3dR centroid,
                              RotMatrixR axes,
                              DVec3dR momentxyz
                              ) const;

//! Return the volume, centroid, orientation, and principal momentsof the surface area (shell) , using a faceted approximation.
//! @remark This method only attempts faceted computations.  Use ComputePrincipalMoments to get exact moments.
//! @param [in] options optional controls for facets
//! @param [in] options optional controls for facet density.
//! @param [out] area surface area
//! @param [out] centroid centroid
//! @param [out] axes columns of this matrix are the principal directions.
//! @param [out] momentxyz moments (yy+zz,xx+zz,xx+yy) around the principal directions.
//! @return false if (a) solid primitive is not capped or (b) facets failed.
    GEOMDLLIMPEXP bool ComputeFacetedPrincipalAreaMoments (
                              IFacetOptionsP options,
                              double &area,
                              DVec3dR centroid,
                              RotMatrixR axes,
                              DVec3dR momentxyz
                              ) const;

//! Return the various integrated products for area moment calculations.   The primitive is treated as a thin shell.
//! @param [out] localToWorld transform from preferred system where the products are most easily computed to world.
//! @param [out] localProducts integrated [xx xy xz x; xy yy yz y; xz yz zz z; x y z 1] dA
//! @return false if unable to compute.
GEOMDLLIMPEXP bool ComputeSecondMomentAreaProducts (TransformR localToWorld, DMatrix4dR localProducts) const;

//! Return the various integrated products for moment calculations.  The primitive is treated as a volume
//! @param [out] localToWorld transform from preferred system where the products are most easily computed to world.
//! @param [out] localProducts integrated [xx xy xz x; xy yy yz y; xz yz zz z; x y z 1] dA
//! @return false if unable to compute.
GEOMDLLIMPEXP bool ComputeSecondMomentVolumeProducts (TransformR localToWorld, DMatrix4dR localProducts) const;

//! Return the various integrated products for area moment calculations.   The primitive is treated as a thin shell.
//! @param [out] worldProducts integrated [xx xy xz x; xy yy yz y; xz yz zz z; x y z 1] dA
//! @return false if unable to compute.
GEOMDLLIMPEXP bool ComputeSecondMomentAreaProducts (DMatrix4dR worldProducts) const;

//! Return curves which are silhoutte curves OTHER than hard edges
//! @param [in] eyePoint For flat view, the view direction with weight=0.  For perspective, the eye point with weight=1.
//! @param [in] curves silhouette curves.
//! @return return false if not implemented.   return true if implemented -- but curves may still be empty.
GEOMDLLIMPEXP bool SilhouetteCurves(DPoint4dCR eyePoint, CurveVectorPtr &curves) const;

//! Return the various integrated products for moment calculations.  The primitive is treated as a volume
//! @param [out] worldProducts integrated [xx xy xz x; xy yy yz y; xz yz zz z; x y z 1] dA
//! @return false if unable to compute.
GEOMDLLIMPEXP bool ComputeSecondMomentVolumeProducts (DMatrix4dR worldProducts) const;

//! Return a polyface mesh approximation of the solid primitive.
GEOMDLLIMPEXP PolyfaceHeaderPtr Facet (IFacetOptionsPtr const &options);

//! Apply the canonical changes to reverse (u,v) fraction space orientation.
//!<ul>
//!<li>u replaced by {1-u)
//!<li>v is unchanged
//!<li>dXdu is negated
//!<li>dXdv is unchanged
//!</ul>
static GEOMDLLIMPEXP void ReverseFractionOrientation (double &u, double &v, DVec3dR dXdu, DVec3dR dXdv)
    {
    u = 1.0 - u;
    dXdu.Negate ();
    }

static GEOMDLLIMPEXP void ReverseFractionOrientation (double &u, double &v){u = 1.0 - u;}   // and v is unchanged
static GEOMDLLIMPEXP void ReverseFractionOrientation (DVec3dR dXdu, DVec3dR dXdv) {dXdu.Negate ();} // and dXdv is unchanged


static GEOMDLLIMPEXP void ReverseFractionOrientation (SolidLocationDetail &pd)
    {
    ReverseFractionOrientation (pd.m_uParameter, pd.m_vParameter, pd.m_uDirection, pd.m_vDirection);
    }
};
END_BENTLEY_GEOMETRY_NAMESPACE
