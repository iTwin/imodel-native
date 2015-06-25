/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/VecMathDGNTolerances.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/VecMath.h>
#include <Geom/internal/dsegment3d.fdf>
/*---------------------------------------------------------------------------------**//**
* see Quaternion Interpolation with extra spins (Jack Morrison, Graphic Gems III)
* @bsimethod                                                    RayBentley      11/94
+---------------+---------------+---------------+---------------+---------------+------*/
Public void LegacyMath::LinearInterpolateQuaternion
(
double          outQuat[4],
double const    p[4],
double const    q[4],
double          t,
int             spin
)
    {
    int         i, bFlip;
    double      alpha = t, beta, theta, sinT, cosT, phi;

    if (false != (bFlip = (cosT = p[0] * q[0] + p[1] * q[1] + p[2] * q[2] + p[3] * q[3]) < 0.0))
        cosT = -cosT;

    if (1 - cosT < mgds_fc_epsilon)
        {
        beta = 1.0 - alpha;
        }
    else
        {
        theta   = Angle::Acos (cosT);
        phi     = theta + spin * msGeomConst_pi;
        sinT    = sin (theta);
        beta    = sin (theta - alpha * phi) / sinT;
        alpha   = sin (alpha * phi) / sinT;
        }

    if (bFlip)
        alpha = -alpha;

    for (i=0; i<4; i++)
        outQuat[i] = beta * p[i] + alpha * q[i];
    }

/*---------------------------------------------------------------------------------**//**
* LegacyMath::RMatrix::multiplyRange - rotates a range cube
* @bsimethod                                                    EDL             10/01
+---------------+---------------+---------------+---------------+---------------+------*/
void LegacyMath::RMatrix::MultiplyRange
(
DPoint3dP low,
DPoint3dP high,
RotMatrixCP rotMatrixP
)
    {
    DPoint3d    corner[8];
    DRange3d    range;
    if (!rotMatrixP)
        return;

    range.low = *low;
    range.high = *high;
    bsiDRange3d_box2Points (&range, corner);
    rotMatrixP->Multiply (corner, corner, 8);
    bsiDRange3d_initFromArray (&range, corner, 8);
    *low = range.low;
    *high = range.high;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/00
+---------------+---------------+---------------+---------------+---------------+------*/
void LegacyMath::RMatrix::FromNormalVector (RotMatrixP rotMatrixP, DPoint3dCP normalP)
    {
    DVec3d    world, xNormal, yNormal, zNormal;

    world.x = world.y = world.z = 0.0;
    zNormal = *(DVec3dP)normalP;
    zNormal.Normalize ();

    if ((fabs (zNormal.x) < 0.01) && (fabs (zNormal.y) < 0.01))
        world.y = 1.0;
    else
        world.z = 1.0;

    xNormal.CrossProduct (world, zNormal);
    xNormal.Normalize ();
    yNormal.CrossProduct (zNormal, xNormal);
    yNormal.Normalize ();
    rotMatrixP->InitFromRowVectors (xNormal, yNormal, zNormal);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/86
+---------------+---------------+---------------+---------------+---------------+------*/
void LegacyMath::TMatrix::TransformArc
(
double              *major,         /* <=> major axis */
double              *minor,         /* <=> minor axis */
RotMatrix           *atran,         /* <=> arc transformation matrix */
double              *rot,
double              *start,
double              *sweep,
Transform const*    trans,
bool                threeD
)
    {
    // Optimized to minimize calculations  if square portion of the transform is pure scale.
    if (trans->form3d[0][1] == 0.0 &&
        trans->form3d[0][2] == 0.0 &&
        trans->form3d[1][0] == 0.0 &&
        trans->form3d[1][2] == 0.0 &&
        trans->form3d[2][0] == 0.0 &&
        trans->form3d[2][1] == 0.0 &&
        trans->form3d[0][0] == trans->form3d[1][1] &&
        trans->form3d[0][0] == trans->form3d[2][2])
        {
        *major *= trans->form3d[0][0];
        *minor *= trans->form3d[0][0];
        }
    else
        {
        double      t0, xMagnitude, yMagnitude, dotProduct;
        DVec3d      xVec, yVec, zVec;
        RotMatrix   arcRM;

        /* Scale columns of the ellipse system */
        arcRM.InitIdentity ();
        arcRM.ScaleRows (arcRM, *major, *minor, 1.0);
        arcRM.InitProduct(*atran, arcRM);
        arcRM.InitProduct(*trans, arcRM);

        /* Transformation alters position of extremal axes in angular space. */
        arcRM.GetColumn(xVec,  0);
        arcRM.GetColumn(yVec,  1);
        xMagnitude = xVec.Normalize ();
        yMagnitude = yVec.Normalize ();
        dotProduct = xVec.DotProduct (yVec);

        if (xMagnitude < mgds_fc_epsilon || yMagnitude < mgds_fc_epsilon || fabs (dotProduct) < 1.0E-8)
            t0 = 0.0;
        else
            t0 = atan2 (2.0 * dotProduct, xMagnitude/yMagnitude - yMagnitude/xMagnitude) / 2.0;

        *start -= t0;
        xVec.x = cos(t0);
        xVec.y = sin(t0);
        xVec.z = 0;

        t0 += msGeomConst_piOver2;
        yVec.x = cos(t0);
        yVec.y = sin(t0);
        yVec.z = 0;

        arcRM.Multiply(xVec);
        arcRM.Multiply(yVec);

        *major = xVec.Magnitude ();
        *minor = yVec.Magnitude ();

        if (threeD)
            {
            if (LegacyMath::DEqual (*major, 0.0))
                {
                if (LegacyMath::DEqual (*minor, 0.0))
                    {
                    atran->InitIdentity ();
                    }
                else
                    {
                    LegacyMath::RMatrix::FromYVector (atran, &yVec);
                    atran->InverseOf(*atran);
                    }
                }
            else if (LegacyMath::DEqual (*minor, 0.0))
                {
                LegacyMath::RMatrix::FromXVector (atran, &xVec);
                atran->InverseOf(*atran);
                }
            else
                {
                xVec.Normalize ();
                zVec.CrossProduct (xVec, yVec);

                zVec.Normalize ();
                yVec.CrossProduct (zVec, xVec);
                yVec.Normalize ();
                atran->InitFromColumnVectors(xVec, yVec, zVec);
                }
            }
        else
            {
            *rot =  Angle::Atan2 (xVec.y, xVec.x);

            if (trans->form3d[0][0]*trans->form3d[1][1] < trans->form3d[1][0]*trans->form3d[0][1])
                {
                *start *= (-1.0);
                *sweep *= (-1.0);
                }

            atran->InitFromAxisAndRotationAngle(2,  *rot);
            }
        }
    }

/*--------------------------------------------------------------------*//**
@description Concatenate transformations in a form useful for text placement.
The overall form is
<pre>
    OUT = TRANSFORM * TRANSLATE * MATRIX * SLANT * SCALE
</pre>
where
<UL>
<li>TRANSFORM is a initial transformation, e.g an overall coordinate system</li>
<li>TRANSLATE is a transform with given origin as its translation part, otherwise identity.</li>
<li>MATRIX is a transform with given RotMatrix as matrix part, no translation.  This is typically
    (but not necessarily) a pure rotation of the text baseline and plane.</li>
<li>SLANT is a transform which slants the y axis towards by given angle towards x</li>
<li>SCALE applies x and y scaling.  Typically the two scale factors are x, y sizes of a character.</li>
</UL>
@param pOutTrans OUT result transformation
@param pInTrans IN base coordinate system
@param pRotMatrix IN character orientation.
@param pOrigin IN origin of local coordinates (e.g. corner of character
@param xScale IN x scale factor (character size)
@param yScale IN y scale factor (character size)
@param slantRadians IN slant angle
@bsimethod                      EarlinLutz          02/04
+------------------------------------------------------------------------*/
void LegacyMath::TMatrix::ComposeOrientationOriginScaleXYShear
(
TransformP pOutTrans,
TransformCP pInTrans,
RotMatrixCP pRotMatrix,
DPoint3dCP pOrigin,
double              xScale,
double              yScale,
double              slantRadians
)
    {
    Transform result;
    double xShear;

    result.InitIdentity ();

    if ((slantRadians < (msGeomConst_piOver2 - .01)) &&
        (slantRadians > (-msGeomConst_piOver2 + .01)))
        xShear = tan (slantRadians);
    else
        xShear = 0.0;

    result.form3d[0][0] = xScale;
    result.form3d[1][1] = yScale;
    result.form3d[0][1] = xShear * yScale;

    if (pRotMatrix)
        result.InitProduct (*pRotMatrix, result);

    if (pOrigin)
        result.SetTranslation (*pOrigin);

    if (pInTrans)
        result.InitProduct (*pInTrans, result);

    if (pOutTrans)
        *pOutTrans = result;
    }

/*--------------------------------------------------------------------*//**
@description Concatenate transformations in a form useful for text placement.
The overall form is
<pre>
    OUT = TRANSFORM * TRANSLATE * MATRIX * SCALE
</pre>
where
<UL>
<li>TRANSFORM is a initial transformation, e.g an overall coordinate system</li>
<li>TRANSLATE is a transform with given origin as its translation part, otherwise identity.</li>
<li>MATRIX is a transform with given RotMatrix as matrix part, no translation.  This is typically
    (but not necessarily) a pure rotation of the text baseline and plane.</li>
<li>SCALE applies x and y scaling.  Typically the two scale factors are x, y sizes of a character.</li>
</UL>
@param pOutTrans OUT result transformation
@param pInTrans IN base coordinate system
@param pRotMatrix IN character orientation.
@param pOrigin IN origin of local coordinates (e.g. corner of character
@param xScale IN x scale factor (character size)
@param yScale IN y scale factor (character size)
@remarks This is equivalent to LegacyMath::TMatrix::composeOrientationOriginSlantScaleXY with zero slant angle.
@bsimethod                      EarlinLutz          02/04
+------------------------------------------------------------------------*/
void LegacyMath::TMatrix::ComposeOrientationOriginScaleXY
(
TransformP pOutTrans,
TransformCP pInTrans,
RotMatrixCP pRotMatrix,
DPoint3dCP pOrigin,
double              xScale,
double              yScale
)
    {
    LegacyMath::TMatrix::ComposeOrientationOriginScaleXYShear (pOutTrans, pInTrans, pRotMatrix, pOrigin, xScale, yScale, 0.0);
    }

/*---------------------------------------------------------------------------------**//**
* Transform = X(p2) . ( R2 . Inverse (R1) ) . X(-p1)
* compose a transformation matrix which transforms coordinate system centered @ p1 into a system centered @ p2.
* The transformation pipeline is: () translate from p1 -> (0,0,0) () un-rotate object () rotate object to new orientation () translate rotated
* object -> p2
* all inputs are optional p2 may be same as p1
* @bsimethod                                                    Sam.Wilson      04/92
+---------------+---------------+---------------+---------------+---------------+------*/
TransformP LegacyMath::TMatrix::ComposeLocalOriginOperations
(                                       /* <= pTransform */
TransformP pTransform,         /* <=  */
DPoint3dCP pP2,                /*  => target location or NULL */
RotMatrixCP pR2,                /*  => target rotation or NULL */
RotMatrixCP pR1,                /*  => current rotation or NULL */
DPoint3dCP pP1                 /*  => current location or NULL */
)
    {
    bsiTransform_composeLocalOriginOperations (pTransform, pP2, pR2, NULL, pR1, pP1);
    return pTransform;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/00
+---------------+---------------+---------------+---------------+---------------+------*/
int LegacyMath::RMatrix::FromYVector (RotMatrixP rotMatrixP, DPoint3dCP yNormalP)
    {
    DVec3d    world, xNormal, yNormal, zNormal;

    world.x = world.y = world.z = 0.0;
    yNormal = *(DVec3dP)yNormalP;

    yNormal.Normalize ();

    if ((fabs (yNormal.x) < 0.01) && (fabs (yNormal.y) < 0.01))
        world.y = 1.0;
    else
        world.z = 1.0;

    xNormal.CrossProduct (yNormal, world);
    xNormal.Normalize ();
    zNormal.CrossProduct (xNormal, yNormal);
    zNormal.Normalize ();
    rotMatrixP->InitFromRowVectors (xNormal, yNormal, zNormal);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/00
+---------------+---------------+---------------+---------------+---------------+------*/
int LegacyMath::RMatrix::FromXVector (RotMatrixP rotMatrixP, DPoint3dCP xNormalP)
    {
    DVec3d    world, xNormal, yNormal, zNormal;

    world.x = world.y = world.z = 0.0;
    xNormal = *(DVec3dP)xNormalP;

    xNormal.Normalize ();

    if ((fabs (xNormal.x) < 0.01) && (fabs (xNormal.y) < 0.01))
        world.y = 1.0;
    else
        world.z = 1.0;

    yNormal.CrossProduct (world, xNormal);
    yNormal.Normalize ();
    zNormal.CrossProduct (xNormal, yNormal);
    zNormal.Normalize ();
    rotMatrixP->InitFromRowVectors (xNormal, yNormal, zNormal);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    kab             01/90
+---------------+---------------+---------------+---------------+---------------+------*/
void LegacyMath::TMatrix::SetMatrixColumn (TransformP transformP, DPoint3dCP vectorP, int columnIndex)
    {
    if (vectorP)
        {
        columnIndex = Angle::Cyclic3dAxis (columnIndex);
        transformP->form3d[0][columnIndex] = vectorP->x;
        transformP->form3d[1][columnIndex] = vectorP->y;
        transformP->form3d[2][columnIndex] = vectorP->z;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    kab             01/90
+---------------+---------------+---------------+---------------+---------------+------*/
void LegacyMath::TMatrix::SetMatrixRow (TransformP transformP, DPoint3dCP vectorP, int rowIndex)
    {
    if (vectorP)
        {
        rowIndex = Angle::Cyclic3dAxis (rowIndex);
        transformP->form3d[rowIndex][0] = vectorP->x;
        transformP->form3d[rowIndex][1] = vectorP->y;
        transformP->form3d[rowIndex][2] = vectorP->z;
        }
    }

/*
This function normalizes the ROWS of the matrix part a transform.  The normalized rows
are returned as a the matrix part of a new transform, and the original lengths are
returned as xyz parts of a point.

This is a bogus operation.  It is only meaningful in the special case where the
matrix contains only rotation and uniform scale.  In this case, the three scales
are identical and all match the uniform scale.

Various caller in mscore extract the scale factors and use either (a) just the x scale,
(b) an average of all three, or (c) an average of the x and y scales as a representative
scale.

From inspection of callers Feb. 18, 2004:
A) Only one caller (mdlTransformCellMatrix) asks for output matrix (the normalized vectors)
    This function then multiplies the normalized matrix with a caller-supplied matrix.
    A quick inspection says that the half-dozen or so callers of mdlTransformCellMatrix
    all pass a transform off the currTrans stack.   There are many other reasons to believe
    that currTrans does NOT have nonuniform scales.
B) All other callers ask for scale but not transform. They then reduce to a single scale factor
    by one of several means:
        a) just use x scale
        b) average all three
        c) if all three are nonzero, average them, otherwise average x and y
Note that under UNIFORM SCALE averaging all three equal values just returns the equal values.
If the transform has uniform scale/rotate in xy submatrix, 0's in z off diagonals, and some other scale
(probably 0 or 1) in the Z place, the average of (equal) x and y scales gives the xy scale back.
*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    kab             09/90
+---------------+---------------+---------------+---------------+---------------+------*/
void LegacyMath::TMatrix::FromNormalizedRowsOfTMatrix
(
TransformP normalizedTMatrix,     /* <= normalized tmatrix */
DPoint3dP scaleVector,           /* <= scale bvector */
TransformCP in                     /* => rotation/scaling matrix */
)
    {
    DVec3d    xRow, yRow, zRow, mag;

    in->GetMatrixRow (xRow, 0);
    mag.x = xRow.Magnitude ();
    in->GetMatrixRow (yRow, 1);
    mag.y = yRow.Magnitude ();
    in->GetMatrixRow (zRow, 2);
    mag.z = zRow.Magnitude ();

    if (scaleVector)
        *scaleVector = mag;

    if (normalizedTMatrix)
        {
        if (mag.x > 0.0)
            xRow.Scale (xRow, 1.0/mag.x);
        if (mag.y > 0.0)
            yRow.Scale (yRow, 1.0/mag.y);
        if (mag.z > 0.0)
            zRow.Scale (zRow, 1.0/mag.z);

        memset (normalizedTMatrix, 0, sizeof (Transform));
        LegacyMath::TMatrix::SetMatrixRow (normalizedTMatrix, &xRow, 0);
        LegacyMath::TMatrix::SetMatrixRow (normalizedTMatrix, &yRow, 1);
        LegacyMath::TMatrix::SetMatrixRow (normalizedTMatrix, &zRow, 2);
        }
    }

/*---------------------------------------------------------------------------------**//**
* ctr for identity transform
* @bsimethod                                                    Keith.Bentley   12/03
+---------------+---------------+---------------+---------------+---------------+------*/
TransformInfo::TransformInfo ()
    {
    m_trans.InitIdentity ();
    m_haveMirrorPlane       = false;
    }

/*---------------------------------------------------------------------------------**//**
* ctr for general transformation matrix, must analyze the transform to determine its "type"
* @bsimethod                                                    Keith.Bentley   12/03
+---------------+---------------+---------------+---------------+---------------+------*/
TransformInfo::TransformInfo (TransformCR trans)
    {
    m_trans                 = trans;
    m_haveMirrorPlane       = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             08/86
+---------------+---------------+---------------+---------------+---------------+------*/
bool LegacyMath::DUorEqual (double r1, double r2)
    {
    return (fabs (r1-r2) < .25);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             08/86
+---------------+---------------+---------------+---------------+---------------+------*/
bool LegacyMath::RpntEqual (DPoint3dCP pt1, DPoint3dCP pt2)
    {
    return (LegacyMath::DUorEqual (pt1->x, pt2->x) &&
            LegacyMath::DUorEqual (pt1->y, pt2->y) &&
            LegacyMath::DUorEqual (pt1->z, pt2->z));
    }

/*---------------------------------------------------------------------------------**//**
 FP equality test with fixed tolerance 0.0001.
* @bsimethod                                                    RBB             08/86
+---------------+---------------+---------------+---------------+---------------+------*/
bool LegacyMath::DEqual (double r1, double r2)
    {
    return (fabs (r1 - r2) < .0001);
    }

/*
  EDL 01/22/02.
   mdlVec parallel and perpendicular.
   Prior behavior: Both of these computed dot products with given vectors -- no normalization.
        Parallel test for dot product within 1.0e-8 of 1.0 "always" failed for
        large near parallel vectors.
        Perpendicular test for dot product <= .0001 is still satisfied for large
            vectors, but effective angular tolerance gets smaller as vectors lengthen.
    New behavior:
        1) Normalize vectors
        2) As per analysis of taylor series, document testing for angles within 1.0e-4

  For unit vectors, cos theta = U dot V.
  For small angles,
    cos theta ~ 1 - theta^2 / 2
    theta ~ sqrt (2 (1 - cos theta))
    For theta = 1.0e-4,
        cos theta = 1 - 10^8/2
  For angles near 90 degrees,
    cos theta ~ (pi/2 - theta)
*/
static double s_parallelCosineTrigger = 1.0 - 0.5e-8;
static double s_perpendicularCosineTrigger = 1.0e-4;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             09/89
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool LegacyMath::Vec::AreParallel
(
DPoint3dCP  pVec1,       /* => bvector 1 */
DPoint3dCP  pVec2        /* => bvector 2 */
)
    {
    DPoint3d unit1 = *pVec1;
    DPoint3d unit2 = *pVec2;
    unit1.Normalize ();
    unit2.Normalize ();
    return fabs (unit1.DotProduct (unit2)) >= s_parallelCosineTrigger;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             09/89
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool LegacyMath::Vec::ArePerpendicular
(
DPoint3dCP  pVec1,
DPoint3dCP  pVec2
)
    {
    DPoint3d unit1 = *pVec1;
    DPoint3d unit2 = *pVec2;
    unit1.Normalize ();
    unit2.Normalize ();
    return fabs (unit1.DotProduct (unit2)) <= s_perpendicularCosineTrigger;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    KiranHebbar                     6/98   
+---------------+---------------+---------------+---------------+---------------+------*/
void LegacyMath::Vec::ProjectPointToPlane
(
DPoint3dP   pointOutP,
DPoint3dCP  pointInP,
DPoint3dCP  planePointP,
DPoint3dCP  normalP            /* => normal to plane */
)
    {
    double      scale;
    DPoint3d    delta, normal;

    if  (NULL == normalP || NULL == pointInP || NULL == planePointP)
        return;

    *pointOutP = *pointInP;
    normal = *normalP;

    normal.Normalize (); 
    delta.DifferenceOf (*pointInP, *planePointP);
    scale = delta.DotProduct (*normalP);

    if  (fabs (scale) > 0.0)
        {
        normal.Scale (*normalP, -scale);
        pointOutP->SumOf (*pointInP, normal);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             06/89
+---------------+---------------+---------------+---------------+---------------+------*/
Public int LegacyMath::Vec::PlanePlaneIntersect
(
DPoint3dP intPnt,
DPoint3dP   intNorm,
DPoint3dP   p1,
DPoint3dP     n1,
DPoint3dP    p2,
DPoint3dP    n2
)
    {
    if (LegacyMath::Vec::AreParallel ((DPoint3dCP )n1, (DPoint3dCP )n2))
        return (-1);        /* Parallel Planes */

    // Classic microstation tolerances are a problem here.
    // The LegacyMath::Vec::areParallel test takes the dot product and compares it to 1.
    // If one of the vectors is zero, the dot product is zero and the vectors are not cosidered parallel.
    // So we fall into here and get a true zero for the intersection direction.
    // Hence even though we have tested for parallel there is still a possibility of falling out
    // with no nonzero component of the normal.
    // (And the fc_zero tolerance is also a dubious thing to use.)
    double  d1, d2, dx, dy, dz;
    ((DPoint3dP)intNorm)->CrossProduct (*((DPoint3dCP)n1), *((DPoint3dCP)n2));
    d1 = ((DPoint3dCP)n1)->DotProduct (*((DPoint3dCP)p1));
    d2 = ((DPoint3dCP)n2)->DotProduct (*((DPoint3dCP)p2));

    dx = n1->x*d2 - n2->x*d1;
    dy = n1->y*d2 - n2->y*d1;
    dz = n1->z*d2 - n2->z*d1;

    if (!LegacyMath::DEqual (intNorm->x, 0.0))
        {
        intPnt->y =  - dz/intNorm->x;
        intPnt->z =    dy/intNorm->x;
        intPnt->x = 0.0;
        }
    else if (!LegacyMath::DEqual (intNorm->y, 0.0))
        {
        intPnt->x =    dz/intNorm->y;
        intPnt->z = -  dx/intNorm->y;
        intPnt->y = 0.0;
        }
    else if (!LegacyMath::DEqual (intNorm->z, 0.0))
        {
        intPnt->x = -dy/intNorm->z;
        intPnt->y =  dx/intNorm->z;
        intPnt->z = 0.0;
        }
    else
        {
        return -1;
        }

    ((DPoint3dP)intNorm)->Normalize ();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EDL             06/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public double LegacyMath::Vec::AngleAndAxisOfRotationFromVectorToVector
(
DPoint3dP   pAxis,
DPoint3dCP  pStartVector,
DPoint3dCP  pEndVector
)
    {
    DVec3d unitStartVector, unitEndVector;
    DVec3d crossProduct, unitCrossProduct;
    double cosine, sine, radians;

    unitStartVector.Normalize (*(DVec3d*)pStartVector);

    unitEndVector.Normalize (*(DVec3d*)pEndVector);


    if (LegacyMath::Vec::AreParallel (&unitStartVector, &unitEndVector))
        {
        if (unitStartVector.DotProduct (unitEndVector) < 0.0)
            {
            RotMatrix matrix0;
            /* The vectors are antiparallel.  Pick out a normal bvector. */
            LegacyMath::RMatrix::FromNormalVector (&matrix0, &unitStartVector);
            matrix0.GetRow(unitCrossProduct,  0);
            radians = msGeomConst_pi;
            }
        else
            {
            /* The vectors are parallel. */
            unitCrossProduct.x = 1.0;
            unitCrossProduct.y = 0.0;
            unitCrossProduct.z = 0.0;
            radians = 0.0;
            }
        }
    else
        {
        /* Usual case -- non parallel, nonzero vectors.
           Rotation is "about" the axis formed by their cross product. */
        crossProduct.CrossProduct (unitStartVector, unitEndVector);
        unitCrossProduct = crossProduct;
        sine = unitCrossProduct.Normalize ();
        cosine = unitStartVector.DotProduct (unitEndVector);

        radians = Angle::Atan2 (sine, cosine);
        }

    if (pAxis)
        *pAxis = unitCrossProduct;
    return radians;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EDL             06/02
+---------------+---------------+---------------+---------------+---------------+------*/
void LegacyMath::RMatrix::FromVectorToVector
(
RotMatrixP pMatrix,
DPoint3dCP pStartVector,    /* xyz components of start bvector */
DPoint3dCP pEndVector    /* xyz components of end bvector */
)
    {
    double radians;
    DVec3d axis;
    radians = LegacyMath::Vec::AngleAndAxisOfRotationFromVectorToVector (&axis, pStartVector, pEndVector);
    if (pMatrix)
        pMatrix->InitFromVectorAndRotationAngle(axis, radians);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB     10/90
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt LegacyMath::Vec::LinePlaneIntersect (DPoint3dP intersectPt, DPoint3dCP linePt,DPoint3dCP lineNormal, 
                                     DPoint3dCP planePt, DPoint3dCP planeNormal, int perpendicular)
    {
    double      t, dotp = 0.0;
    DPoint3d    diff, rtmp;

    if (lineNormal)
        dotp = lineNormal->DotProduct (*planeNormal);
    else
        perpendicular = true;

    if (perpendicular || LegacyMath::DEqual (dotp, 0.0))
        {
        diff.DifferenceOf (*planePt, *linePt);
        t = diff.DotProduct (*planeNormal);
        rtmp.Scale (*planeNormal, t);
        }
    else
        {
        t = (planeNormal->DotProduct (*((DPoint3dCP)planePt)) -
             planeNormal->DotProduct (*((DPoint3dCP)linePt)))/dotp;
        rtmp.Scale (*lineNormal, t);
        }

    ((DPoint3dP)intersectPt)->SumOf (rtmp, *((DPoint3dCP)linePt));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                      EarlinLutz          02/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt LegacyMath::Vec::LinePlaneIntersectParameter
(
DPoint3d       *intersectionP,
double          *parameterP,
DPoint3d const *lineStartP,
DPoint3d   const *lineDirectionP,
DPoint3d const *planePointP,
DPoint3d   const *planeNormalP
)
    {
    double parameter = 0.0;
    StatusInt status = SUCCESS;

    double aa = lineDirectionP->DotProduct (*planeNormalP);
    double bb = planePointP->DotDifference(*lineStartP, *((DVec3dCP) planeNormalP));

    if (    lineDirectionP->IsPerpendicularTo(*planeNormalP)
        || !DoubleOps::SafeDivide (parameter, bb, aa, 0.0))
        status = ERROR;
    if (parameterP)
        *parameterP = parameter;
    if (intersectionP)
        intersectionP->SumOf (*lineStartP, *lineDirectionP, parameter);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             04/89
+---------------+---------------+---------------+---------------+---------------+------*/
static void getRealCorners (DPoint3dP corner, DPoint3dCP low, DPoint3dCP high)
    {
    corner[0].x = low->x;     corner[0].y = low->y;    corner[0].z = low->z;
    corner[1].x = high->x;    corner[1].y = low->y;    corner[1].z = low->z;
    corner[2].x = low->x;     corner[2].y = high->y;   corner[2].z = low->z;
    corner[3].x = high->x;    corner[3].y = high->y;   corner[3].z = low->z;
    corner[4].x = low->x;     corner[4].y = low->y;    corner[4].z = high->z;
    corner[5].x = high->x;    corner[5].y = low->y;    corner[5].z = high->z;
    corner[6].x = low->x;     corner[6].y = high->y;   corner[6].z = high->z;
    corner[7].x = high->x;    corner[7].y = high->y;   corner[7].z = high->z;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    RayBentley                      3/94   
+---------------+---------------+---------------+---------------+---------------+------*/
Public void LegacyMath::Vec::ComputeRangeProjection
(
double      *minProjectionP,            /* <= minimum projection on ray */
double      *maxProjectionP,            /* => maximum projection on ray */
DPoint3d    *rangeLowP,                 /* => low range */
DPoint3d    *rangeHighP,                /* => high range */
DPoint3d    *rayOriginP,                /* => ray origin (or NULL for 0,0,0) */
DPoint3d    *rayNormalP                 /* => ray normal */
)               
    {
    int         i;
    double      projection;
    DPoint3d    corners[8], delta;
    
    getRealCorners (corners, rangeLowP, rangeHighP);
    for (i=0; i<8; i++)
        {
        if (NULL == rayOriginP)
            delta = corners[i];
        else
            delta.DifferenceOf (corners[i], *rayOriginP);
            
        projection = delta.DotProduct (*((DPoint3dCP)rayNormalP));
        if (i)
            {
            if (projection < *minProjectionP) *minProjectionP = projection;
            if (projection > *maxProjectionP) *maxProjectionP = projection;
            }
        else
            {
            *minProjectionP = *maxProjectionP = projection;
            }
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                         
+---------------+---------------+---------------+---------------+---------------+------*/
Public int  LegacyMath::Vec::ProjectPointToLine
(
DPoint3d    *outPointP,
double      *outFractionP,
DPoint3dCP inPointP,
DPoint3dCP startPointP,
DPoint3dCP endPointP
)
    {
    DSegment3d segment;
    bsiDSegment3d_initFromDPoint3d (&segment, startPointP, endPointP);
    return bsiDSegment3d_projectPoint (&segment, outPointP, outFractionP, inPointP)
            ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 06/06
* This function does a singular value decomposition of the given matrix into
* a translation, two rotations and a scale. It then multiplies the translation and
* two rotations back together. The effect of this function on a transform containing
* reflection is not verified.
* The anchor point is the origin of the element that will be transformed.
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt LegacyMath::TMatrix::Unscale
(
TransformP    newTransform,
TransformCP   oldTransform,
DPoint3dCP    anchor
)
    {
    double      determinant = 0;
    DPoint3d    R_anchor, U_anchor, oldTranslation, newTranslation;
    RotMatrix   matrixR, matrixU, rotMatrix1, rotMatrix2;

    // Old transform is [R B]
    // New transform is [U A]
    // U is R with its scale stripped.
    // We want [R B]*anchor = [U A]*anchor
    //          R*anchor + B = U*anchor + A
    //          A = R*anchor + B - u * anchor

    // Check for right-handed, orthogonal
    oldTransform->GetMatrix (matrixR);
    oldTransform->GetTranslation (oldTranslation);
    determinant = matrixR.Determinant ();
    if (determinant < 0.0)
        return ERROR;

    bsiRotMatrix_factorRotateScaleRotate (&matrixR, &rotMatrix1, NULL, &rotMatrix2);
    matrixU.InitProduct (rotMatrix1, rotMatrix2);
    matrixU.Multiply (U_anchor, *anchor);
    matrixR.Multiply (R_anchor, *anchor);
    //matrixU.Multiply (&U_anchor, anchor, 1);
    //matrixR.Multiply (&R_anchor, anchor, 1);
    newTranslation.SumOf (
                        R_anchor, 1.0,
                        oldTranslation, 1.0,
                        U_anchor, -1.0);
    newTransform->InitFrom (matrixU, newTranslation);

    return SUCCESS;
    }

/*----------------------------------------------------------------------+
@bsimethod                              EarlinLutz 03/04
+----------------------------------------------------------------------*/
static RotMatrixCP derefRotMatrixPointer (RotMatrixP destP, RotMatrixCP sourceP)
    {
    if (sourceP)
        *destP = *sourceP;
    else
        destP->InitIdentity ();
    return destP;
    }

/*---------------------------------------------------------------------------------**//**
* author kab.JRG 9/94
* Note: in assumed to be post-multiplied by scale: in = rotation * scale
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LegacyMath::RMatrix::GetColumnScaleVector
(
RotMatrixP normalizedRMatrix,     /* <= normalized rmatrix */
DPoint3dP scaleVector,           /* <= scale bvector */
RotMatrixCP in                     /* => rotation/scaling matrix */
)
    {
    DVec3d      xCol, yCol, zCol, mag;
    RotMatrix   matrix;
    in = derefRotMatrixPointer (&matrix, in);

    in->GetColumn (xCol, 0);
    mag.x = xCol.Magnitude ();
    in->GetColumn (yCol, 1);
    mag.y = yCol.Magnitude ();
    in->GetColumn (zCol, 2);
    mag.z = zCol.Magnitude ();

    if (scaleVector)
        *scaleVector = mag;

    if (normalizedRMatrix)
        {
        if (mag.x > DBL_EPSILON)
            xCol.Scale (xCol, 1.0/mag.x);
        if (mag.y > DBL_EPSILON)
            yCol.Scale (yCol, 1.0/mag.y);
        if (mag.z > DBL_EPSILON)
            zCol.Scale (zCol, 1.0/mag.z);

        normalizedRMatrix->InitFromColumnVectors (xCol, yCol, zCol);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    kab                             1/90   
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool LegacyMath::Vec::ColinearToTolerance
(
DPoint3dCP  pt,                /* => 3 points to test*/
double      tolerance           /* => tolerance for components of perpendicular */
)
    {
    bool        colinear;
    DPoint3d    normal1, delta2, perpendicular, parallel;

    if (LegacyMath::RpntEqual (&pt[0],&pt[2]) ||
        LegacyMath::RpntEqual (&pt[0],&pt[1]) ||
        LegacyMath::RpntEqual (&pt[1],&pt[2]))
        return (true);

    normal1.NormalizedDifference (pt[1], pt[0]);
    delta2.DifferenceOf (pt[2], pt[0]);

    parallel.Scale (normal1, delta2.DotProduct (normal1));
    perpendicular.DifferenceOf (delta2, parallel);

    colinear = fabs (perpendicular.x) <= tolerance &&
               fabs (perpendicular.y) <= tolerance &&
               fabs (perpendicular.z) <= tolerance;

    return colinear;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    kab                             1/90   
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool LegacyMath::Vec::Colinear
(
const DPoint3d *pt                  /* <=> */
)
    {
    return LegacyMath::Vec::ColinearToTolerance (pt, mgds_fc_epsilon);
    }

