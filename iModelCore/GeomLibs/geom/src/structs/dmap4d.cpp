/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "../DeprecatedFunctions.h"
#include <stdlib.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
Public GEOMDLLIMPEXP bool    bsiDMatrix4d_invertQR

(
DMatrix4dP    pMatrixB,
DMatrix4dCP    pMatrixA
);

/*----------------------------------------------------------------------+
|                                                                       |
|   Local defines                                                       |
|                                                                       |
+----------------------------------------------------------------------*/


#define MDL_HM_TRANSLATE_BIT    0x00000001
#define MDL_HM_FRAME_BIT        0x00000002
#define MDL_HM_PERSPECTIVE_BIT  0x00000004
#define MDL_HM_SCALE_BIT        0x00000008
#define MDL_HM_SINGULAR_BIT     0x10000000

#define MDL_HM_TYPE_BITS                0x0000000F
#define MDL_HM_IS_IDENTITY_MASK(bits) \
        ( !(bits & MDL_HM_TYPE_BITS) )

#define MDL_HM_AFFINE_BITS              0x00000003
#define MDL_HM_WEIGHT_BITS              0x0000000C
#define MDL_HM_IS_AFFINE_MASK(bits) \
        ( !(bits & MDL_HM_WEIGHT_BITS) )

#define MDL_HM_IS_PERSPECTIVE_MASK(bits) \
        ( (bits & MDL_HM_PERSPECTIVE_BIT) )

#define MDL_HM_SETMASK(pHMap,bits) \
        ( (pHMap)->mask |= bits )

#define MDL_HM_CLEARMASK(pHMap,bits ) \
        ( (pHMap)->mask &= ~bits )

#define MDL_HM_GETMASK(pHMap,bits ) \
        ( (pHMap)->mask & bits )

#define MDL_HM_BIT_DOT(outBit, pAb, aBit, bBit, pCd, cBit, dBit ) \
        (       ( (pAb->mask & aBit) && (pCd->mask & cBit) )    \
            ||  ( (pAb->mask & bBit) && (pCd->mask & dBit) )    \
        ? outBit : 0 )

#define MDL_HM_SETTYPE(pT,bits) \
        ( MDL_HM_CLEARMASK(pT, MDL_HM_TYPE_BITS ),      \
          MDL_HM_SETMASK( pT, bits) )

void DMap4d::SetTypeBits (int mask) {MDL_HM_SETTYPE (this, mask);}


//<refMethodImpl>
void DMap4d::InitFrom (DMatrix4dCR _M0, DMatrix4dCR _M1)
    {
    M0 = _M0;
    M1 = _M1;
    SetTypeBits(MDL_HM_TYPE_BITS);
    }
//</refMethodImpl>

//<refMethodImpl>
DMap4d DMap4d::From (DMatrix4dCR _M0, DMatrix4dCR _M1)
    {
    DMap4d result;
    result.InitFrom (_M0, _M1);
    return result;
    }
//</refMethodImpl>


/*-----------------------------------------------------------------*//**
* @class DMap4d
*
* An DMap4d structure is a homogeneous linear transformation (4x4) and its
* inverse.  The two are stored in the same structure, and all construction
* and modification operations act on both the forward and inverse
* transformations.                        |

* The forward and inverse matrices are individually accessible as
* pHMap->M0 and pHMap->M1.

*
* @author   EarlinLutz
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+===============+===============+===============+===============+======*/

/*----------------------------------------------------------------------+
| TITLE intro  Invertible homogeneous transformations                   |
| An DMap4d structure is a homogeneous linear transformation (4x4) and its|
| inverse.  The two are stored in the same structure, and all construction
and modification operations act on both the forward and inverse
transformations.                        |

 The forward and inverse matrices are individually accessible as
 pHMap->M0 and pHMap->M1.

|                                                                       |
|<UL>
<LI> Initializations
<UL>
|<LI>bsiDMap4d_initIdentity -- initialize as an identity transform       |
|<LI>jmdlDMap4d_fillBoxMap -- initialize a mapping between two range cubes        |
|<LI>jmdlDMap4d_fillglLFrustum -- initalize a z-direction
perspective projection projection frustum from backplane window and
clip plane depths
<LI>jmdlDMap4d_fillglOrtho -- initialize a z-direction flat projection
frustum from backplane window and clip depths
<LI>bsiDMap4d_zFrustum -- initialize a z-direction perspective frustum
from a back-plane depth and half-scale fraction
<LI>bsiDMap4d_initRotateTrig -- initialize a rotation about an arbitrary
axis, with rotation angle given as the cosine and sine of the angle.
<LI>bsiDMap4d_initRotateTrig -- initialize a rotation about an arbitrary
axis, with rotation angle given as an integer multiple of 90 degrees
<LI>bsiDMap4d_initRotateTrig -- initialize a rotation about an arbitrary
axis, with rotation angle given in radians
<LI>bsiDMap4d_initScale -- initialize a transform with scale factors
along each principle axis.
<LI>bsiDMap4d_initUniformScale -- initialize a transform with uniform
scaling.
<LI>bsiDMap4d_initTranslate -- initialize a pure translation transform
<LI>jmdlDMap4d_fillPerspective -- initialize with an identity matrix
and additional nonzero values for the perspective row.
<LI>jmdlDMap4d_fillFromDTransform3d -- initialize from an affine transformation
<LI>bsiDMap4d_initFromCameraParameters -- initialize a perspective
transformation from typical camera parameters
</UL>
<LI>Operations
<UL>
<LI>bsiDMap4d_multiply -- multiply two transforms
<LI>bsiDMap4d_sandwich -- form a product inverse(B)*A*B for two
transforms A and B.
<LI>bsiDMap4d_invert -- initialize a mapping that is the inverse
of the given mapping.
</UL>
<LI>Queries
<UL>
<LI>bsiDMap4d_isIndependent -- test if a transformation is a principle axis
scaling operation.
<LI>bsiDMap4d_isSingular -- test if the transformation is singular.
<LI>bsiDMap4d_isAffine -- test if the transformation has only affine

(non-perspective) components.
<LI>bsiDMap4d_size -- return the size of the DMap4d structure.
</UL>
</UL>
+----------------------------------------------------------------------*/




/*-----------------------------------------------------------------*//**
*
* Initialize and identity mapping.
*
* @instance pHMap <= identity transform
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMap4d_initIdentity

(
DMap4dP  pHMap
)

    {
    pHMap->M0.InitIdentity ();
    pHMap->M1.InitIdentity ();
    pHMap->mask = 0;
    MDL_HM_CLEARMASK(pHMap,
          MDL_HM_SCALE_BIT
        | MDL_HM_FRAME_BIT
        | MDL_HM_PERSPECTIVE_BIT
        | MDL_HM_TRANSLATE_BIT );
    }
#ifdef CompileAll
/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlDMap4d_interpY                                        |
|                                                                       |
| author        EarlinLutz                                      7/96    |
|                                                                       |
| Interpolate y as a function of x. Return average if vertical.         |
|RETURNS interpolated y
+----------------------------------------------------------------------*/
static double jmdlDMap4d_interpY /* <= interpolated y value */

(
double x0,                      /* => start point x coordinate */
double y0,                      /* => start point y coordinate */
double x1,                      /* => end point x coordinate */
double y1,                      /* => end point y coordinate */
double x                        /* => interpolation x coordinate */
)
    {
    double dx = x1 - x0;
    double y;
    if ( dx == 0.0 )
        {
        y = 0.5*(y0 + y1);
        }
    else
        {
        y = y0 + (x - x0)*(y1 - y0)/dx;
        }
    return y;
    }
#endif

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlDMap4d_mapBetweenBoxes                                |
|                                                                       |
| author        EarlinLutz                                      7/96    |
|                                                                       |
+----------------------------------------------------------------------*/
static bool    boolean_jmdlDMap4d_mapBetweenBoxes

(
DPoint3dP pOrg,
DPoint3dP pSlope,
DPoint3dCP pA0,
DPoint3dCP pA1,
DPoint3dCP pB0,
DPoint3dCP pB1
)
    {
    double dx = pA1->x - pA0->x;
    double dy = pA1->y - pA0->y;
    double dz = pA1->z - pA0->z;

    if ( dx != 0.0 && dy != 0.0 && dz != 0.0 )
        {
        pSlope->x = (pB1->x - pB0->x) / dx;
        pSlope->y = (pB1->y - pB0->y) / dy;
        pSlope->z = (pB1->z - pB0->z) / dz;
        pOrg->x   = pB0->x - pSlope->x * pA0->x;
        pOrg->y   = pB0->y - pSlope->y * pA0->y;
        pOrg->z   = pB0->z - pSlope->z * pA0->z;
        return  true;
        }

    return false;
    }



/*-----------------------------------------------------------------*//**
* Initialize a transform which translates and scales along principle
* axes so box loAP..hiAP maps to box loBP..hiBP
*
* @instance pHMap <= transform
* @param loAP => corner of box A
* @param hiAP => diagonally opposite corner of box A
* @param loBP => corner of box B
* @param hiBP => diagonally opposite corner of box B
* @see
* @return int
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDMap4d_initBoxMap

(
DMap4dP  pHMap,
DPoint3dCP loAP,
DPoint3dCP hiAP,
DPoint3dCP loBP,
DPoint3dCP hiBP
)

    {
    DPoint3d slopeAinB, slopeBinA;
    DPoint3d orgAinB, orgBinA;

    if (  boolean_jmdlDMap4d_mapBetweenBoxes( &orgAinB, &slopeAinB,
                                                loAP, hiAP, loBP, hiBP )
       && boolean_jmdlDMap4d_mapBetweenBoxes( &orgBinA, &slopeBinA,
                                                loBP, hiBP, loAP, hiAP )
       )
        {
        pHMap->M0.InitFromScaleAndTranslation (slopeAinB, orgAinB );
        pHMap->M1.InitFromScaleAndTranslation (slopeBinA, orgBinA );
        pHMap->mask = 0;
        MDL_HM_SETTYPE( pHMap, MDL_HM_FRAME_BIT | MDL_HM_TRANSLATE_BIT );
        return true;
        }

    bsiDMap4d_initIdentity( pHMap );
    return false;
    }




/*-----------------------------------------------------------------*//**
* Initialize a rotation about axis vx,vy,vz by angle whose cosine
* and sine are (proportional to) c and s.
*
* @instance pHMap <= returned DMap4d
* @param c => cosine of angle
* @param s => sine of angle
* @param vx => x component of rotation axis
* @param vy => y component of rotation axis
* @param vz => z component of rotation axis
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMap4d_initRotateTrig

(
DMap4dP  pHMap,
double c,
double s,
double vx,
double vy,
double vz
)

    {
    double r = sqrt (c*c + s*s);
    int i,j;
    double u[3];
    double a;
    double lv = sqrt(vx*vx + vy*vy + vz*vz);

    if (lv <= 0.0)
        {
        bsiDMap4d_initIdentity (pHMap);
        return;
        }

    if (r <= 0.0)
        {
        c = 1.0;
        s = 0.0;
        }
    else
        {
        c /= r;
        s /= r;
        }

    u[0] = vx/lv;
    u[1] = vy/lv;
    u[2] = vz/lv;

    for ( i = 0; i < 3; i++)
        for ( j = 0; j < 3; j++ )
            {
            a = u[i]*u[j];
            pHMap->M0.coff[i][j] =
                a - c * ( i == j ? a - 1.0 : a );
            }
    pHMap->M0.coff[0][1] -= s*u[2];
    pHMap->M0.coff[0][2] += s*u[1];
    pHMap->M0.coff[1][0] += s*u[2];
    pHMap->M0.coff[1][2] -= s*u[0];
    pHMap->M0.coff[2][0] -= s*u[1];
    pHMap->M0.coff[2][1] += s*u[0];

    pHMap->M0.coff[0][3] = 0.0;
    pHMap->M0.coff[1][3] = 0.0;
    pHMap->M0.coff[2][3] = 0.0;

    pHMap->M0.coff[3][3] = 1.0;

    pHMap->M0.coff[3][0] = 0.0;
    pHMap->M0.coff[3][1] = 0.0;
    pHMap->M0.coff[3][2] = 0.0;

    pHMap->M1.TransposeOf (pHMap->M0);
    pHMap->mask = 0;
    MDL_HM_SETTYPE( pHMap, MDL_HM_FRAME_BIT );
    }




/*-----------------------------------------------------------------*//**
* Rotate about vx,yv,vz by an integer multiple of 90 degrees.
* Providing the angle as an integer allows exact table lookup without
* approximation of pi.
*
* @instance pHMap <= returned DMap4d
* @param multiple => rotation angle is multiple * 90 degrees
* @param vx => x component of rotation axis
* @param vy => y component of rotation axis
* @param vz => z component of rotation axis
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMap4d_initRotateQuadrant

(
DMap4dP  pHMap,
int     multiple,
double vx,
double vy,
double vz
)

    {
    static double trigArray[4] =
        {
        1.0,            /* = cos(0) = sin(90)           */
        0.0,            /* = cos(90) = sin(180)         */
        -1.0,           /* = cos(180) = sin(270)        */
        0.0             /* = cos(270) = sin(0)          */
        };
    double c, s;
    int absMultiple = abs(multiple);

    c = trigArray[ absMultiple % 4];
    s = trigArray[ (absMultiple + 3) % 4];

    if (multiple < 0.0)
        s = -s;

    bsiDMap4d_initRotateTrig (pHMap, c, s, vx, vy, vz);
    }




/*-----------------------------------------------------------------*//**
* Initialize a rotation by angle in radians about axis vx,vy,vz

* @instance pHMap <= rotational transform
* @param radians => angle in degrees
* @param vx => x component of rotation axis
* @param vy => y component of rotation axis
* @param vz => z component of rotation axis
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMap4d_initRotate

(
DMap4dP  pHMap,
double radians,
double vx,
double vy,
double vz
)

    {
    bsiDMap4d_initRotateTrig (pHMap, cos(radians), sin(radians), vx, vy, vz);
    }




/*-----------------------------------------------------------------*//**
* Initialize a pure scaling transformation.
* If any scale factor is zero, the corresponding inverse entry is also
* zero.

* @instance pHMap <= scaling transformation
* @param ax => x scale factor
* @param ay => y scale factor
* @param az => z scale factor
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMap4d_initScale

(
DMap4dP  pHMap,
double ax,
double ay,
double az
)

    {

    double bx, by, bz;
    pHMap->M0.InitIdentity ();
    pHMap->M0.coff[0][0] = ax;
    pHMap->M0.coff[1][1] = ay;
    pHMap->M0.coff[2][2] = az;
    MDL_HM_SETTYPE( pHMap, MDL_HM_SCALE_BIT );
    pHMap->mask = 0;

    bx =  ( ax == 0.0 ? 0.0 : 1.0 / ax);
    by =  ( ay == 0.0 ? 0.0 : 1.0 / ay);
    bz =  ( az == 0.0 ? 0.0 : 1.0 / az);

    pHMap->M1.InitIdentity ();
    pHMap->M1.coff[0][0] = bx;
    pHMap->M1.coff[1][1] = by;
    pHMap->M1.coff[2][2] = bz;

    if ( bx == 0.0 || by == 0.0 || bz == 0.0 )
        {
        MDL_HM_SETMASK(pHMap,MDL_HM_SINGULAR_BIT);
        }

    }



/*-----------------------------------------------------------------*//**
* Initialize a transform that is a (noninvertible) projection to a
* principle plane.

* @instance pHMap <= projection map
* @param height => distance of plane from origin
* @param axis => 0,1,2 for x,y,z normal
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMap4d_initPrincipleProjection

(
DMap4dP  pHMap,
double height,
int    axis
)
    {
    pHMap->M0.InitIdentity ();
    axis = axis % 3;
    pHMap->M0.coff[axis][axis] = 0.0;
    pHMap->M0.coff[axis][3]    = height;
    MDL_HM_SETTYPE( pHMap, MDL_HM_SCALE_BIT );
    MDL_HM_SETMASK(pHMap,MDL_HM_SINGULAR_BIT);
    }


/*-----------------------------------------------------------------*//**
* Initialize a transform that incorporates a given origin, x direction, and additional
* local transformation, all relative to a given transformation.
* (This is the guts of AgentContext::pushTransformTo())
* @instance pHMap <= projection map
* @param pOrigin => world origin
* @param pXDir   => world x direction
* @param pWorldToTarget => map from world to target system
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMap4d_initFromTransformedOriginAndDirection

(
DMap4dP pCompositeMap,
DPoint3dCP pOrigin,
DPoint3dCP pXDir,
DMap4dCP pTargetToWorld,
DMap4dCP pLocalFrame
)
    {
    int numWorkPoint;
    DPoint3d workPoint[3];
    DPoint3d origin;
    DMap4d mapOrigin, mapRotate;

    /* Apply origin and (possibly) rotation shifts. */
    numWorkPoint = 1;

    if (pOrigin)
        {
        workPoint[0] = *pOrigin;
        }
    else
        {
        origin.Zero ();
        bsiDMatrix4d_multiplyAndRenormalizeDPoint3dArray (
                    &pTargetToWorld->M0, workPoint, &origin, 1);
        }

    if (pXDir)
        {
        workPoint[1].SumOf (*workPoint, *pXDir);
        numWorkPoint++;
        }

    bsiDMatrix4d_multiplyAndRenormalizeDPoint3dArray (
                    &pTargetToWorld->M1, workPoint, workPoint, numWorkPoint);

    bsiDMap4d_initTranslate (&mapOrigin, workPoint[0].x, workPoint[0].y, workPoint[0].z);

    bsiDMap4d_multiply (pCompositeMap, pTargetToWorld, &mapOrigin);

    if (pXDir)
        {
        double dx = workPoint[1].x - workPoint[0].x;
        double dy = workPoint[1].y - workPoint[0].y;
        bsiDMap4d_initRotateTrig (&mapRotate, dx, dy, 0.0, 0.0, 1.0);
        bsiDMap4d_multiply (pCompositeMap, pCompositeMap, &mapRotate);
        }

    if (pLocalFrame)
        {
        bsiDMap4d_multiply (pCompositeMap, pCompositeMap, pLocalFrame);
        }
    }



/*-----------------------------------------------------------------*//**
* Initialize a uniform scaling transformation.

* @instance pHMap <= scaling transformation
* @param a => scale factor
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMap4d_initUniformScale

(
DMap4dP  pHMap,
double a
)

    {
    bsiDMap4d_initScale(pHMap, a,a,a);
    }





/**


Initialize a translation transform.

* @instance pHMap <= translation transform
* @param tx => x component of translation
* @param ty => y component of translation
* @param tz => z component of translation
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMap4d_initTranslate

(
DMap4dP  pHMap,
double tx,
double ty,
double tz
)
    {
    pHMap->M0.InitFromTranslation (tx, ty, tz );
    pHMap->M1.InitFromTranslation (-tx, -ty, -tz);
    pHMap->mask = 0;
    MDL_HM_SETTYPE( pHMap, MDL_HM_TRANSLATE_BIT );
    }




/*-----------------------------------------------------------------*//**
* Initialize a transform with perspective entries for a (nonzero)
* taper in the z direction.

* @instance pHMap <= transform
* @param taper => taper fraction
* @see
* @return true if an invertible map was constructed.
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDMap4d_initTaper

(
DMap4dP     pHMap,
double      taper
)

    {
    static double minTaper = 1.0e-12;
    bool    boolStat = false;
    bsiDMap4d_initIdentity (pHMap);

    if ( fabs (taper) < minTaper)
        {
        boolStat = false;
        }
    else if (taper == 1.0)
        {
        /* just leave it as an identity */
        boolStat = true;
        }
    else
        {
        /* npc to slab:*/
        pHMap->M0.coff[2][2] = 1.0 / taper;
        pHMap->M0.coff[3][2] = (1.0 - taper) / taper;

        /* slab to npc:*/
        pHMap->M1.coff[2][2] = taper;
        pHMap->M1.coff[3][2] = taper - 1.0;

        pHMap->mask = 0;
        MDL_HM_SETTYPE( pHMap, MDL_HM_TYPE_BITS );
        boolStat = true;
        }
    return boolStat;
    }








/*-----------------------------------------------------------------*//**
* @instance pHMap <= mapping constructed from the matrix
* @param pTransform => affine transformation 3x4 matrix
* @param invert => true to treat this matrix as the inverse
*                                        of the mapping, false if forward.
* @see
* @return true if the Transfrom was invertible.
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDMap4d_initFromTransform

(
DMap4dP pHMap,
TransformCP pTransform,
bool        invert
)

    {
    Transform inverseTransform;
    bool    boolStat = inverseTransform.InverseOf (*pTransform);

    if (!boolStat)
        inverseTransform.InitIdentity ();

    if (invert)
        {
        pHMap->M1.InitFrom (*pTransform);
        pHMap->M0.InitFrom (inverseTransform);
        }
    else
        {
        pHMap->M0.InitFrom (*pTransform);
        pHMap->M1.InitFrom (inverseTransform);
        }

    pHMap->mask = 0;
    MDL_HM_SETTYPE( pHMap, MDL_HM_FRAME_BIT | MDL_HM_TRANSLATE_BIT);
    return boolStat;
    }

/*--------------------------------------------------------------+
|SUBSECTION frustums Frustum construction                       |
| bsiDMap4d_glOrtho, bsiDMap4d_glFrustum, and bsiDMap4d_zFrustum   |
| construct mappings to a viewing frustum.                      |
+--------------------------------------------------------------*/



/*-----------------------------------------------------------------*//**
* Construct a projective transformations corresponding to
* the opengl functions glOrtho and glFrustum.
* The usual description is:
<UL>
<LI>Eyepoint at origin, looking down the negative z axis.
<LI> far and near clipping planes at
*   z=farCoord and z=nearCoord, respectively.
<LI>Side clipping planes pass through (leftCoord,backCoord,nearCoord) and (rightCoord,topCoord,nearCoord)
* i.e. leftCoord,rightCoord,backCoord,topCoord give window coordinates on a plane at distance nearCoord.
* The view frustum is mapped to a unit cube with +1 values
* at the corner points.
*<LI>In x: (leftCoord,rightCoord) maps to (1,+1)
*<LI>In y: (backCoord,topCoord) maps to (1,+1)
*<LI>In z: (farCoord,nearCoord) maps to (1,+1)
<UL>
* In strict use of the opengl documentation, the
* quantities should satisfy leftCoord<rightCoord, backCoord<topCoord, and nearCoord<farCoord,
* i.e. correspond to the names left, right, bottom, top, near,
  far.
* However, it is perfectly fine to reverse any or all in order
* to control the orientation and scaling.  In particular,
* you can get a right handed coordinate system in the projected
* box by using farCoord<nearCoord. The interpretation that leftCoord,rightCoord,backCoord,topCoord are a window
* at depth nearCoord still holds, but it is just at the back of the
* frustum instead of the front.

* @param pHMap <= projection transform
* @param leftCoord => left x coordinate at depth z=-nearCoord near plane
* @param rightCoord => right x coordiante at depth z=-nearCoord near plane
* @param backCoord => bottom y coordiante at depth z=-nearCoord near plane
* @param topCoord => top y coordiante at depth z=-nearCoord near plane
* @param nearCoord => distance to near plane
* @param farCoord => distance to far plane
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMap4d_glOrtho

(
DMap4dP  pHMap,
double leftCoord,
double rightCoord,
double backCoord,
double topCoord,
double nearCoord,
double farCoord
)
    {
    static double ndc_scale = 0.5;
    static double ndc_shift = 0.5;
    static double ndc_orientation = -1.0;
    double sx =     ndc_scale*(rightCoord-leftCoord);
    double sy =     ndc_scale*(topCoord-backCoord);
    double sz =     ndc_scale*(farCoord-nearCoord) * ndc_orientation;
    double xm = leftCoord + ndc_shift*(rightCoord-leftCoord);
    double ym = backCoord + ndc_shift*(topCoord-backCoord);
    double zm = -(farCoord + ndc_shift*(nearCoord-farCoord));

    pHMap->mask = 0;
    MDL_HM_SETTYPE( pHMap, MDL_HM_AFFINE_BITS );

    pHMap->M0.SetRow (0,
                1.0/sx, 0.0,   0.0, - xm/sx );
    pHMap->M0.SetRow (1,
                0.0, 1.0/sy,   0.0, - ym/sy );
    pHMap->M0.SetRow (2,
                0.0,  0.0 ,  1.0/sz ,  - zm / sz );
    pHMap->M0.SetRow (3,
                0.0,  0.0 ,  0.0, 1.0 );

    pHMap->M1.SetRow (0,
                sx , 0.0, 0.0,   xm );
    pHMap->M1.SetRow (1,
                0.0, sy ,  0.0,  ym );
    pHMap->M1.SetRow (2,
                0.0, 0.0,  sz,   zm );
    pHMap->M1.SetRow (3,
                0.0, 0.0, 0.0,  1.0 );
    }



/*-----------------------------------------------------------------*//**
* Initialize a flat projection frustum.

* @param pHMap <= projection transform
* @param leftCoord => left x coordinate at depth z=-nearCoord near plane
* @param rightCoord => right x coordiante at depth z=-nearCoord near plane
* @param backCoord => bottom y coordiante at depth z=-nearCoord near plane
* @param topCoord => top y coordiante at depth z=-nearCoord near plane
* @param nearCoord => distance to near plane
* @param farCoord => distance to far plane
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMap4d_glFrustum

(
DMap4dP  pHMap,
double leftCoord,
double rightCoord,
double backCoord,
double topCoord,
double nearCoord,
double farCoord
)
    {
    static double ndc_scale = 0.5;
    static double ndc_shift = 0.5;
    static double ndc_orientation = -1.0;

    double sx =     ndc_scale*(rightCoord-leftCoord);
    double sy =     ndc_scale*(topCoord-backCoord);
    double sz =     ndc_scale*(farCoord-nearCoord) * ndc_orientation;
    double xm = leftCoord + ndc_shift*(rightCoord-leftCoord);
    double ym = backCoord + ndc_shift*(topCoord-backCoord);
    double zm = -(farCoord + ndc_shift*(nearCoord-farCoord));

    pHMap->mask = 0;
    MDL_HM_SETTYPE( pHMap, MDL_HM_TYPE_BITS );
    pHMap->M0.SetRow (0,
                nearCoord / sx, 0.0, xm/sx , 0.0 );
    pHMap->M0.SetRow (1,
                0.0,  nearCoord / sy,  ym/sy , 0.0 );
    pHMap->M0.SetRow (2,
                0.0,  0.0 ,   -zm/sz , farCoord*nearCoord / sz );
    pHMap->M0.SetRow (3,
                0.0,  0.0 ,  -1.0, 0.0 );

    pHMap->M1.SetRow (0,
                sx / nearCoord, 0.0, 0.0, xm / nearCoord );
    pHMap->M1.SetRow (1,
                0.0, sy / nearCoord,  0.0, ym / nearCoord );
    pHMap->M1.SetRow (2,
                0.0, 0.0, 0.0, -1.0 );
    pHMap->M1.SetRow (3,
                0.0, 0.0, sz / (farCoord*nearCoord), -zm / ( farCoord*nearCoord) );
    }




/*-----------------------------------------------------------------*//**
* There is a simple, direct transformation from world to a perspective
* in a slab.   Unfortunately, camerabased abstraction presented to
* the outside world wants to go there by way of a (rigid) camera.
* So we have to take the long way around the barn to achieve
*<UL>
*<LI> world > camera > slab > perspective slab
*</UL>
* by combining the two easily computed chains:
*<UL>
*<LI> world > camera
*<LI> world > slab > perspective slab
*</UL>

* @param pWorldToCameraMap <= world to camera part of transform. May be NULL pointer.
* @param pCameraToNpcMap <= camera to npc part of transform. May be NULL pointer.
* @param pWorldToNpcMap <= world to npc.  May be NULL pointer
* @param pOrigin => lower left rear of frustum
* @param pUPoint => lower right rear of frustum
* @param pVPoint => upper right rear of frustum
* @param pWPoint => lower left front of frustum
* @param fraction => front size divided by back size
* @see
* @return true if frustum is well defined
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDMap4d_init3PointCameraAndFrustum


(
DMap4dP        pWorldToCameraMap,
DMap4dP        pCameraToNpcMap,
DMap4dP        pWorldToNpcMap,
DPoint3dCP pOrigin,
DPoint3dCP pUPoint,
DPoint3dCP pVPoint,
DPoint3dCP pWPoint,
double      fraction
)
    {
    Transform slabToWorldTransform;

    DMap4d worldToSlabMap;
    DMap4d slabToNpcMap;
    DMap4d cameraToWorldMap;

    /* These allocations are for use when NULL pointers are provided */
    DMap4d worldToCameraMap;
    DMap4d cameraToNpcMap;
    DMap4d worldToNpcMap;
    static double frustumFractionLimit = 1.0e-8;

    Transform cameraToWorldTransform;
    static double flatCameraTolerance = 1.0e-8;
    DVec3d cameraXVector, cameraYVector, cameraZVector;
    static int debugMode = 0;
    int viewIsFlat = fabs(fraction - 1.0) < flatCameraTolerance;

    if (!pWorldToCameraMap)
        pWorldToCameraMap = &worldToCameraMap;
    if (!pCameraToNpcMap)
        pCameraToNpcMap = &cameraToNpcMap;
    if (!pWorldToNpcMap)
        pWorldToNpcMap = &worldToNpcMap;

        /* slab-world transformation */
    slabToWorldTransform.InitFrom4Points(*pOrigin, *pUPoint, *pVPoint, *pWPoint);
    if (    fabs (fraction) < frustumFractionLimit
        ||  !bsiDMap4d_initFromTransform (&worldToSlabMap,
                        &slabToWorldTransform, true)
        )
        {
        bsiDMap4d_initIdentity (pWorldToCameraMap);
        bsiDMap4d_initIdentity (pWorldToNpcMap);
        bsiDMap4d_initIdentity (pCameraToNpcMap);
        return false;
        }

        /* slab-npc transformation */


    bsiDMap4d_initIdentity (&slabToNpcMap);
    slabToNpcMap.M0.SetRow (2,
                0.0, 0.0, fraction , 0.0 );
    slabToNpcMap.M0.SetRow (3,
                0.0, 0.0, fraction - 1.0 , 1.0 );

    slabToNpcMap.M1.SetRow (2,
                0.0, 0.0, 1.0 / fraction , 0.0 );
    slabToNpcMap.M1.SetRow (3,
                0.0, 0.0, (1.0 - fraction) / fraction , 1.0 );

    if (viewIsFlat)
        {
        MDL_HM_SETTYPE( &slabToNpcMap, MDL_HM_AFFINE_BITS);
        }
    else
        {
        MDL_HM_SETTYPE( &slabToNpcMap, MDL_HM_TYPE_BITS );
        }

    /* world-camera transformation */
    cameraXVector.DifferenceOf (*pUPoint, *pOrigin);
    cameraYVector.DifferenceOf (*pVPoint, *pOrigin);
    cameraZVector.NormalizedCrossProduct (cameraXVector, cameraYVector);
    cameraYVector.NormalizedCrossProduct (cameraZVector, cameraXVector);
    cameraXVector.NormalizedCrossProduct (cameraYVector, cameraZVector);

    DPoint3d cameraTranslation;
    if (viewIsFlat )
        {
        /* Artificially place the camera on the front plane of the frustum */
        slabToWorldTransform.Multiply (cameraTranslation, fraction, fraction, 1.0);
        }
    else
        {
        cameraTranslation.Interpolate (*pOrigin, 1.0 / (1.0 - fraction), *pWPoint);
        }
            
    cameraToWorldTransform.InitFromOriginAndVectors (cameraTranslation, cameraXVector, cameraYVector, cameraZVector);

    bsiDMap4d_initFromTransform (&cameraToWorldMap,
                &cameraToWorldTransform, false);

    bsiDMap4d_invert (pWorldToCameraMap, &cameraToWorldMap);

    bsiDMap4d_multiply (pWorldToNpcMap, &slabToNpcMap, &worldToSlabMap);
    bsiDMap4d_multiply (pCameraToNpcMap,  pWorldToNpcMap, &cameraToWorldMap);
    /* Whew !!! */

    if (debugMode == 1)
        {
        bsiDMap4d_initIdentity (pWorldToCameraMap);
        *pCameraToNpcMap = *pWorldToNpcMap = worldToSlabMap;
        }
    return true;
    }



/*-----------------------------------------------------------------*//**
* There is a simple, direct transformation from world to a perspective
* in a slab.   Unfortunately, camerabased abstraction presented to
* the outside world wants to go there by way of a (rigid) camera.
* So we have to take the long way around the barn to achieve
*<UL>
*<LI> world > camera > slab > perspective slab
*</UL>
* by combining the two easily computed chains:
*<UL>
*<LI> world > camera
*<LI> world > slab > perspective slab
*</UL>

* @param pWorldToCameraMap <= world to camera part of transform. May be NULL pointer.
* @param pCameraToNpcMap <= camera to npc part of transform. May be NULL pointer.
* @param pWorldToNpcMap <= world to npc.  May be NULL pointer
* @param pOrigin => lower left rear of frustum
* @param pUVector => lower left to lower right backface extent
* @param pVVector => lower left to upper left backface extent vector
* @param pWPoint => lower left rear to lower left front extent vector
* @param fraction => front size divided by back size
* @see
* @return true if frustum is well defined
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDMap4d_initFromVectorFrustum


(
DMap4dP        pWorldToNpcMap,
DPoint3dCP pOrigin,
DPoint3dCP pUVector,
DPoint3dCP pVVector,
DPoint3dCP pWVector,
double      fraction
)
    {
    static double s_frustumFractionLimit = 1.0e-8;
    static double s_flatViewTolerance = 1.0e-8;
    DMap4d worldToSlabMap;
    DMap4d slabToNpcMap;
    Transform slabToWorldTransform;

    if (fraction < s_frustumFractionLimit)
        fraction = s_frustumFractionLimit;

        /* slab-world transformation */
    slabToWorldTransform.InitFromOriginAndVectors(*pOrigin, *(DVec3d const*)pUVector, *(DVec3d const*)pVVector, *(DVec3d const*)pWVector);
    if (!bsiDMap4d_initFromTransform (&worldToSlabMap,
                        &slabToWorldTransform, true)
        )
        {
        bsiDMap4d_initIdentity (pWorldToNpcMap);
        return false;
        }

        /* slab-npc transformation */


    if (fabs (fraction - 1.0) > s_flatViewTolerance)
        {
        /* Premulitply by perspective effects matrix, which we build ad mysterium ... */
        bsiDMap4d_initIdentity (&slabToNpcMap);
        slabToNpcMap.M0.SetRow (2,
                    0.0, 0.0, fraction , 0.0 );
        slabToNpcMap.M0.SetRow (3,
                    0.0, 0.0, fraction - 1.0 , 1.0 );

        slabToNpcMap.M1.SetRow (2,
                    0.0, 0.0, 1.0 / fraction , 0.0 );
        slabToNpcMap.M1.SetRow (3,
                    0.0, 0.0, (1.0 - fraction) / fraction , 1.0 );
        MDL_HM_SETTYPE( &slabToNpcMap, MDL_HM_TYPE_BITS );
        bsiDMap4d_multiply (pWorldToNpcMap, &slabToNpcMap, &worldToSlabMap);
        }
    else
        {
        MDL_HM_SETTYPE( &slabToNpcMap, MDL_HM_AFFINE_BITS);
        *pWorldToNpcMap = worldToSlabMap;
        }

    return true;
    }






/*-----------------------------------------------------------------*//**
* Fill a mapping between a unit prism and homogeneous skew space.
* Example use:
* We want to map a unit rectangle from font space to screen, under
* a full perspective mapping.   That is, given font space point (x,y)
* we want A * (x,y,0,1)^T  = the visible pixel.   Also, given pixel
* (i,j) we want Ainverse * (i,j,0,1) to map back into font space.
* A long time ago, we were told points P0,P1,P2 which are the
* preperspective points that correspond to font space (0,0), (1,0), and
* (0,1).
* Since then, P00, P10, P01 have been through a homogeneous transformation.
* (For instance, there may be 'weight' of other than 1 on each one.)
* The transformed (homogeneous) points are Q00, Q10, Q01
* In device space, we do a projection in the z direction.  Hence we
* need a 4th point Qz=(0,0,1,0).
* Build this matrix by calling
* jmdlDMap4d_fillHomogeneousSkewFrame (pHMap, Q00, Q10,Q01,Qz)

* @instance pHMap <= projective transform
* @param pPoint0001 0001 in the unit space maps here
* @param pPoint1001 1001 in the unit space maps here
* @param pPoint0101 0100 in the unit space maps here
* @param pPoint0010 0010 in the unit space maps here
* @see
* @return true if the 4 points are independent
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDMap4d_initHomgeneousSkewFrame

(
DMap4dP pHMap,
DPoint4dCP pPoint0001,
DPoint4dCP pPoint1001,
DPoint4dCP pPoint0101,
DPoint4dCP pPoint0010
)

    {
    pHMap->M0.coff[0][0] = pPoint1001->x - pPoint0001->x;
    pHMap->M0.coff[1][0] = pPoint1001->y - pPoint0001->y;
    pHMap->M0.coff[2][0] = pPoint1001->z - pPoint0001->z;
    pHMap->M0.coff[3][0] = pPoint1001->w - pPoint0001->w;

    pHMap->M0.coff[0][1] = pPoint0101->x - pPoint0001->x;
    pHMap->M0.coff[1][1] = pPoint0101->y - pPoint0001->y;
    pHMap->M0.coff[2][1] = pPoint0101->z - pPoint0001->z;
    pHMap->M0.coff[3][1] = pPoint0101->w - pPoint0001->w;

    pHMap->M0.coff[0][2] = pPoint0010->x;
    pHMap->M0.coff[1][2] = pPoint0010->y;
    pHMap->M0.coff[2][2] = pPoint0010->z;
    pHMap->M0.coff[3][2] = pPoint0010->w;

    pHMap->M0.coff[0][3] = pPoint0001->x;
    pHMap->M0.coff[1][3] = pPoint0001->y;
    pHMap->M0.coff[2][3] = pPoint0001->z;
    pHMap->M0.coff[3][3] = pPoint0001->w;

    if (bsiDMatrix4d_invertQR(&pHMap->M1, &pHMap->M0))
        {
        MDL_HM_SETTYPE( pHMap, MDL_HM_TYPE_BITS );
        return true;
        }
    else
        {
        bsiDMap4d_initIdentity (pHMap);
        return false;
        }
    }





/*-----------------------------------------------------------------*//**
*
* @instance pHMap <= projective transform
* @param z0 => reference z.  Normalized projective coordinate
                                is 0 at this z
* @param zetaHalf => controls rate of growth of normalized
                                projective z.   Projective z at z0/k
                                is k-1*zetahalf, i.e. is zetahalf at
                                z0/2
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMap4d_zFrustum

(
DMap4dP  pHMap,
double z0,
double zetaHalf
)

    {
    pHMap->M0.InitIdentity ();
    pHMap->M1.InitIdentity ();
    pHMap->M0.coff[2][2] = -zetaHalf;
    pHMap->M0.coff[2][3] = zetaHalf*z0;
    pHMap->M0.coff[3][2] = 1.0;
    pHMap->M0.coff[3][3] = 0.0;

    pHMap->M1.coff[2][2] = 0.0;
    pHMap->M1.coff[3][3] = 1.0/z0;
    pHMap->M1.coff[3][2] = 1.0/(zetaHalf*z0);
    pHMap->M1.coff[2][3] = 1.0;
    pHMap->mask = 0;
    MDL_HM_SETTYPE( pHMap, MDL_HM_TYPE_BITS );
    }


/*-----------------------------------------------------------------*//**
* Build a transform that maps a perspective frustum to a rectilinear
* box.  If either point of the box is NULL, the box defaults to
* 1..+1 in all dimensions.

* @param pHMap <= DMap4d structure to be filled
* @param pCamera => camera point origin of viewing system
* @param pTarget => target point
                                where viewplane intersects -z axis
* @param pUpVector => camera up vector
* @param pViewplaneWindow => viewplane clip window.  Only xy parts are used.
* @param dFront => unsigned distance to front clip plane
* @param dBack => unsigned distance to back clip plane
* @param pLoPoint => the xyz coordinates that are to be the
                                lower left in transformed space
* @param pHiPoint => the xyz coordinates that are to be the
                                upper right in transformed space
* @see
* @return int
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDMap4d_initFromCameraParameters

(
DMap4dP  pHMap,
DPoint3dCP pCamera,
DPoint3dCP pTarget,
DPoint3dCP pUpVector,
DRange3dCP pViewplaneWindow,
double dFront,
double dBack,
DPoint3dCP pLoPoint,
DPoint3dCP pHiPoint
)
    {
    double dTarget;
    DPoint3d xVec,yVec,zVec;
    DPoint3d originFromCamera;  /* coordinates of world origin relative
                                        to camera system */
    DMap4d dollyMap, cameraMap,scaleMap, cameraOnDollyMap;
    bool    boolStat = false;
    DPoint3d glLL, glUR;

    glLL.Init ( -1.0, -1.0, -1.0 );
    glUR.Init (  1.0,  1.0,  1.0 );

    yVec = *pUpVector;

    if (   0.0 < (dTarget = zVec.NormalizedDifference (*pTarget, *pCamera))
        && 0.0 < yVec.Normalize ()
        && 0.0 < xVec.NormalizedCrossProduct (yVec, zVec)
        && 0.0 < yVec.NormalizedCrossProduct (zVec, xVec)
        && dFront > 0.0
        && dBack > 0.0
        && dBack != dFront
        && pViewplaneWindow->low.x != pViewplaneWindow->high.x
        && pViewplaneWindow->low.y != pViewplaneWindow->high.y
        && ( !pLoPoint || !pHiPoint ||
                pLoPoint->Distance (*pHiPoint) != 0.0 )
       )
        {
        originFromCamera.x = - pCamera->DotProduct (xVec);
        originFromCamera.y = - pCamera->DotProduct (yVec);
        originFromCamera.z = - pCamera->DotProduct (zVec);
        bsiDMatrix4d_initAffineRows(
                        &dollyMap.M0, &xVec, &yVec, &zVec, &originFromCamera );
        bsiDMatrix4d_initAffineColumns(
                        &dollyMap.M1, &xVec, &yVec, &zVec, pCamera);
        dollyMap.mask = 0;
        MDL_HM_SETTYPE( &dollyMap,
                MDL_HM_FRAME_BIT | MDL_HM_TRANSLATE_BIT | MDL_HM_SCALE_BIT
                        | MDL_HM_PERSPECTIVE_BIT );

        bsiDMap4d_glFrustum( &cameraMap,
                pViewplaneWindow->low.x,
                pViewplaneWindow->high.x,
                pViewplaneWindow->low.y,
                pViewplaneWindow->high.y,
                dFront, dBack
                );
        bsiDMap4d_multiply( &cameraOnDollyMap, &cameraMap, &dollyMap );
        if ( pLoPoint && pHiPoint )
            {
            bsiDMap4d_initBoxMap( &scaleMap,
                &glLL, &glUR, pLoPoint, pHiPoint );
            bsiDMap4d_multiply( pHMap,
                &scaleMap, &cameraOnDollyMap );
            }
       else
            {
            *pHMap = cameraOnDollyMap;
            }
        boolStat = true;
        }
    else
        {
        bsiDMap4d_initIdentity ( pHMap );
        }
    return boolStat;
    }
/*----------------------------------------------------------------------+
|SUBSECTION operations Inversion and multiplication operations
+----------------------------------------------------------------------*/




/*-----------------------------------------------------------------*//**
* Sets pA to the inverse of pB.

* @instance pA <= inverted mapping
* @param pB => original mapping
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMap4d_invert

(
DMap4dP pA,
DMap4dCP pB
)

    {
    DMap4d C;
    C = *pB;
    pA->mask = C.mask;
    pA->M1 = C.M0;
    pA->M0 = C.M1;
    }





/*-----------------------------------------------------------------*//**
* Form the product
*   C * Binv * A * B * Cinv
* A NULL for B or C skips that part.

* @instance pResult <= product DMap4d Binv*A*B
* @param pA => inside term of sandwich
* @param pB => middle term of sandwich
* @param pC => outer term of sandwich
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMap4d_sandwich

(
DMap4dP  pResult,
DMap4dCP  pA,
DMap4dCP  pB,
DMap4dCP  pC
)

    {
    DMap4d inverse, product;
    if (pB && pC)
        {
        /* rather than fuss with best  way to dovetail double application*/
        /* with singles, just recurse twice with NULL parameters to get the singles.*/
        bsiDMap4d_sandwich (&product, pA, pB, NULL);
        bsiDMap4d_sandwich (pResult, &product, NULL, pC);
        }
    else if (pB)
        {
        bsiDMap4d_invert ( &inverse, pB);
        bsiDMap4d_multiply ( &product, pA, pB );
        bsiDMap4d_multiply ( pResult, &inverse, &product);
        }
    else if (pC)
        {
        bsiDMap4d_invert ( &inverse, pC);
        bsiDMap4d_multiply ( &product, pC, pA );
        bsiDMap4d_multiply ( pResult, &product, &inverse);
        }
    else
        {
        *pResult = *pA;
        }
    }




/*-----------------------------------------------------------------*//**
* Multiply transforms
*
* @instance pC <= product A * B
* @param pA => transform A
* @param pB => transform B
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMap4d_multiply

(
DMap4dP pC,
DMap4dCP pA,
DMap4dCP pB
)

    {
    int frameBit, translateBit, scaleBit, perspectiveBit;

    /* Multiply the forward matrices .. */
    pC->M0.InitProduct (pA->M0, pB->M0);


    /* then the (reversed) inverses .. */
    pC->M1.InitProduct (pB->M1, pA->M1);


    /* and symbolically multiply the bits for non-identity entries */
    frameBit = MDL_HM_BIT_DOT( MDL_HM_FRAME_BIT,
                pA,MDL_HM_FRAME_BIT, MDL_HM_TRANSLATE_BIT,
                pB,MDL_HM_FRAME_BIT, MDL_HM_PERSPECTIVE_BIT );
    translateBit = MDL_HM_BIT_DOT( MDL_HM_TRANSLATE_BIT,
                pA,MDL_HM_FRAME_BIT, MDL_HM_TRANSLATE_BIT,
                pB,MDL_HM_TRANSLATE_BIT, MDL_HM_SCALE_BIT );
    /*
    perspectiveBit = MDL_HM_BIT_DOT( MDL_HM_PERSPECTIVE_BIT,
                pA,MDL_HM_PERSPECTIVE_BIT, MDL_HM_SCALE_BIT,
                pB,MDL_HM_FRAME_BIT, MDL_HM_PERSPECTIVE_BIT );
    scaleBit = MDL_HM_BIT_DOT( MDL_HM_SCALE_BIT,
                pA,MDL_HM_PERSPECTIVE_BIT, MDL_HM_SCALE_BIT,
                pB,MDL_HM_TRANSLATE_BIT, MDL_HM_SCALE_BIT );
    */
    perspectiveBit = (   pC->M0.coff[3][0] != 0.0
                      || pC->M0.coff[3][1] != 0.0
                      || pC->M0.coff[3][2] != 0.0
                     )
                     ? MDL_HM_PERSPECTIVE_BIT : 0;

    scaleBit       =    pC->M0.coff[3][3] != 1.0
                     ? MDL_HM_SCALE_BIT : 0;

    pC->mask = 0;
    MDL_HM_SETTYPE( pC,
        frameBit | perspectiveBit | translateBit | scaleBit );
    if  (    MDL_HM_GETMASK(pA, MDL_HM_SINGULAR_BIT)
          || MDL_HM_GETMASK(pB, MDL_HM_SINGULAR_BIT)
        )
        {
        MDL_HM_SETMASK(pC,MDL_HM_SINGULAR_BIT);
        }

    }




/*-----------------------------------------------------------------*//**
* Multiply transforms, selecting optional forward or inverse for each

* @instance pC <= product
* @param pA => transform A
* @param invertA => if true, use invese of A
* @param pB => transform B
* @param invertB => if true, use invese of B
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMap4d_multiplyInverted

(
DMap4dP pC,
DMap4dCP pA,
bool        invertA,
DMap4dCP pB,
bool        invertB
)

    {
    static double zeroTol = 1.0e-14;
    int frameBit, translateBit, scaleBit, perspectiveBit;

    pC->M0.InitProduct (*(invertA ? &pA->M1 : &pA->M0), *(invertB ? &pB->M1 : &pB->M0));

    pC->M1.InitProduct (*(invertB ? &pB->M0 : &pB->M1), *(invertA ? &pA->M0 : &pA->M1));

    /* and symbolically multiply the bits for non-identity entries */
    frameBit = MDL_HM_BIT_DOT( MDL_HM_FRAME_BIT,
                pA,MDL_HM_FRAME_BIT, MDL_HM_TRANSLATE_BIT,
                pB,MDL_HM_FRAME_BIT, MDL_HM_PERSPECTIVE_BIT );
    translateBit = MDL_HM_BIT_DOT( MDL_HM_TRANSLATE_BIT,
                pA,MDL_HM_FRAME_BIT, MDL_HM_TRANSLATE_BIT,
                pB,MDL_HM_TRANSLATE_BIT, MDL_HM_SCALE_BIT );

    perspectiveBit = (   fabs(pC->M0.coff[3][0]) > zeroTol
                      || fabs(pC->M0.coff[3][1]) > zeroTol
                      || fabs(pC->M0.coff[3][2]) > zeroTol
                     )
                     ? MDL_HM_PERSPECTIVE_BIT : 0;

    scaleBit       =    fabs(pC->M0.coff[3][3] - 1.0) > zeroTol
                     ? MDL_HM_SCALE_BIT : 0;

    pC->mask = 0;
    MDL_HM_SETTYPE( pC,
        frameBit | perspectiveBit | translateBit | scaleBit );
    if  (    MDL_HM_GETMASK(pA, MDL_HM_SINGULAR_BIT)
          || MDL_HM_GETMASK(pB, MDL_HM_SINGULAR_BIT)
        )
        {
        MDL_HM_SETMASK(pC,MDL_HM_SINGULAR_BIT);
        }

    }


/*----------------------------------------------------------------------+
|SUBSECTION queries Inquiring properties of transformations
+----------------------------------------------------------------------*/



/*-----------------------------------------------------------------*//**
* Checks if the mapping is strictly a scale/translate in specified
* directions.

* @instance pA => Transform to test
* @param xChange => 1 if x may change, 0 if it must stay fixed
* @param yChange => 1 if y may change, 0 if it must stay fixed
* @param zChange => 1 if z may change, 0 if it must stay fixed
* @see
* @return true if independence tests are satisfied.
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDMap4d_isIndependent

(
DMap4dP    pA,
int     xChange,
int     yChange,
int     zChange
)

    {
    int changeAxis[3];
    int i;
    int j;
    double diag, translate;
    /* Toss perspective and overall scale */
    if (   pA->M0.coff[3][0] != 0.0
        || pA->M0.coff[3][1] != 0.0
        || pA->M0.coff[3][2] != 0.0
        || pA->M0.coff[3][3] != 1.0
        )
        return false;

    changeAxis[0] = xChange;
    changeAxis[1] = yChange;
    changeAxis[2] = zChange;

    for (i = 0; i < 3; i++)
        {
        diag = pA->M0.coff[i][i];
        translate = pA->M0.coff[i][3];
        if (   !changeAxis[i]
            && ( diag != 1.0 || translate != 0.0 )
            )
            return false;
        if ( changeAxis[i] && diag == 0.0 )
            return false;
        for (j = 0; j < 3; j++)
            {
            if (i != j && pA->M0.coff[i][j] != 0.0)
                return false;
            }
        }
    return true;
    }



/*-----------------------------------------------------------------*//**
* Test if a transform is singular

* @instance pHMap => transform to test
* @see
* @return true if the mapping is singular
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDMap4d_isSingular

(
DMap4dCP  pHMap
)
    {
    return ( MDL_HM_GETMASK(pHMap, MDL_HM_SINGULAR_BIT ) ? 1 : 0 );
    }




/*-----------------------------------------------------------------*//**
* Test if a transform is affiine.

* @instance pHMap => transform to test
* @see
* @return true if the mapping is affine.
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDMap4d_isAffine

(
DMap4dCP pHMap
)
    {
    return MDL_HM_IS_AFFINE_MASK( pHMap->mask );
    }



/*-----------------------------------------------------------------*//**
* Test if a transform is perspective.

* @instance pHMap => transform to test
* @see
* @return true if the mapping contains perspective.
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDMap4d_isPerspective

(
DMap4dCP pHMap
)
    {
    return 0 != MDL_HM_IS_PERSPECTIVE_MASK( pHMap->mask );
    }





/*-----------------------------------------------------------------*//**
*
* @return size required to allocate a DMap4d
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDMap4d_size

(
)
    {
    return sizeof(DMap4d);
    }




/*-----------------------------------------------------------------*//**
*
* @param => mapping to test
* @see
* @return true if the mapping is an identity
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDMap4d_isIdentity

(
DMap4dCP pInstance
)
    {
    return    pInstance->M0.IsIdentity ()
           && pInstance->M1.IsIdentity ();
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlHMap_fillFromMatrices                               |
|                                                                       |
| author        EarlinLutz                                      7/96    |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    bsiDMap4d_initFromMatrices

(
DMap4dP pHMap,     /* <= transform */
DMatrix4dCP pM0,       /* => forward matrix */
DMatrix4dCP pM1     /* => inverse matrix.  Caller is responsible for accuracy!!! */
)
    {
    pHMap->M0 = *pM0;
    pHMap->M1 = *pM1;
    MDL_HM_SETTYPE( pHMap, MDL_HM_TYPE_BITS );
    return true;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlHMap_fillFromIndexedMatrices                        |
|                                                                       |
| author        EarlinLutz                                      7/96    |
|                                                                       |
| Initialize a transform with perspective entries for a (nonzero)       |
| taper in the z direction.                                             |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    bsiDMap4d_initFromIndexedMatrices

(
DMap4dP pHMap,     /* <= transform */
DMatrix4dCP pM0,       /* => forward matrix */
DMatrix4dCP pM1,            /* => inverse matrix.  Caller is responsible for accuracy!!! */
const int       *pColIndex  /* => indices of columns in M0, rows in M1. */
)
    {

    int i, j, k;
    for (k = 0; k < 4; k++)
        {
        j = pColIndex[k];
        for (i = 0; i < 4; i++)
            {
            pHMap->M0.coff[i][k] = pM0->coff[i][j];
            pHMap->M1.coff[k][i] = pM1->coff[j][i];
            }
        }
    MDL_HM_SETTYPE( pHMap, MDL_HM_TYPE_BITS );
    return true;
    }

#ifdef TESTPROGRAM
/* test commands:
   identity - push identity matrix on stack
*/
#define STACK_LIMIT 100
static int stackSize = 0;
static DMap4d stackMatrix[STACK_LIMIT];
void hStack_init

(
void
)
    {
    stackSize = 0;
    }

void hStack_push

(
DMap4dP  pHMap
)
    {
    if( stackSize < STACK_LIMIT )
        {
        stackMatrix[stackSize++] = *pHMap;
        }
    }

void hStack_pop

(
DMap4dP  pHMap
)
    {
    if ( stackSize > 0 )
        {
        *pHMap = stackMatrix[--stackSize];
        }
    else
        {
        bsiDMap4d_initIdentity( pHMap );
        }
    }

void hStack_peek

(
DMap4dP  pHMap
)
    {
    if ( stackSize > 0 )
        {
        *pHMap = stackMatrix[stackSize - 1];
        }
    else
        {
        bsiDMap4d_initIdentity( pHMap );
        }
    }

int getDouble

(
FILE *fp,
double *pX,
char *sx
)
    {
    if( fp == stdin )
        {
        printf("%s: ",sx);
        }
    return fscanf(fp,"%lf", pX) == 1;
    }

FILE *getFile

(
FILE *fp,
char *pPrompt
)
    {
    char fileName[1024];
    FILE *fp1 = NULL;
    if ( fp == stdin )
        printf("%s: ",pPrompt);
    if ( fscanf( fp, "%s" ,fileName ) == 1 )
        {
        fp1 = fopen(fileName, "r");
        }
    return fp1;
    }

int getDPoint3d

(
FILE *fp,
DPoint3dP pPoint,
char *name
)
    {
    if( fp == stdin )
        {
        printf("%s: ",name);
        }
    return fscanf(fp,"%lf %lf %lf", &pPoint->x, &pPoint->y, &pPoint->z ) == 3;
    }

int getVector

(
FILE *fp,
double *pX,
double *pY,
double *pZ,
char *sx,
char *sy,
char *sz
)
    {
    if( fp == stdin )
        {
        printf("%s %s %s: ",sx,sy,sz);
        }
    return fscanf(fp,"%lf %lf %lf", pX, pY, pZ) == 3;
    }

void process(FILE *fp)
    {
    char command[256];
    double vx,vy,vz,vw,degrees;
    DPoint3d camera, upVector,target;
    DPoint3d loA,hiA,loB,hiB;
    DMap4d A,B,C;
    FILE *fp1;
    double vL,vR,vB,vT,vN,vF,vh, vxr,vyr,vzr;
    while( fscanf(fp,"%s",command) == 1 )
        {
        if( 0 == strcmp( command, "identity" ) )
            {
            bsiDMap4d_initIdentity(&A);
            hStack_push(&A);
            }
        else if ( 0 == strcmp( command, "map" )
                && getVector(fp,&vx,&vy,&vz,"px","py","pz")
                )
            {
            DPoint4d P,Q;
            P.x = vx;
            P.y = vy;
            P.z = vz;
            P.w = 1.0;
            hStack_peek(&A);
            bsiDMatrix4d_multiplyMatrixPoint (&A.M0, &Q, &P);
            printf("%lf %lf %lf %lf\n",Q.x,Q.y,Q.z,Q.w);
            }
        else if ( 0 == strcmp( command, "map4" )
                && getVector(fp,&vx,&vy,&vz,"px","py","pz")
                && getDouble( fp, &vw,"w")
                )
            {
            DPoint4d P,Q;
            P.x = vx;
            P.y = vy;
            P.z = vz;
            P.w = 1.0;
            hStack_peek(&A);
            bsiDMatrix4d_multiplyMatrixPoint (&A.M0, &Q, &P);
            printf("%lf %lf %lf %lf => ",P.x,P.y,P.z,P.w);
            printf("%lf %lf %lf %lf\n",Q.x,Q.y,Q.z,Q.w);
            if ( Q.w != 0.0 )
                printf("%lf %lf %lf %lf\n",
                        Q.x/Q.w,Q.y/Q.w,Q.z/Q.w,Q.w/Q.w);
            }
        else if ( 0 == strcmp( command, "inv4" )
                && getVector(fp,&vx,&vy,&vz,"px","py","pz")
                && getDouble( fp, &vw,"w")
                )
            {
            DPoint4d P,Q;
            P.x = vx;
            P.y = vy;
            P.z = vz;
            P.w = 1.0;
            hStack_peek(&A);
            bsiDMatrix4d_multiplyMatrixPoint (&A.M1, &Q, &P);
            printf("%lf %lf %lf %lf => ",P.x,P.y,P.z,P.w);
            printf("%lf %lf %lf %lf\n",Q.x,Q.y,Q.z,Q.w);
            if ( Q.w != 0.0 )
                printf("%lf %lf %lf %lf\n",
                        Q.x/Q.w,Q.y/Q.w,Q.z/Q.w,Q.w/Q.w);
            }
        else if ( 0 == strcmp( command, "*" ) )
            {
            hStack_pop(&B);
            hStack_pop(&A);
            bsiDMap4d_multiply( &C, &A, &B );
            hStack_push(&C);
            }
        else if ( 0 == strcmp( command, "pop" ) )
            {
            hStack_pop(&B);
            }
        else if ( 0 == strcmp( command, "dup" ) )
            {
            hStack_peek(&B);
            hStack_push(&B);
            }
        else if ( 0 == strcmp( command, "translate" )
                && getVector(fp,&vx,&vy,&vz,"tx","ty","tz")
                )
            {
            bsiDMap4d_initTranslate(&A,vx,vy,vz);
            hStack_push(&A);
            }
        else if ( 0 == strcmp( command, "perspective" )
                && getVector(fp,&vx,&vy,&vz,"px","py","pz")
                )
            {
            jmdlDMap4d_fillPerspective(&A,vx,vy,vz);
            hStack_push(&A);
            }
        else if ( 0 == strcmp( command, "scale" )
                && getVector(fp,&vx,&vy,&vz,"ax","ay","az")
                )
            {
            bsiDMap4d_initScale(&A,vx,vy,vz);
            hStack_push(&A);
            }
        else if ( 0 == strcmp( command, "boxes" )
                && getDPoint3d( fp, &loA,"loA")
                && getDPoint3d( fp, &hiA,"hiA")
                && getDPoint3d( fp, &loB,"loB")
                && getDPoint3d( fp, &hiB,"hiB")
                )
            {
            if ( bsiDMap4d_initBoxMap( &A, &loA, &hiA, &loB, &hiB ) )
                {
                hStack_push( &A );
                }
            }
        else if ( 0 == strcmp( command, "glbox" )
                && getDPoint3d( fp, &loB,"loB")
                && getDPoint3d( fp, &hiB,"hiB")
                )
            {
            loA.x = loA.y = loA.z = -1.0;
            hiA.x = hiA.y = hiA.z =  1.0;
            if ( bsiDMap4d_initBoxMap( &A, &loA, &hiA, &loB, &hiB ) )
                {
                hStack_push( &A );
                }
            }
        else if ( 0 == strcmp( command, "camera" )
                && getVector(fp, &camera.x, &camera.y, &camera.z,
                                "Cx", "Cy", "Cz" )
                && getVector(fp, &target.x, &target.y, &target.z,
                                "Tx", "Ty", "Tz" )
                && getVector(fp, &upVector.x, &upVector.y, &upVector.z,
                                "Ux", "Uy", "Uz" )
                && getDouble( fp, &vL,"left")
                && getDouble( fp, &vR,"right")
                && getDouble( fp, &vB,"bottom")
                && getDouble( fp, &vT,"top")
                && getDouble( fp, &vN,"near")
                && getDouble( fp, &vF,"far")
                )
            {
            DRange3d window;
            DPoint3d loPoint,hiPoint;
            loPoint.Init ( 0.0, 0.0, 0.0);
            hiPoint.Init ( 1.0, 1.0, 1.0);
            window.low.x = vL;
            window.low.y = vB;
            window.high.x = vR;
            window.high.y = vT;
            if ( boolean_bsiDMap4d_initFromCameraParameters ( &A,
                                &camera, &target, &upVector,
                                &window,
                                vN, vF,
                                &loPoint, &hiPoint)
                )
                {
                hStack_push(&A);
                }
            else
                {
                printf(" Camera parameters rejected \n");
                }
            }
        else if ( 0 == strcmp( command, "glFrustum" )
                && getDouble( fp, &vL,"left")
                && getDouble( fp, &vR,"right")
                && getDouble( fp, &vB,"bottom")
                && getDouble( fp, &vT,"top")
                && getDouble( fp, &vN,"near")
                && getDouble( fp, &vF,"far")
                )
            {
            bsiDMap4d_glFrustum( &A, vL, vR, vB, vT, vN, vF );
            hStack_push(&A);
            }
        else if ( 0 == strcmp( command, "zFrustum" )
                && getDouble( fp, &vz,"vz")
                && getDouble( fp, &vh,"vh")
                )
            {
            bsiDMap4d_zFrustum ( &A, vz, vh);
            hStack_push(&A);
            }
        else if ( 0 == strcmp( command, "glOrtho" )
                && getDouble( fp, &vL,"left")
                && getDouble( fp, &vR,"right")
                && getDouble( fp, &vB,"bottom")
                && getDouble( fp, &vT,"top")
                && getDouble( fp, &vN,"near")
                && getDouble( fp, &vF,"far")
                )
            {
            bsiDMap4d_glOrtho ( &A, vL,vR,vB,vT,vN,vF);
            hStack_push(&A);
            }
        else if ( 0 == strcmp( command, "rotate" )
                && getDouble( fp, &degrees,"degrees")
                && getVector(fp, &vx,&vy,&vz,"ax","ay","az")
                )
            {
            double radians = degrees * atan(1.0) / 45.0;
            bsiDMap4d_initRotate(&A,radians,vx,vy,vz);
            hStack_push(&A);
            }
        else if ( 0 == strcmp( command, "check" ) )
            {
            DMatrix4d P,Q,D;
            int i;
            hStack_peek ( &A );
            P.InitProduct (A.M0, A.M1);
            Q.InitIdentity ();
            bsiDMatrix4d_subtract(&D,&P,&Q);
            printf("========== inverse error norm %le \n",
                        bsiDMatrix4d_RMS(&D));
            for( i=0; i<4; i++ )
                {
                printf("\t%8.5lf %8.5lf %8.5lf %10.3lf\n",
                        P.coff[i][0],P.coff[i][1],P.coff[i][2],P.coff[i][3]);
                }
            }
        else if ( 0 == strcmp( command, "print" ) )
            {
            int i;
            hStack_peek(&A);
            printf("========== matrix codes %x\n",
                                A.mask );
            for( i=0; i<4; i++ )
                {
                printf("\t%8.5lf %8.5lf %8.5lf %10.3lf\n",
                        A.M0.coff[i][0],A.M0.coff[i][1],A.M0.coff[i][2],A.M0.coff[i][3]);
                }
            if( !bsiDMap4d_isSingular(&A))
                {
                printf("\n");
                for( i=0; i<4; i++ )
                    {
                    printf("\t%8.5lf %8.5lf %8.5lf %10.3lf\n",
                        A.M1.coff[i][0],A.M1.coff[i][1],A.M1.coff[i][2],A.M1.coff[i][3]);
                    }
                }
            }
        else if (
                  (
                     strcmp( command, "include") == 0
                  || strcmp( command, "inc") == 0
                  )
                && ( fp1 = getFile(fp,"include file name") )
                )
            {
            process (fp1);
            fclose(fp1);
            }
        else
            {
            printf(" ?? %s\n", command );
            }
        }
    }

int main()
    {
    hStack_init();
    process(stdin);
    return 0;
    }
#endif
END_BENTLEY_GEOMETRY_NAMESPACE
