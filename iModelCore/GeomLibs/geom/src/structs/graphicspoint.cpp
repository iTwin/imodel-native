/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/graphicspoint.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

GraphicsPoint::GraphicsPoint ()
    {
    point.x = point.y = point.z = point.w = a = 0.0;
    userData = 0;
    mask = 0;
    }

GraphicsPoint::GraphicsPoint (DPoint4dCR _point, double _a, int _userData, int _mask, double _b, size_t _index)
    {
    point = _point;
    a = _a;
    userData = _userData;
    mask = _mask;
    b = _b;
    index = _index;
    }

GraphicsPoint::GraphicsPoint (DPoint3dCR _point, double _w, double _a, int _userData, int _mask, double _b, size_t _index)
    {
    point.x = _point.x;
    point.y = _point.y;
    point.z = _point.z;
    point.w = _w;
    a = _a;
    userData = _userData;
    mask = _mask;
    b = _b;
    index = _index;
    }

GraphicsPoint::GraphicsPoint (double _x, double _y, double _z, double _w, double _a, int _userData, int _mask, double _b, size_t _index)
    {
    point.x = _x;
    point.y = _y;
    point.z = _z;
    point.w = _w;
    a = _a;
    userData = _userData;
    mask = _mask;
    b = _b;
    index = _index;
    }

void       GraphicsPoint::SetPointPreserveWeight (DPoint4dCR inputPoint)
    {
    double f = point.w / inputPoint.w;
    point.x = f * inputPoint.x;
    point.y = f * inputPoint.y;
    point.z = f * inputPoint.z;
    }

bool GraphicsPoint::SetOrder (int order)
    {
    if (order < 0 || order > HPOINT_MAX_ORDER)
        return false;
    int apply = (uint32_t) order << HPOINT_MASK_ORDER_BITSHIFT;
    mask &= ~(HPOINT_MASK_ORDER);
    mask |= apply;
    return true;
    }

int GraphicsPoint::GetOrder() const
    {
    int value = mask & HPOINT_MASK_ORDER;
    return value >> HPOINT_MASK_ORDER_BITSHIFT;
    }

int GraphicsPoint::GetCurveType() const
    {
    return mask & HPOINT_MASK_CURVETYPE_BITS;
    }
int GraphicsPoint::GetPointType() const
    {
    return mask & HPOINT_MASK_POINTTYPE_BITS;
    }


bool GraphicsPoint::IsCurveBreak() const
    {
    return (mask & HPOINT_MASK_BREAK) != 0;
    }

void GraphicsPoint::SetCurveBreak (bool value)
    {
    if (value)
        mask |= HPOINT_MASK_BREAK;
    else
        mask &= ~HPOINT_MASK_BREAK;
    }

void GraphicsPoint::SetLoopBreak (bool value)
    {
    if (value)
        mask |= HPOINT_MASK_MAJOR_BREAK;
    else
        mask &= ~HPOINT_MASK_MAJOR_BREAK;
    }



bool GraphicsPoint::IsLoopBreak() const
    {
    return (mask & HPOINT_MASK_MAJOR_BREAK) != 0;
    }


bool GraphicsPoint::GetNormalized (DPoint3dR xyz) const
    {
    xyz.Init (point.x, point.y, point.z);
    // most commonly ....
    if (point.w == 1.0)
        return true;
    if (point.w == 0.0)
        return false;
    double dw   = 1.0 / point.w;
    xyz.x *= dw;
    xyz.y *= dw;
    xyz.z *= dw;
    return true;
    }

bool GraphicsPoint::CheckCurveAndPointType (int curveType, int pointType) const
    {
    int cType = mask & HPOINT_MASK_CURVETYPE_BITS;
    int pType = mask & HPOINT_MASK_POINTTYPE_BITS;
    return cType == curveType && pType == pointType;
    }

void GraphicsPoint::Zero ()
    {
    memset (this, 0, sizeof (*this));
    }
/*-----------------------------------------------------------------*//**
* @struct GraphicsPoint
* A GraphicsPoint is an 4d (homogeneous) point with an additional double
*   and 2 integers for labeling use during graphics operations.
* @fields
* @field DPoint4d point = homogeneous point
* @field double a = double label field.
* @field        mask = integer mask used by GraphicsPointArray.
* @field    userData =  integer label field.
* @endfields
* @bsistruct                                        EarlinLutz      09/00
+----------------------------------------------------------------------*/



/*---------------------------------------------------------------------------------**//**
* Initialize a graphics point from complete data
*
* @instance pInstance <= graphics point to initialize
* @param pPoint => point to add
* @param a      => floating point label value
* @param mask   => mask value
* @param userData => user data labal
* @indexVerb init
* @bsihdr                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiGraphicsPoint_initFromDPoint4d

(
GraphicsPointP pInstance,
DPoint4dCP pPoint,
double          a,
int             mask,
int             userData
)
    {
    pInstance->point = *pPoint;
    pInstance->mask = mask;
    pInstance->a = a;
    pInstance->userData = userData;
    }


/*---------------------------------------------------------------------------------**//**
* Initialize a graphics point from complete data
*
* @instance pInstance <= graphics point to initialize
* @param pPoint => point to add
* @param a      => floating point label value
* @param mask   => mask value
* @param userData => user data labal
* @indexVerb init
* @bsihdr                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiGraphicsPoint_initFromDPoint3d

(
GraphicsPointP pInstance,
DPoint3dCP pPoint,
double          weight,
double          a,
int             mask,
int             userData
)
    {
    pInstance->point.x = pPoint->x;
    pInstance->point.y = pPoint->y;
    pInstance->point.z = pPoint->z;
    pInstance->point.w = weight;

    pInstance->a = a;
    pInstance->mask = mask;
    pInstance->userData = userData;
    pInstance->b = 0.0;
    }


/*---------------------------------------------------------------------------------**//**
* Initialize a graphics point from complete data
*
* @instance pInstance <= graphics point to initialize
* @param pPoint => point to add
* @param a      => floating point label value
* @param mask   => mask value
* @param userData => user data labal
* @indexVerb init
* @bsihdr                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiGraphicsPoint_init

(
GraphicsPointP pInstance,
double          x,
double          y,
double          z,
double          w,
double          a,
int             mask,
int             userData
)
    {
    pInstance->point.x = x;
    pInstance->point.y = y;
    pInstance->point.z = z;
    pInstance->point.w = w;

    pInstance->mask = mask;
    pInstance->a = a;
    pInstance->userData = userData;
    pInstance->b = 0.0;
    }


/*---------------------------------------------------------------------------------**//**
* Normalize the point weight in place.
* @instance pInstance <= normalized graphics point
* @param pPoint => source point
* @return true if the weight is nonzero
* @indexVerb normalize
* @bsihdr                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiGraphicsPoint_normalizeWeight

(
GraphicsPointP pInstance
)
    {
    double dw;
    if (pInstance->point.w == 0.0)
        return false;

    dw = 1.0 / pInstance->point.w;
    pInstance->point.x *= dw;
    pInstance->point.y *= dw;
    pInstance->point.z *= dw;
    pInstance->point.w  = 1.0;
    return  true;
    }


/*-----------------------------------------------------------------*//**
* Copies an array of graphics point
*
* @param pOutPoint <= destination array
* @param m => maximum number of points to copy
* @param pInPoint => source array
* @param n => number of points
* @see
* @return number of copied
* @indexVerb copy
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiGraphicsPoint_copyArray


(
GraphicsPointP pOutPoint,
int           m,
GraphicsPointCP pInPoint,
int           n
)
    {
    if (n < 0)
        n = 0;
    if (n > m)
        n = m;

    memcpy (pOutPoint, pInPoint, n * sizeof (GraphicsPoint));

    return n;
    }


/*-----------------------------------------------------------------*//**
* copies n DPoint4d structures from the pSource array to the pDest
* using an index array to rearrange (not necessarily 1to1) the order.
* The indexing assigns pDest[i] = pSource[indexP[i]].

*
* @param pDest <= destination array
* @param pSource => source array
* @param pIndex => array of indices into source array
* @param nIndex => number of points
* @see
* @indexVerb copy
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiGraphicsPoint_copyIndexedArray


(
GraphicsPointP pDest,
GraphicsPointCP pSource,
int             *pIndex,
int             nIndex
)
    {
    int     i;
    int    *indP;

    for (i = 0, indP = pIndex; i < nIndex; i++, indP++)
        {
        pDest[i] = pSource[*indP];
        }
    }


/*-----------------------------------------------------------------*//**
* Sets the in and out bits of the mask.
* @param bIn  => in bit setting.  Any nonzero sets it.
* @param bOut => out bit setting. Any nonzero sets it.
* @param returns the exact bits set in the point.  Calling with NULL point
*               is a way to convert generic bool    classifitions to
*               exact bits for the mask.
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiGraphicsPoint_setInOut

(
GraphicsPointP pGP,
bool            bIn,
bool            bOut
)
    {
    int mask = 0;
    if (bIn)
        mask |= HPOINT_MASK_INOUT_BIT_IN;
    if (bOut)
        mask |= HPOINT_MASK_INOUT_BIT_OUT;
    if (pGP)
        {
        pGP->mask &= ~(HPOINT_MASK_INOUT_BIT_IN | HPOINT_MASK_INOUT_BIT_OUT);
        pGP->mask |= mask;
        }
    return mask;
    }


/*-----------------------------------------------------------------*//**
* Sets the in and out bits of the mask.
* @param bIn  => in bit setting.  Any nonzero sets it.
* @param bOut => out bit setting. Any nonzero sets it.
* @param returns the exact bits set in the point.  Calling with NULL point
*               is a way to convert generic bool    classifitions to
*               exact bits for the mask.
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiGraphicsPoint_setInOutArray

(
GraphicsPointP pGP,
int             numGP,
bool            bIn,
bool            bOut
)
    {
    int mask = 0;
    int mask0 = ~(HPOINT_MASK_INOUT_BIT_IN | HPOINT_MASK_INOUT_BIT_OUT);
    int i;
    if (bIn)
        mask |= HPOINT_MASK_INOUT_BIT_IN;
    if (bOut)
        mask |= HPOINT_MASK_INOUT_BIT_OUT;
    if (pGP)
        {
        for (i = 0; i < numGP; i++)
            {
            pGP[i].mask &= mask0;
            pGP[i].mask |= mask;
            }
        }
    return mask;
    }


/*-----------------------------------------------------------------*//**
* Gets the inout bits.
* @param bbIn  <= in part of bit setting.
* @param bbOut <= out part of bit setting.
* @param returns the bits from the the point.
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiGraphicsPoint_getInOut

(
GraphicsPointP  pGP,
int             *pbIn,
int             *pbOut
)
    {
    int inBit = pGP->mask & HPOINT_MASK_INOUT_BIT_IN;
    int outBit = pGP->mask & HPOINT_MASK_INOUT_BIT_OUT;
    if (pbIn)
        *pbIn = inBit;

    if (pbOut)
        *pbOut = outBit;

    return inBit | outBit;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
