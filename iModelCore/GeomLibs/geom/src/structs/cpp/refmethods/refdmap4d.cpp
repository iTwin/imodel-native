/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/cpp/refmethods/refdmap4d.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE



/*-----------------------------------------------------------------*//**
*
* Initialize and identity mapping.
*
+----------------------------------------------------------------------*/
DMap4d DMap4d::FromIdentity
(
)
    {
    DMap4d map;
    map.InitIdentity ();
    return map;
    }

/*-----------------------------------------------------------------*//**
* Initialize a rotation about axis vx,vy,vz by angle whose cosine
* and sine are (proportional to) c and s.
*
* @param [in] c cosine of angle
* @param [in] s sine of angle
* @param [in] vx x component of rotation axis
* @param [in] vy y component of rotation axis
* @param [in] vz z component of rotation axis
+----------------------------------------------------------------------*/
DMap4d DMap4d::FromRotation
(
double          c,
double          s,
double          vx,
double          vy,
double          vz
)
    {
    DMap4d map;
    map.InitFromRotation (c, s, vx, vy, vz);
    return map;
    }

/*-----------------------------------------------------------------*//**
* Rotate about vx,yv,vz by an integer multiple of 90 degrees.
* Providing the angle as an integer allows exact table lookup without
* approximation of pi.
*
* @param [in] multiple rotation angle is multiple * 90 degrees
* @param [in] vx x component of rotation axis
* @param [in] vy y component of rotation axis
* @param [in] vz z component of rotation axis
+----------------------------------------------------------------------*/
DMap4d DMap4d::FromQuadrantRotation
(
int             multiple,
double          vx,
double          vy,
double          vz
)
    {
    DMap4d map;
    map.InitFromQuadrantRotation (multiple, vx, vy, vz);
    return map;
    }

/*-----------------------------------------------------------------*//**
* Initialize a rotation by angle in radians about axis vx,vy,vz

* @param [in] radians angle in degrees
* @param [in] vx x component of rotation axis
* @param [in] vy y component of rotation axis
* @param [in] vz z component of rotation axis
+----------------------------------------------------------------------*/
DMap4d DMap4d::FromRotation
(
double          radians,
double          vx,
double          vy,
double          vz
)
    {
    DMap4d map;
    map.InitFromRotation (radians, vx, vy, vz);
    return map;
    }

/*-----------------------------------------------------------------*//**
* Initialize a pure scaling transformation.
* If any scale factor is zero, the corresponding inverse entry is also
* zero.

* @param [in] ax x scale factor
* @param [in] ay y scale factor
* @param [in] az z scale factor
+----------------------------------------------------------------------*/
DMap4d DMap4d::FromScale
(
double          ax,
double          ay,
double          az
)
    {
    DMap4d map;
    map.InitFromScale (ax, ay, az);
    return map;
    }

/*-----------------------------------------------------------------*//**
* Initialize a transform that is a (noninvertible) projection to a
* principle plane.

* @param [in] height distance of plane from origin
* @param [in] axis 0,1,2 for x,y,z normal
+----------------------------------------------------------------------*/
DMap4d DMap4d::FromPrincipleProjection
(
double          height,
int             axis
)
    {
    DMap4d map;
    map.InitFromPrincipleProjection (height, axis);
    return map;
    }

/*__PUBLISH_SECTION_END__*/

/*-----------------------------------------------------------------*//**
* Initialize a transform that incorporates a given origin, x direction, and additional
* local transformation, all relative to a given transformation.
* (This is the guts of AgentContext::PushTransformTo())
* @param [in] origin world origin
* @param [in] xDir world x direction
* @param [in] pWorldToTarget map from world to target system
+----------------------------------------------------------------------*/
DMap4d DMap4d::FromTransformedOriginAndDirection
(
DPoint3dCR      origin,
DPoint3dCR      xDir,
DMap4dCR        targetToWorld,
DMap4dCR        localFrame
)
    {
    DMap4d map;
    map.InitFromTransformedOriginAndDirection (origin, xDir, targetToWorld, localFrame);
    return map;
    }
/*__PUBLISH_SECTION_START__*/


/*-----------------------------------------------------------------*//**
* Initialize a uniform scaling transformation.

* @param [in] a scale factor
+----------------------------------------------------------------------*/
DMap4d DMap4d::FromScale
(
double          a
)
    {
    DMap4d map;
    map.InitFromScale (a);
    return map;
    }

/**


Initialize a translation transform.

* @param [in] tx x component of translation
* @param [in] ty y component of translation
* @param [in] tz z component of translation
+----------------------------------------------------------------------*/
DMap4d DMap4d::FromTranslation
(
double          tx,
double          ty,
double          tz
)
    {
    DMap4d map;
    map.InitFromTranslation (tx, ty, tz);
    return map;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DMap4d::InitIdentity ()
    {
    bsiDMap4d_initIdentity (this);
    }





/*-----------------------------------------------------------------*//**
* Initialize a transform which translates and scales along principle
* axes so box loAP..hiAP maps to box loBP..hiBP
*
* @param [in] loAP corner of box A
* @param [in] hiAP diagonally opposite corner of box A
* @param [in] loBP corner of box B
* @param [in] hiBP diagonally opposite corner of box B
* @return int
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DMap4d::InitFromRanges
(

DPoint3dCR loAP,
DPoint3dCR hiAP,
DPoint3dCR loBP,
DPoint3dCR hiBP

)
    {
    return bsiDMap4d_initBoxMap (this, (&loAP),(&hiAP),(&loBP),(&hiBP)) ? true : false;
    }


/*-----------------------------------------------------------------*//**
* Initialize a rotation about axis vx,vy,vz by angle whose cosine
* and sine are (proportional to) c and s.
*
* @param [in] c cosine of angle
* @param [in] s sine of angle
* @param [in] vx x component of rotation axis
* @param [in] vy y component of rotation axis
* @param [in] vz z component of rotation axis
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMap4d::InitFromRotation
(

double c,
double s,
double vx,
double vy,
double vz

)
    {
    bsiDMap4d_initRotateTrig (this, c,s,vx,vy,vz);
    }


/*-----------------------------------------------------------------*//**
* Rotate about vx,yv,vz by an integer multiple of 90 degrees.
* Providing the angle as an integer allows exact table lookup without
* approximation of pi.
*
* @param [in] multiple rotation angle is multiple * 90 degrees
* @param [in] vx x component of rotation axis
* @param [in] vy y component of rotation axis
* @param [in] vz z component of rotation axis
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMap4d::InitFromQuadrantRotation
(

int     multiple,
double vx,
double vy,
double vz

)
    {
    bsiDMap4d_initRotateQuadrant (this, multiple,vx,vy,vz);
    }


/*-----------------------------------------------------------------*//**
* Initialize a rotation by angle in radians about axis vx,vy,vz

* @param [in] radians angle in degrees
* @param [in] vx x component of rotation axis
* @param [in] vy y component of rotation axis
* @param [in] vz z component of rotation axis
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMap4d::InitFromRotation
(

double radians,
double vx,
double vy,
double vz

)
    {
    bsiDMap4d_initRotate (this, radians,vx,vy,vz);
    }


/*-----------------------------------------------------------------*//**
* Initialize a pure scaling transformation.
* If any scale factor is zero, the corresponding inverse entry is also
* zero.

* @param [in] ax x scale factor
* @param [in] ay y scale factor
* @param [in] az z scale factor
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMap4d::InitFromScale
(

double ax,
double ay,
double az

)
    {
    bsiDMap4d_initScale (this, ax,ay,az);
    }


/*-----------------------------------------------------------------*//**
* Initialize a transform that is a (noninvertible) projection to a
* principle plane.

* @param [in] height distance of plane from origin
* @param [in] axis 0,1,2 for x,y,z normal
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMap4d::InitFromPrincipleProjection
(

double height,
int    axis

)
    {
    bsiDMap4d_initPrincipleProjection (this, height,axis);
    }


/*-----------------------------------------------------------------*//**
* Initialize a transform that incorporates a given origin, x direction, and additional
* local transformation, all relative to a given transformation.
* (This is the guts of AgentContext::PushTransformTo())
* @param [in] origin world origin
* @param [in] xDir world x direction
* @param [in] pWorldToTarget map from world to target system
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMap4d::InitFromTransformedOriginAndDirection
(

DPoint3dCR origin,
DPoint3dCR xDir,
DMap4dCR targetToWorld,
DMap4dCR localFrame

)
    {
    bsiDMap4d_initFromTransformedOriginAndDirection (this, (&origin),(&xDir),(&targetToWorld),(&localFrame));
    }


/*-----------------------------------------------------------------*//**
* Initialize a uniform scaling transformation.

* @param [in] a scale factor
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMap4d::InitFromScale
(

double a

)
    {
    bsiDMap4d_initUniformScale (this, a);
    }


/**


Initialize a translation transform.

* @param [in] tx x component of translation
* @param [in] ty y component of translation
* @param [in] tz z component of translation
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMap4d::InitFromTranslation
(

double tx,
double ty,
double tz

)
    {
    bsiDMap4d_initTranslate (this, tx,ty,tz);
    }


/*-----------------------------------------------------------------*//**
* Initialize a transform with perspective entries for a (nonzero)
* taper in the z direction.

* @param [in] taper taper fraction
* @return true if an invertible map was constructed.
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DMap4d::InitFromTaper
(

double      taper

)
    {
    return bsiDMap4d_initTaper (this, taper) ? true : false;
    }


/*-----------------------------------------------------------------*//**
* @param [in] transform affine transformation 3x4 matrix
* @param [in] invert true to treat this matrix as the inverse
*                                        of the mapping, false if forward.
* @return true if the Transfrom was invertible.
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DMap4d::InitFromTransform
(

TransformCR transform,
      bool invert

)
    {
    return bsiDMap4d_initFromTransform (this, (&transform),(invert ? true : false)) ? true : false;
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

* @param point0001 0001 in the unit space maps here
* @param point1001 1001 in the unit space maps here
* @param point0101 0100 in the unit space maps here
* @param point0010 0010 in the unit space maps here
* @return true if the 4 points are independent
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DMap4d::InitFromSkewBox
(

DPoint4dCR point0001,
DPoint4dCR point1001,
DPoint4dCR point0101,
DPoint4dCR point0010

)
    {
    return bsiDMap4d_initHomgeneousSkewFrame (this, (&point0001),(&point1001),(&point0101),(&point0010)) ? true : false;
    }


/*-----------------------------------------------------------------*//**
*
* @param [in] z0 reference z.  Normalized projective coordinate
                                is 0 at this z
* @param [in] zetaHalf controls rate of growth of normalized
                                projective z.   Projective z at z0/k
                                is k-1*zetahalf, i.e. is zetahalf at
                                z0/2
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMap4d::ZFrustum
(

double z0,
double zetaHalf

)
    {
    bsiDMap4d_zFrustum (this, z0,zetaHalf);
    }


/*-----------------------------------------------------------------*//**
* Sets pA to the inverse of B.

* @param [in] B original mapping
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMap4d::InverseOf
(

DMap4dCR B

)
    {
    bsiDMap4d_invert (this, (&B));
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DMap4d::SandwichOf
(
DMap4dCR A,
DMap4dCR B,
DMap4dCR C
)
    {
    bsiDMap4d_sandwich (this, (&A),(&B),(&C));
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DMap4d::SandwichOfBABinverse
(
DMap4dCR A,
DMap4dCR B
)
    {
    DMap4d Binverse, product;
    bsiDMap4d_invert ( &Binverse, &B);
    bsiDMap4d_multiply (&product, &B, &A );
    bsiDMap4d_multiply (this, &product, &Binverse);
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DMap4d::SandwichOfBinverseAB
(
DMap4dCR A,
DMap4dCR B
)
    {
    DMap4d Binverse, product;
    bsiDMap4d_invert ( &Binverse, &B);
    bsiDMap4d_multiply (&product, &A, &B );
    bsiDMap4d_multiply (this, &Binverse, &product);
    }





/*-----------------------------------------------------------------*//**
* Multiply transforms
*
* @param [in] A transform A
* @param [in] B transform B
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMap4d::InitProduct
(

DMap4dCR A,
DMap4dCR B

)
    {
    bsiDMap4d_multiply (this, (&A),(&B));
    }


/*-----------------------------------------------------------------*//**
* Multiply transforms, selecting optional forward or inverse for each

* @param [in] A transform A
* @param [in] invertA if true, use invese of A
* @param [in] B transform B
* @param [in] invertB if true, use invese of B
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMap4d::InitProduct
(

DMap4dCR A,
bool invertA,
DMap4dCR B,
bool invertB

)
    {
    bsiDMap4d_multiplyInverted (this, (&A),(invertA ? true : false),(&B),(invertB ? true : false));
    }


/*-----------------------------------------------------------------*//**
* Checks if the mapping is strictly a scale/translate in specified
* directions.

* @param [in] xChange 1 if x may change, 0 if it must stay fixed
* @param [in] yChange 1 if y may change, 0 if it must stay fixed
* @param [in] zChange 1 if z may change, 0 if it must stay fixed
* @return true if independence tests are satisfied.
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DMap4d::IsIndependent
(

int     xChange,
int     yChange,
int     zChange

)
    {
    return bsiDMap4d_isIndependent (this, xChange,yChange,zChange) ? true : false;
    }


/*-----------------------------------------------------------------*//**
* Test if a transform is singular

* @return true if the mapping is singular
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DMap4d::IsSingular
(

) const
    {
    return bsiDMap4d_isSingular (this) ? true : false;
    }


/*-----------------------------------------------------------------*//**
* Test if a transform is affiine.

* @return true if the mapping is affine.
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DMap4d::IsAffine
(

) const
    {
    return bsiDMap4d_isAffine (this) ? true : false;
    }


/*-----------------------------------------------------------------*//**
* Test if a transform is perspective.

* @return true if the mapping contains perspective.
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DMap4d::IsPerspective
(
) const
    {
    return bsiDMap4d_isPerspective (this) ? true : false;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void extractParts
(
DMatrix4dCR source,
RotMatrixR matrix,
DPoint3dR  translation,
DPoint4dR  perspective
)
    {
    for (int i = 0; i < 3; i++)
       for (int j = 0; j < 3; j++)
            matrix.form3d[i][j] = source.coff[i][j];

    translation.x = source.coff[0][3];
    translation.y = source.coff[1][3];
    translation.z = source.coff[2][3];

    perspective.x = source.coff[3][0];
    perspective.y = source.coff[3][1];
    perspective.z = source.coff[3][2];
    perspective.w = source.coff[3][3];
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DMap4d::Explode
(
RotMatrixR matrix,
DPoint3dR  translation,
DPoint4dR  perspective,
bool       inverse
) const
    {
    if (inverse)
        extractParts (M1, matrix, translation, perspective);
    else
        extractParts (M0, matrix, translation, perspective);
    }



/*-----------------------------------------------------------------*//**
*
* @param => mapping to test
* @return true if the mapping is an identity
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DMap4d::IsIdentity
(

) const
    {
    return bsiDMap4d_isIdentity (this) ? true : false;
    }


END_BENTLEY_GEOMETRY_NAMESPACE
