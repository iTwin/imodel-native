/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#ifdef __cplusplus
class GEOMDLLIMPEXP DMoments3d
{
private:
    Transform mLocalToWorld;    // Coordinate frame used in moment calculation
    Transform mWorldToLocal;    // Inverse frame
    bool      mbWorldToLocalOK; // true if mLocalToWorld was inverted.
    double    mMass;          // Volume -- integral of density * dV
    double    mIx, mIy, mIz;    // Integrals of x dV, y dV, z dV
    double    mIxx, mIyy, mIzz; // Integrals of xx dV, yy dV, zz dV.  These are RAW integrals ---
                                //    do not confuse with "moments" Mxx = Iyy + Izz
    double    mIxy, mIyz, mIzx; // Integrals of xy dV, yz dV, zx dV

protected:
//!
//! @description Add a scaled outer product to the second moments.
//!
void AddScaledOuterProduct
(
DPoint3dCR vectorU,
DPoint3dCR vectorV,
double   a
);


public:

//!
//! @description Constructor for a moment structure with all integrals zero in an identity frame.
//!
DMoments3d ();


//!
//! @description Replace the coordinate frame.
//! Integrals values are NOT changed.
//!
bool SetFrame (TransformCR localToWorld);


//!
//! @description Return all scalar integrals.
//! @param [out] localToWorld  the coordinate frame in which the integrals were evaluated.
//! @param [out] worldToLocal  the inverse frame.
//! @param [out] bWorldToLocalOK  flag confirming that the coordinate frame is nonsingular.
//! @param [out] mass  the mass of the underlying geometry.
//! @param [out] Ix  integral of (x dV)
//! @param [out] Iy  integral of (y dV)
//! @param [out] Iz  integral of (z dV)
//!
//! @param [out] Ixx  integral of (x^2 dV)
//! @param [out] Iyy  integral of (y^2 dV)
//! @param [out] Izz  integral of (z^2 dV)
//!
//! @param [out] Ixy  integral of (x y dV)
//! @param [out] Iyz  integral of (y z dV)
//! @param [out] Izx  integral of (z x dV)
//!
void GetScalars
(
TransformR localToWorld,
TransformR worldToLocal,
bool       &bWorldToLocalOK,
double &mass,
double &Ix,
double &Iy,
double &Iz,
double &Ixx,
double &Iyy,
double &Izz,
double &Ixy,
double &Iyz,
double &Izx
);


//!
//! @description Return all scalar integrals.
//! @param [out] localToWorld  the coordinate frame in which the integrals were evaluated.
//! @param [out] worldToLocal  the inverse frame.
//! @param [out] bWorldToLocalOK  flag confirming that the coordinate frame is nonsingular.
//! @param [out] mass  the mass of the underlying geometry.
//! @param [out] moment1Vector  vector of moments (x dm, y dm, z dm)
//! @param [out] moment2Tensor  matrix with moments -- ij entry is integral of (deltaij r^2 - xi xj)dm
//!
void GetTensor
(
TransformR localToWorld,
TransformR worldToLocal,
bool       &bWorldToLocalOK,
double     &mass,
DVec3dR    moment1Vector,
RotMatrixR moment2Tensor
);




//!
//! @description Return a new moment structure with all moment integrals relative to a specified frame.
//!      (in effect, transform the geometry from old local to global, then to the new local)
//! @remark Be sure you understand the difference between
//!        CopyWithTransformAppliedToGeoemtry
//!        and CopyWithTransformToNewFrame.
//! @param [in] newLocaltoWorld  new coordinate frame.
//!
DMoments3d CopyWithTransformToNewFrame (TransformCR newLocalToWorld);


//!
//! @description Return a new moment structure with all integrals
//!    reflecting a transform applied to the coordinates used in the integrations.
//!    (i.e. same effect as a local-to-local transform applyed pointwise to the body,
//!     and integrating again in local coordinates.)
//! @remark Be sure you understand the difference between
//!        CopyWithTransformAppliedToFrame
//!        and CopyWithTransformToNewFrame.
//! @param [in] transform  transform applied to the underlying geometry.
//!
DMoments3d CopyWithTransformAppliedToFrame (TransformCR transform);


//!
//! @description Return a new moment structure whose integral values are
//!    the (scaled) sum of two inputs.
//!    Frame data is copied directly from momentA.
//!    There is no test for compatible frames.
//! @param [in] momentA  first set of moments.
//! @param [in] scaleA  scale factor for momentA
//! @param [in] momentB  second set of moments
//! @param [in] scaleB  scale factor for momentB
//!
static DMoments3d FromSumOf
(
DMoments3dCR momentA,
double scaleA,
DMoments3dCR momentB,
double scaleB
);


//!
//! @description Return a moment structure initialized with scalar values of
//!    each integral.
//! @param [in] localToWorld  the coordinate frame in which the integrals were evaluated.
//! @param [in] mass  the mass of the underlying geometry.
//! @param [in] Ix  integral of (x dV)
//! @param [in] Iy  integral of (y dV)
//! @param [in] Iz  integral of (z dV)
//!
//! @param [in] Ixx  integral of (x^2 dV)
//! @param [in] Iyy  integral of (y^2 dV)
//! @param [in] Izz  integral of (z^2 dV)
//!
//! @param [in] Ixy  integral of (x y dV)
//! @param [in] Iyz  integral of (y z dV)
//! @param [in] Izx  integral of (z x dV)
//!
static DMoments3d FromScalars
(
TransformCR localToWorld,
double mass = 0.0,
double Ix = 0.0,
double Iy = 0.0,
double Iz = 0.0,
double Ixx = 0.0,
double Iyy = 0.0,
double Izz = 0.0,
double Ixy = 0.0,
double Iyz = 0.0,
double Izx = 0.0
);



//!
//! @description Add a concentrated mass to the moments.
//! @param [in] mass  mass
//! @param [in] xyz  point where mass is concentrated
//! @param [in] bTransformToLocal  true if the the coordinate
//!    is to be transformed from world to local.
//!
bool AddConcentratedMass
(
double mass,
DPoint3dCR xyz,
bool    bTransformToLocal = false
);


//!
//! @description Add a wire mass to the moments.
//! @param [in] density  mass per unit length (after transform)
//! @param [in] xyzA  start point of line
//! @param [in] xyzB  end point of line
//! @param [in] bTransformToLocal  true if the the coordinate
//!    is to be transformed from world to local.
//!
bool AddWireMass
(
double density,
DPoint3dCR xyzA,
DPoint3dCR xyzB,
bool    bTransformToLocal = false
);


//!
//! @description Return the centroidal coordinate frame. (Using current moment integrals --
//!    i.e. relative to whatever frame was usd.
//! @param [out] transform  frame with origin at centroid, axes aligned with principal directions.
//!
bool GetCentroidalFrame (TransformR frame);


//!
//! @description Return the moments in centroidal system.
//! @param [out] mass  mass
//! @param [out] centroid  center of gravity
//! @param [out] orientation  principal axes
//! @param [out] moments  moments (yy+zz, xx+zz, xx+yy) in principal frame
//!
bool GetPrincipalMoments
(
double&     mass,
DPoint3dR   centroid,
RotMatrixR  orientation,
DVec3dR     moments
);


//!
//! @description Add a triangle mass to the moments.
//! @param [in] density  mass per unit area (after transform)
//! @param [in] xyzA  start point of line
//! @param [in] xyzB  end point of line
//! @param [in] bTransformToLocal  true if the the coordinate
//!    is to be transformed from world to local.
//!
bool AddTriangleMass
(
double density,
DPoint3dCR xyzA,
DPoint3dCR xyzB,
DPoint3dCR xyzC,
bool    bTransformToLocal = false
);


//!
//! @description Convert scalar-form moments to principal axes and diagonalized moments.
//! @param [out] principalSecondMoments  the diagonal entries of the moment tensor in principal system.
//!            (integrated [yy+zz, zz+xx, xx+yy])
//! @param [out] principalDirections  matrix whose columns are the principal directions
//! @param [in] iXX  xx integral (NOT yy+zz from tensor !!!)
//! @param [in] iYY  yy integral (NOT zz+xx from tensor !!!)
//! @param [in] iZZ  zz integral (NOT xx+yy from tensor !!!)
//! @param [in] iXY  xy integral
//! @param [in] iXZ  xz integral
//! @param [in] iYZ  yz integral
//!
static void  PrincipalMomentsFromScalars
(
DVec3dR             principalSecondMoments,
RotMatrixR          principalDirections,
double              iXX,
double              iYY,
double              iZZ,
double              iXY,
double              iXZ,
double              iYZ
);


//! @description  Accumulate moments for the swept by a line from the moment system origin to a specified line.
//! @param [in] density  mass per unit area (after transform)
//! @param [in] segment line segment.
//! @param [in] bTransformToLocal  true if the the coordinates
//!    need to be transformed from world to local system.
bool AddSweptMass_LocalOriginToDSegment3d
(
double density,
DSegment3dCR segment,
bool    bTransformToLocal
);

//! @description Accumulate moments for the surface swept by a line from the moment system origin to an arc
//! @param [in] density  mass per unit area (after transform)
//! @param [in] arc circular or elliptic arc.
//! @param [in] bTransformToLocal  true if the the coordinates
//!    need to be transformed from world to local system.
bool AddSweptMass_LocalOriginToDEllipse3d
(
double density,
DEllipse3dCR arc,
bool    bTransformToLocal
);

//! @description  Accumulate moments for the swept by a line from the moment system origin to a bezier curve.
//! @param [in] density  mass per unit area (after transform)
//! @param [in] arc circular or elliptic arc.
//! @param [in] bTransformToLocal  true if the the coordinates
//!    need to be transformed from world to local system.
bool AddSweptMass_LocalOriginToBCurveSegment
(
double density,
BCurveSegmentCR bezier,
bool    bTransformToLocal
);

//! @description  Accumulate moments for the swept by a line from the moment system origin to a bspline curve
//! @param [in] density  mass per unit area (after transform)
//! @param [in] arc circular or elliptic arc.
//! @param [in] bTransformToLocal  true if the the coordinates
//!     need to be transformed from world to local system.
bool AddSweptMass_LocalOriginToMSBsplineCurve
(
double density,
MSBsplineCurveCR curve,
bool    bTransformToLocal
);

};

#endif
END_BENTLEY_GEOMETRY_NAMESPACE