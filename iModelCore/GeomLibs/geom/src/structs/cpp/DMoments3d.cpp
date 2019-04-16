/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*-----------------------------------------------------------------*//**
@description Constructor for a moment structure with all integrals zero in an identity frame.
+---------------+---------------+---------------+---------------+------*/
DMoments3d::DMoments3d
(
)
    {
    mWorldToLocal.initIdentity ();
    mLocalToWorld.initIdentity ();
    mMass = 0.0;
    mIx  = mIy  = mIz  = 0.0;
    mIxx = mIyy = mIzz = 0.0;
    mIxy = mIyz = mIzx = 0.0;
    mbWorldToLocalOK = true;
    }

/*-----------------------------------------------------------------*//**
@description Replace the coordinate frame.
Integrals values are NOT changed.
+---------------+---------------+---------------+---------------+------*/
bool DMoments3d::SetFrame
(
TransformCR localToWorld
)
    {
    mLocalToWorld = localToWorld;
    mbWorldToLocalOK = mWorldToLocal.inverseOf (&mLocalToWorld) ? true : false;
    return mbWorldToLocalOK;
    }

/*-----------------------------------------------------------------*//**
@description Return a new moment structure whose integral values are
    the (scaled) sum of two inputs.
    Frame data is copied directly from momentA.
    There is no test for compatible frames.
@param [in] momentA  first set of moments.
@param [in] scaleA  scale factor for momentA
@param [in] momentB  second set of moments
@param [in] scaleB  scale factor for momentB
+---------------+---------------+---------------+---------------+------*/
DMoments3d DMoments3d::FromSumOf
(
DMoments3dCR momentA,
double scaleA,
DMoments3dCR momentB,
double scaleB
)
    {
    DMoments3d sum = DMoments3d ();
    sum.mWorldToLocal = momentA.mWorldToLocal;
    sum.mLocalToWorld = momentA.mLocalToWorld;
    sum.mbWorldToLocalOK = momentA.mbWorldToLocalOK;
    sum.mMass = momentA.mMass * scaleA +  momentB.mMass * scaleB;

    sum.mIx = momentA.mIx * scaleA +  momentB.mIx * scaleB;
    sum.mIy = momentA.mIy * scaleA +  momentB.mIy * scaleB;
    sum.mIz = momentA.mIz * scaleA +  momentB.mIz * scaleB;

    sum.mIxx = momentA.mIxx * scaleA +  momentB.mIxx * scaleB;
    sum.mIyy = momentA.mIyy * scaleA +  momentB.mIyy * scaleB;
    sum.mIzz = momentA.mIzz * scaleA +  momentB.mIzz * scaleB;

    sum.mIxy = momentA.mIxy * scaleA +  momentB.mIxy * scaleB;
    sum.mIyz = momentA.mIyz * scaleA +  momentB.mIyz * scaleB;
    sum.mIzx = momentA.mIzx * scaleA +  momentB.mIzx * scaleB;
    return sum;
    }

/*-----------------------------------------------------------------*//**
@description Return all integrals in tensor form.
@param [out]localToWorld  the coordinate frame in which the integrals were evaluated.
@param [out]worldToLocal  the inverse frame.
@param [out]bWorldToLocalOK  flag confirming that the coordinate frame is nonsingular.
@param [out]mass  the mass of the underlying geometry.
@param [out]moment1Vector  vector of moments (x dm, y dm, z dm)
@param [out]moment2Tensor  matrix with moments -- ij entry is integral of (deltaij r^2 - xi xj)dm
+---------------+---------------+---------------+---------------+------*/
void DMoments3d::GetTensor
(
TransformR localToWorld,
TransformR worldToLocal,
bool       &bWorldToLocalOK,
double     &mass,
DVec3dR    moment1Vector,
RotMatrixR moment2Tensor
)
    {
    localToWorld = mLocalToWorld;
    worldToLocal = mWorldToLocal;

    mass = mMass;

    moment1Vector.x = mIx;
    moment1Vector.y = mIy;
    moment1Vector.z = mIz;

    //double mxx = mIyy + mIzz;
    //double myy = mIzz + mIxx;
    //double mzz = mIxx + mIyy;
    moment2Tensor.initFromRowValues
            (
            mIyy + mIzz, -mIxy,        -mIzx,
            -mIxy,        mIzz + mIxx, -mIyz,
            -mIzx,       -mIyz,        mIxx + mIyy
            );
    }

/*-----------------------------------------------------------------*//**
@description Return all scalar integrals.
@param [out]localToWorld  the coordinate frame in which the integrals were evaluated.
@param [out]worldToLocal  the inverse frame.
@param [out]bWorldToLocalOK  flag confirming that the coordinate frame is nonsingular.
@param [out]mass  the mass of the underlying geometry.
@param [out]Ix  integral of (x dV)
@param [out]Iy  integral of (y dV)
@param [out]Iz  integral of (z dV)

@param [out]Ixx  integral of (x^2 dV)
@param [out]Iyy  integral of (y^2 dV)
@param [out]Izz  integral of (z^2 dV)

@param [out]Ixy  integral of (x y dV)
@param [out]Iyz  integral of (y z dV)
@param [out]Izx  integral of (z x dV)
+---------------+---------------+---------------+---------------+------*/
void DMoments3d::GetScalars
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
)
    {
    localToWorld = mLocalToWorld;
    worldToLocal = mWorldToLocal;

    mass = mMass;

    Ix = mIx;
    Iy = mIy;
    Iz = mIz;

    Ixx = mIxx;
    Iyy = mIyy;
    Izz = mIzz;

    Ixy = mIxy;
    Iyz = mIyz;
    Izx = mIzx;
    }


/*-----------------------------------------------------------------*//**
@description Return a new moment structure with all moment integrals relative to a specified frame.
      (in effect, transform the geometry from old local to global, then to the new local)
@remark Be sure you understand the difference between
        CopyWithTransformAppliedToGeoemtry
        and CopyWithTransformToNewFrame.
@param [in] newLocaltoWorld  new coordinate frame.
+---------------+---------------+---------------+---------------+------*/
DMoments3d DMoments3d::CopyWithTransformToNewFrame
(
TransformCR newLocalToWorld
)
    {
    Transform newWorldToLocal;
    newWorldToLocal.inverseOf (&newLocalToWorld);
    Transform oldToNew;
    oldToNew.productOf (&newWorldToLocal, &mLocalToWorld);
    return CopyWithTransformAppliedToFrame (oldToNew);
    }

/*-----------------------------------------------------------------*//**
@description Return the centroidal coordinate frame. (Using current moment integrals --
    i.e. relative to whatever frame was usd.
@param [out]transform  frame with origin at centroid, axes aligned with principal directions.
+---------------+---------------+---------------+---------------+------*/
bool DMoments3d::GetCentroidalFrame
(
TransformR frame
)
    {
    double mass;
    DPoint3d centroid;
    RotMatrix orientation;
    DVec3d moments;
    frame.initIdentity ();
    if (GetPrincipalMoments (mass, centroid, orientation, moments))
        {
        frame.initFrom (&orientation, &centroid);
        return true;
        }
    return false;
    }

/*-----------------------------------------------------------------*//**
@description Return the moments in centroidal system.
@param [out]mass  mass
@param [out]centroid  center of gravity
@param [out]orientation  principal axes
@param [out]moments  moments (yy+zz, xx+zz, xx+yy) in principal frame
+---------------+---------------+---------------+---------------+------*/
bool DMoments3d::GetPrincipalMoments
(
double&     mass,
DPoint3dR   centroid,
RotMatrixR  orientation,
DVec3dR     moments
)
    {
    mass = mMass;
    centroid.zero ();
    moments.zero ();
    orientation.initIdentity ();
    if (   bsiTrig_safeDivide (&centroid.x, mIx, mMass, 0.0)
        && bsiTrig_safeDivide (&centroid.y, mIy, mMass, 0.0)
        && bsiTrig_safeDivide (&centroid.z, mIz, mMass, 0.0)
        )
        {
        DMoments3d centroidalMoments;
        Transform centroidalTranslation;
        DVec3d shift;
        shift.negate ((DVec3d*)&centroid);
        centroidalTranslation.initFrom (&shift);
        centroidalMoments = CopyWithTransformAppliedToFrame (centroidalTranslation);
        PrincipalMomentsFromScalars (moments, orientation,
                    centroidalMoments.mIxx, centroidalMoments.mIyy, centroidalMoments.mIzz,
                    centroidalMoments.mIxy, centroidalMoments.mIzx, centroidalMoments.mIyz);
        return true;
        }
    return false;
    }

/*-----------------------------------------------------------------*//**
@description Convert scalar-form moments to principal axes and diagonalized moments.
@param [out]principalSecondMoments  the diagonal entries of the moment tensor in principal system.
            (integrated [yy+zz, zz+xx, xx+yy])
@param [out]principalDirections  matrix whose columns are the principal directions
@param [in] iXX  xx integral (NOT yy+zz from tensor !!!)
@param [in] iYY  yy integral (NOT zz+xx from tensor !!!)
@param [in] iZZ  zz integral (NOT xx+yy from tensor !!!)
@param [in] iXY  xy integral
@param [in] iXZ  xz integral
@param [in] iYZ  yz integral
+---------------+---------------+---------------+---------------+------*/
void  DMoments3d::PrincipalMomentsFromScalars
(
DVec3dR             principalSecondMoments,
RotMatrixR          principalDirections,
double              iXX,
double              iYY,
double              iZZ,
double              iXY,
double              iXZ,
double              iYZ
)
    {
    int             nRot;
    double          a[3][3];;

    /*  --  define interia tensor as matrix to diagonalize -- */
    a[0][0] =  iYY + iZZ;  a[0][1] = -iXY;        a[0][2] = -iXZ;
    a[1][0] = -iXY;        a[1][1] =  iZZ + iXX;  a[1][2] = -iYZ;
    a[2][0] = -iXZ;        a[2][1] = -iYZ;        a[2][2] =  iXX + iYY;

    bsiGeom_jacobi3X3 ((double*)&principalSecondMoments, principalDirections.form3d, &nRot, a);
    }

/*-----------------------------------------------------------------*//**
@description Return a new moment structure with all integrals
    reflecting a transform applied to the coordinates used in the integrations.
    (i.e. same effect as a local-to-local transform applyed pointwise to the body,
     and integrating again in local coordinates.)
@remark Be sure you understand the difference between
        CopyWithTransformAppliedToFrame
        and CopyWithTransformToNewFrame.
@param [in] transform  transform applied to the underlying geometry.
+---------------+---------------+---------------+---------------+------*/
DMoments3d DMoments3d::CopyWithTransformAppliedToFrame
(
TransformCR transform
)
    {
    // Here are the formulas.
    // These will make no sense if you haven't fought with outer products.
    //    (In short:  A "vector" is a column.  A vector transpose is a row.
    //       A column times a row is a square matrix.  (That's the outer product))
    // matrix_A is the matrix part of the transform
    // vector_R is the translation part of the transform
    // intermediate vector_A_R  is matrixA times vector_R
    // I1, J1 are the vectors of degree one 1 integrals (x dV, y dV, z dV) in old and new frames
    // I2, J2 are the the symmetric matrices of degree two product integrals (x^2 dV, xy dV, etc) in new frame.

    // The degree one integrals are I1 = integral(X,dV) where X is the vector of x,y,z integrands
    // We want to know J1 = integral (A*X+R,dV) = A*integral(X,dV) + R*integral (1, dV)
    // The degree two integrals are I2 = integral (X XT, dV)
    // We want to know J2 = integral ((A*X+R)*(A*X+R)T, dV)
    //                    = integral (A*X*XT*AT + A*X*RT + R*(A*X)T + R*RT, dV)
    //                    =  A*I2*AT + A*I1*RT + R*(A*I1)T + R*RT*integral (1,dV)
    // (.... and don't forget that dV is scaled by detA. .....)

    RotMatrix matrix_AT, matrix_A;
    DVec3d vector_R;

    transform.getMatrix (&matrix_A);
    double detA = matrix_A.determinant ();

    matrix_AT.transposeOf (&matrix_A);
    transform.getTranslation (&vector_R);

    DVec3d vector_I1, vector_J1, vector_A_I1;
    vector_I1.init (mIx, mIy, mIz);
    vector_A_I1.multiply (&matrix_A, &vector_I1);
    vector_J1.sumOf (&vector_A_I1, &vector_R, mMass);

    RotMatrix matrix_J2, matrix_I2;
    matrix_I2.initFromRowValues
                (
                mIxx, mIxy, mIzx,
                mIxy, mIyy, mIyz,
                mIzx, mIyz, mIzz
                );

    // Two steps to form J2 = A*I2*AT:

    matrix_J2.productOf (&matrix_I2, &matrix_AT);
    matrix_J2.productOf (&matrix_A,  &matrix_J2);

    matrix_J2.addScaledOuterProductInPlace (&vector_A_I1, &vector_R, 1.0);
    matrix_J2.addScaledOuterProductInPlace (&vector_R, &vector_A_I1, 1.0);
    matrix_J2.addScaledOuterProductInPlace (&vector_R, &vector_R, mMass);

    // dV correction by determinant ....
    vector_J1.scale (detA);
    matrix_J2.scaleColumns (&matrix_J2, detA, detA, detA);
    double mass2 = mMass * detA;

    Transform newLocalToWorld;
    Transform inverse;
    inverse.inverseOf (&transform);
    newLocalToWorld.productOf (&transform, &mLocalToWorld);
    DMoments3d result = DMoments3d::FromScalars
            (
            newLocalToWorld,
            mass2,
            vector_J1.x, vector_J1.y, vector_J1.z,
            matrix_J2.form3d[0][0], matrix_J2.form3d[1][1], matrix_J2.form3d[2][2],
            matrix_J2.form3d[0][1], matrix_J2.form3d[1][2], matrix_J2.form3d[2][0]
            );
    return result;
    }

/*-----------------------------------------------------------------*//**
@description Return a moment structure initialized with scalar values of
    each integral.
@param [in] localToWorld  the coordinate frame in which the integrals were evaluated.
@param [in] mass  the mass of the underlying geometry.
@param [in] Ix  integral of (x dV)
@param [in] Iy  integral of (y dV)
@param [in] Iz  integral of (z dV)

@param [in] Ixx  integral of (x^2 dV)
@param [in] Iyy  integral of (y^2 dV)
@param [in] Izz  integral of (z^2 dV)

@param [in] Ixy  integral of (x y dV)
@param [in] Iyz  integral of (y z dV)
@param [in] Izx  integral of (z x dV)
+---------------+---------------+---------------+---------------+------*/
DMoments3d DMoments3d::FromScalars
(
TransformCR localToWorld,
double mass,
double Ix,
double Iy,
double Iz,
double Ixx,
double Iyy,
double Izz,
double Ixy,
double Iyz,
double Izx
)
    {
    DMoments3d result;
    result.mLocalToWorld = localToWorld;
    result.mbWorldToLocalOK = result.mWorldToLocal.inverseOf (&localToWorld) ? true : false;
    result.mMass = mass;
    result.mIx = Ix;
    result.mIy = Iy;
    result.mIz = Iz;

    result.mIxx = Ixx;
    result.mIyy = Iyy;
    result.mIzz = Izz;

    result.mIxy = Ixy;
    result.mIyz = Iyz;
    result.mIzx = Izx;
    return result;
    }

/*-----------------------------------------------------------------*//**
@description Add a concentrated mass to the moments.
@param [in] mass  mass
@param [in] xyz  point where mass is concentrated
@param [in] bTransformToLocal  true if the the coordinate
    is to be transformed from world to local.
+---------------+---------------+---------------+---------------+------*/
bool DMoments3d::AddConcentratedMass
(
double mass,
DPoint3dCR xyz,
bool    bTransformToLocal
)
    {
    DPoint3d uvw = xyz;
    if (bTransformToLocal)
        {
        if (!mbWorldToLocalOK)
            return false;
        mWorldToLocal.multiply (&uvw, &xyz);
        }
    mMass += mass;
    mIx += mass * uvw.x;
    mIy += mass * uvw.y;
    mIz += mass * uvw.z;

    mIxx += mass * uvw.x * uvw.x;
    mIyy += mass * uvw.y * uvw.y;
    mIzz += mass * uvw.z * uvw.z;

    mIxy += mass * uvw.x * uvw.y;
    mIyz += mass * uvw.y * uvw.z;
    mIzx += mass * uvw.z * uvw.x;

    return true;
    }

/*-----------------------------------------------------------------*//**
@description Add a scaled outer product to the second moments.
+---------------+---------------+---------------+---------------+------*/
void DMoments3d::AddScaledOuterProduct
(
DPoint3dCR vectorU,
DPoint3dCR vectorV,
double   a
)
    {
    mIxx += a * vectorU.x * vectorV.x;
    mIyy += a * vectorU.y * vectorV.y;
    mIzz += a * vectorU.z * vectorV.z;

    mIxy += a * vectorU.x * vectorV.y;
    mIyz += a * vectorU.y * vectorV.z;
    mIzx += a * vectorU.z * vectorV.x;
    }

/*-----------------------------------------------------------------*//**
@description Add a wire mass to the moments for a line segment.
@param [in] density  mass per unit length (after transform)
@param [in] xyzA  start point of line
@param [in] xyzB  end point of line
@param [in] bTransformToLocal  true if the the coordinate
    is to be transformed from world to local.
+---------------+---------------+---------------+---------------+------*/
bool DMoments3d::AddWireMass
(
double density,
DPoint3dCR xyzA,
DPoint3dCR xyzB,
bool    bTransformToLocal
)
    {
    DPoint3d uvwA = xyzA;
    DPoint3d uvwB = xyzB;
    DVec3d   vectorAB;
    if (bTransformToLocal)
        {
        if (!mbWorldToLocalOK)
            return false;
        mWorldToLocal.multiply (&uvwA, &xyzA);
        mWorldToLocal.multiply (&uvwB, &xyzB);
        }
    vectorAB.differenceOf (&uvwB, &uvwA);
    double mass = density * vectorAB.magnitude ();
    DPoint3d uvwMid;
    uvwMid.interpolate (&uvwA, 0.5, &uvwB);

    mMass += mass;

    mIx += mass * uvwMid.x;
    mIy += mass * uvwMid.y;
    mIz += mass * uvwMid.z;

    AddScaledOuterProduct (uvwA, uvwA, mass);
    AddScaledOuterProduct (uvwA, vectorAB, mass * 0.5);
    AddScaledOuterProduct (vectorAB, uvwA, mass * 0.5);
    AddScaledOuterProduct (vectorAB, vectorAB, mass / 3.0);
    return true;
    }

/*-----------------------------------------------------------------*//**
@description Add a triangle mass to the moments.
@param [in] density  mass per unit area (after transform)
@param [in] xyzA  first triangle point
@param [in] xyzB  second triangle point
@param [in] xyzC  third triangle point
@param [in] bTransformToLocal  true if the the coordinate
    is to be transformed from world to local.
+---------------+---------------+---------------+---------------+------*/
bool DMoments3d::AddTriangleMass
(
double density,
DPoint3dCR xyzA,
DPoint3dCR xyzB,
DPoint3dCR xyzC,
bool    bTransformToLocal
)
    {
    DPoint3d uvwA = xyzA;
    DPoint3d uvwB = xyzB;
    DPoint3d uvwC = xyzC;
    DVec3d   vectorAB, vectorAC, cross;
    if (bTransformToLocal)
        {
        if (!mbWorldToLocalOK)
            return false;
        mWorldToLocal.multiply (&uvwA, &xyzA);
        mWorldToLocal.multiply (&uvwB, &xyzB);
        mWorldToLocal.multiply (&uvwC, &xyzC);
        }
    vectorAB.differenceOf (&uvwB, &uvwA);
    vectorAC.differenceOf (&uvwC, &uvwA);
    cross.crossProduct (&vectorAB, &vectorAC);
    double b = cross.magnitude ();
    double mass = density * b * 0.5;
    DPoint3d uvwCentroid;
    double a = 1.0 / 3.0;
    uvwCentroid.sumOf (&uvwA, &vectorAB, a, &vectorAC, a);

    mMass += mass;

    mIx += mass * uvwCentroid.x;
    mIy += mass * uvwCentroid.y;
    mIz += mass * uvwCentroid.z;

    double c = density * b / 24;
    AddScaledOuterProduct (uvwA, uvwA, c * 12.0);
    AddScaledOuterProduct (uvwA, vectorAB, c * 4.0);
    AddScaledOuterProduct (uvwA, vectorAC, c * 4.0);

    AddScaledOuterProduct (vectorAB, uvwA, c * 4.0);
    AddScaledOuterProduct (vectorAB, vectorAB, c * 2.0);
    AddScaledOuterProduct (vectorAB, vectorAC, c);

    AddScaledOuterProduct (vectorAC, uvwA, c * 4.0);
    AddScaledOuterProduct (vectorAC, vectorAB, c);
    AddScaledOuterProduct (vectorAC, vectorAC, c * 2.0);

    return true;
    }




bool DMoments3d::AddSweptMass_LocalOriginToDSegment3d
(
double density,
DSegment3dCR segment,
bool    bTransformToLocal
)
    {
    return false;
    }





bool DMoments3d::AddSweptMass_LocalOriginToDEllipse3d
(
double density,
DEllipse3dCR arc,
bool    bTransformToLocal
)
    {
    return false;
    }






bool DMoments3d::AddSweptMass_LocalOriginToBCurveSegment
(
double density,
BCurveSegmentCR bezier,
bool    bTransformToLocal
)
    {
    return false;
    }





bool DMoments3d::AddSweptMass_LocalOriginToMSBsplineCurve
(
double density,
MSBsplineCurveCR curve,
bool    bTransformToLocal
)
    {
    return false;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
