/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#define CompileMultiplyTranspose
BEGIN_BENTLEY_NAMESPACE


// Make dest the same size as source. (But without touching its contents.)
// Return true if this is a nozero size.
template <typename Tr, typename Ts>
static bool ResizeDestination (bvector<Tr> &dest, bvector<Ts> const &source)
    {
    size_t n = source.size();
    if (n == 0)
        {
        dest.empty ();
        return false;
        }
   else 
        {
        dest.resize (n);
        }
    return true;
    }

/*-----------------------------------------------------------------*//**
* @description Return (just) the X component of transform*point, 
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double Transform::MultiplyX (DPoint3dCR point) const
    {
    return        form3d[0][0] * point.x
                + form3d[0][1] * point.y
                + form3d[0][2] * point.z
                + form3d[0][3];
    }

/*-----------------------------------------------------------------*//**
* @description Return (just) the Y component of transform*point, 
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double Transform::MultiplyY (DPoint3dCR point) const
    {
    return        form3d[1][0] * point.x
                + form3d[1][1] * point.y
                + form3d[1][2] * point.z
                + form3d[1][3];
    }

/*-----------------------------------------------------------------*//**
* @description Return (just) the Z component of transform*point, 
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double Transform::MultiplyZ (DPoint3dCR point) const
    {
    return        form3d[2][0] * point.x
                + form3d[2][1] * point.y
                + form3d[2][2] * point.z
                + form3d[2][3];
    }

/*-----------------------------------------------------------------*//**
* @description Multiplies a point by a transform, returning the result in
*   place of the input point.
* @param [in,out] point point to be updated
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::Multiply (DPoint3dR point) const
    {
    if (point.IsDisconnect ())
        return; // leave it alone !!!
    DPoint3d  inPoint = point;
    point.x =    form3d[0][0] * inPoint.x
                + form3d[0][1] * inPoint.y
                + form3d[0][2] * inPoint.z
                + form3d[0][3];

    point.y =    form3d[1][0] * inPoint.x
                + form3d[1][1] * inPoint.y
                + form3d[1][2] * inPoint.z
                + form3d[1][3];

    point.z =    form3d[2][0] * inPoint.x
                + form3d[2][1] * inPoint.y
                + form3d[2][2] * inPoint.z
                + form3d[2][3];
    }


/*-----------------------------------------------------------------*//**
* @description Returns the product of a transform times a point.
* @param [out] result returned point.
* @param [in] point The input point.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::Multiply (DPoint3dR result, DPoint3dCR point) const
    {
    if (point.IsDisconnect ())
        {
        result = point;
        return;
        }
    DPoint3d  inPoint;

    inPoint = point;

    result.x =    form3d[0][0] * inPoint.x
                + form3d[0][1] * inPoint.y
                + form3d[0][2] * inPoint.z
                + form3d[0][3];

    result.y =    form3d[1][0] * inPoint.x
                + form3d[1][1] * inPoint.y
                + form3d[1][2] * inPoint.z
                + form3d[1][3];

    result.z =    form3d[2][0] * inPoint.x
                + form3d[2][1] * inPoint.y
                + form3d[2][2] * inPoint.z
                + form3d[2][3];
    }



/*-----------------------------------------------------------------*//**
* @description Returns the product of a transform times a point.
* @param [out] result returned point.
* @param [in] point The input point.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::Multiply
(
DPoint2dR result,
DPoint2dCR point
) const
    {
    if (point.IsDisconnect ())
        {
        result = point;
        return;
        }
    
    DPoint3d  inPoint;
    inPoint.x = point.x;
    inPoint.y = point.y;
    result.x =    form3d[0][0] * inPoint.x
                + form3d[0][1] * inPoint.y
                + form3d[0][3];

    result.y =    form3d[1][0] * inPoint.x
                + form3d[1][1] * inPoint.y
                + form3d[1][3];
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void Transform::Multiply
(
DPoint4dR result,
DPoint4dCR point
) const
    {
    double x = point.x;
    double y = point.y;
    double z = point.z;
    double w = point.w;
    if (w == 1.0)
        {
        result.x =   form3d[0][3]
                + form3d[0][0] * x
                + form3d[0][1] * y
                + form3d[0][2] * z;

        result.y =   form3d[1][3]
                + form3d[1][0] * x
                + form3d[1][1] * y
                + form3d[1][2] * z;

        result.z =   form3d[2][3]
                + form3d[2][0] * x
                + form3d[2][1] * y
                + form3d[2][2] * z;
        result.w = 1.0;
        }
    else
        {
        result.x =   form3d[0][3] * w
                + form3d[0][0] * x
                + form3d[0][1] * y
                + form3d[0][2] * z;

        result.y =   form3d[1][3] * w
                + form3d[1][0] * x
                + form3d[1][1] * y
                + form3d[1][2] * z;

        result.z =   form3d[2][3] * w
                + form3d[2][0] * x
                + form3d[2][1] * y
                + form3d[2][2] * z;
        result.w = w;
        }
    }

/*-----------------------------------------------------------------*//**
* @description Returns the product of a transform times a point.
* @param [out] result returned point.
* @param [in] point The input point.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::Multiply
(
DPoint2dR result,
DPoint3dCR point
) const
    {
    if (point.IsDisconnect ())
        {
        result.InitDisconnect ();
        return;
        }
    DPoint3d  inPoint;

    inPoint = point;

    result.x =    form3d[0][0] * inPoint.x
                + form3d[0][1] * inPoint.y
                + form3d[0][2] * inPoint.z
                + form3d[0][3];

    result.y =    form3d[1][0] * inPoint.x
                + form3d[1][1] * inPoint.y
                + form3d[1][2] * inPoint.z
                + form3d[1][3];
    }



/*-----------------------------------------------------------------*//**
* @description Returns the product of a transform times a point.
* @param [out] result returned point.
* @param [in] point The input point.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::Multiply
(
DPoint3dR result,
DPoint2dCR point
) const
    {
    if (point.IsDisconnect ())
        {
        result.InitDisconnect ();
        return;
        }
    DPoint2d  inPoint = point;

    result.x =    form3d[0][0] * inPoint.x
                + form3d[0][1] * inPoint.y
                + form3d[0][3];

    result.y =    form3d[1][0] * inPoint.x
                + form3d[1][1] * inPoint.y
                + form3d[1][3];

    result.z =    form3d[2][0] * inPoint.x
                + form3d[2][1] * inPoint.y
                + form3d[2][3];
    }




/*-----------------------------------------------------------------*//**
* Multiplies a "weighted point" in place.  That is, the point is input and output as
*   (wx,wy,wz,w) where x,y,z are the cartesian image coordinates.
*
* @param [in,out] point point to be updated
* @param [in] weight The weight
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::MultiplyWeighted
(
DPoint3dR point,
double    weight
) const
    {
    DPoint3d  inPoint;

    inPoint = point;

    point.x =     form3d[0][0] * inPoint.x
                + form3d[0][1] * inPoint.y
                + form3d[0][2] * inPoint.z
                + form3d[0][3] * weight;

    point.y =     form3d[1][0] * inPoint.x
                + form3d[1][1] * inPoint.y
                + form3d[1][2] * inPoint.z
                + form3d[1][3] * weight;

    point.z =     form3d[2][0] * inPoint.x
                + form3d[2][1] * inPoint.y
                + form3d[2][2] * inPoint.z
                + form3d[2][3] * weight;
    }


/*-----------------------------------------------------------------*//**
* Multiplies an array of "weighted points" in place.  That is, the point is input and output as
*   (wx,wy,wz,w) where x,y,z are the cartesian image coordinates.
*
* @param [in] outPoint The transformed points.
* @param [in] inPoint The original points
* @param [in] pWeight The weight array.  If null, unit weight is used.
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::MultiplyWeighted
(

DPoint3dP       outPoint,
DPoint3dCP      inPointIN,
const double   *pWeightIN,
int             numPoint

) const
    {
    DPoint3d  inPoint;
    double    weight;

    int i;

    for (i = 0; i < numPoint; i++)
        {
        inPoint = inPointIN[i];
        weight  = pWeightIN[i];

        outPoint[i].x = form3d[0][0] * inPoint.x
                      + form3d[0][1] * inPoint.y
                      + form3d[0][2] * inPoint.z
                      + form3d[0][3] * weight;

        outPoint[i].y = form3d[1][0] * inPoint.x
                      + form3d[1][1] * inPoint.y
                      + form3d[1][2] * inPoint.z
                      + form3d[1][3] * weight;

        outPoint[i].z = form3d[2][0] * inPoint.x
                      + form3d[2][1] * inPoint.y
                      + form3d[2][2] * inPoint.z
                      + form3d[2][3] * weight;
        }

    }

void Transform::MultiplyWeighted (bvector<DPoint3d>&out, bvector<DPoint3d>const &in, bvector <double> const *weights) const
    {
    if (NULL == weights)
        {
        Multiply (out, in);
        }
    if (ResizeDestination (out, in))
        MultiplyWeighted (&out[0], &in[0], &weights->at(0), (int)in.size ());
    return;
    }


#ifdef CompileMultiplyTranspose
/*-----------------------------------------------------------------*//**
* Multiplies this instance times the column vector point and replaces point
* with the result, using the transpose of the matrix part of this instance in
* the multiplication.
* Symbolically, this is equivalent to being given transform [R t] and row
* vector p, and returning the point p*R + t.
*
* @param [in,out] point point to be updated
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::MultiplyTranspose
(

DPoint3dR point

) const
    {
    if (point.IsDisconnect ())
        return;
    
    DPoint3d  inPoint;

    inPoint = point;

    point.x =     form3d[0][0] * inPoint.x
                + form3d[1][0] * inPoint.y
                + form3d[2][0] * inPoint.z
                + form3d[0][3];

    point.y =     form3d[0][1] * inPoint.x
                + form3d[1][1] * inPoint.y
                + form3d[2][1] * inPoint.z
                + form3d[1][3];

    point.z =     form3d[0][2] * inPoint.x
                + form3d[1][2] * inPoint.y
                + form3d[2][2] * inPoint.z
                + form3d[2][3];
    }
#endif

/*-----------------------------------------------------------------*//**
* @description Returns the product of a matrix times a point, with the point
*       specified as explict x, y, and z values.
* @param [out] point result of transformation * point operation
* @param [in] x The x component of the point
* @param [in] y The y component of the point
* @param [in] z The z component of the point
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::Multiply
(

DPoint3dR point,
double        x,
double        y,
double        z

) const
    {
    point.x =     form3d[0][3]
                + form3d[0][0] * x
                + form3d[0][1] * y
                + form3d[0][2] * z;

    point.y =     form3d[1][3]
                + form3d[1][0] * x
                + form3d[1][1] * y
                + form3d[1][2] * z;

    point.z =     form3d[2][3]
                + form3d[2][0] * x
                + form3d[2][1] * y
                + form3d[2][2] * z;
    }

#ifdef CompileMultiplyTranspose
/*-----------------------------------------------------------------*//**
* Multiplies this instance times the column vector constructed from components
* x,y,z, using the transpose of the matrix part of this instance in the
* multiplication.
* Symbolically, this is equivalent to being given transform [R t] and row
* vector p, and returning the point p*R + t.

* @param [out] point result of transformation * point operation
* @param [in] x The x component of the point
* @param [in] y The y component of the point
* @param [in] z The z component of the point
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::MultiplyTranspose
(

DPoint3dR point,
double        x,
double        y,
double        z

) const
    {
    point.x =     form3d[0][0] * x
                + form3d[1][0] * y
                + form3d[2][0] * z
                + form3d[0][3];

    point.y =     form3d[0][1] * x
                + form3d[1][1] * y
                + form3d[2][1] * z
                + form3d[1][3];

    point.z =     form3d[0][2] * x
                + form3d[1][2] * y
                + form3d[2][2] * z
                + form3d[2][3];
    }
#endif


/*-----------------------------------------------------------------*//**
* Multiplies the matrix part of this instance times the column vector
* constructed from components x,y,z.
* Symbolically, given transform [R t] and column vector p,
* the returned point is R*p.
*
* @param [out] point result of matrix * point operation
* @param [in] x The x component of the point
* @param [in] y The y component of the point
* @param [in] z The z component of the point
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::MultiplyMatrixOnly
(
DPoint3dR point,
double        x,
double        y,
double        z

) const
    {
    point.x =     form3d[0][0] * x
                + form3d[0][1] * y
                + form3d[0][2] * z;

    point.y =     form3d[1][0] * x
                + form3d[1][1] * y
                + form3d[1][2] * z;

    point.z =     form3d[2][0] * x
                + form3d[2][1] * y
                + form3d[2][2] * z;
    }


/*-----------------------------------------------------------------*//**
* Multiplies the matrix part of this instance times column vector inPoint.
* Symbolically, given transform [R t] and column vector p,
* the returned point is R*p.
*
* @param [out] outPoint result of matrix * point operation
* @param [in] inPoint The point by which matrix is multiplied
* @indexVerb
* @bsimethod                                                    DavidAssaf      7/98
+----------------------------------------------------------------------*/
void Transform::MultiplyMatrixOnly
(

DPoint3dR outPoint,
DPoint3dCR point

) const
    {
    if (point.IsDisconnect ())
        {
        outPoint.InitDisconnect ();
        return;
        }
    
    DPoint3d inPoint;
    inPoint = point;
    outPoint.x =    form3d[0][0] * inPoint.x
                  + form3d[0][1] * inPoint.y
                  + form3d[0][2] * inPoint.z;

    outPoint.y =    form3d[1][0] * inPoint.x
                  + form3d[1][1] * inPoint.y
                  + form3d[1][2] * inPoint.z;

    outPoint.z =    form3d[2][0] * inPoint.x
                  + form3d[2][1] * inPoint.y
                  + form3d[2][2] * inPoint.z;
    }


/*-----------------------------------------------------------------*//**
* Multiplies the matrix part of this instance times column vector point and
* replaces point with the result.
* Symbolically, given transform [R t] and column vector p,
* the returned point is R*p.
*
* @param [in,out] point point to be updated
* @indexVerb
* @bsimethod                                                    DavidAssaf      7/98
+----------------------------------------------------------------------*/
void Transform::MultiplyMatrixOnly
(

DPoint3dR point

) const
    {
    if (point.IsDisconnect ())
        return;
    DPoint3d  inPoint;

    inPoint = point;

    point.x =    form3d[0][0] * inPoint.x
               + form3d[0][1] * inPoint.y
               + form3d[0][2] * inPoint.z;

    point.y =    form3d[1][0] * inPoint.x
               + form3d[1][1] * inPoint.y
               + form3d[1][2] * inPoint.z;

    point.z =    form3d[2][0] * inPoint.x
               + form3d[2][1] * inPoint.y
               + form3d[2][2] * inPoint.z;
    }


/*-----------------------------------------------------------------*//**
* Multiplies the row vector constructed from components x,y,z times the matrix
* part of this instance.
* Symbolically, given transform [R t] and row vector p,
* the returned point is p*R.
*
* @param [out] point result of point * matrix operation
* @param [in] x The x component of the point
* @param [in] y The y component of the point
* @param [in] z The z component of the point
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::MultiplyTransposeMatrixOnly
(
DPoint3dR point,
double        x,
double        y,
double        z
) const
    {
    point.x =     form3d[0][0] * x
                + form3d[1][0] * y
                + form3d[2][0] * z;

    point.y =     form3d[0][1] * x
                + form3d[1][1] * y
                + form3d[2][1] * z;

    point.z =     form3d[0][2] * x
                + form3d[1][2] * y
                + form3d[2][2] * z;
    }


/*-----------------------------------------------------------------*//**
* Multiplies the row vector inPoint times the matrix
* part of this instance.
* Symbolically, given transform [R t] and row vector p,
* the returned point is p*R.
*
* @param [out] outPoint result of point * matrix operation
* @param [in] inPoint The point which multiplies matrix
* @indexVerb
* @bsimethod                                                    DavidAssaf      7/98
+----------------------------------------------------------------------*/
void Transform::MultiplyTransposeMatrixOnly
(
DPoint3dR outPoint,
DPoint3dCR point
) const
    {
    if (point.IsDisconnect ())
        outPoint.InitDisconnect ();    
    DPoint3d  inPoint;
    inPoint = point;
    outPoint.x =    form3d[0][0] * inPoint.x
                  + form3d[1][0] * inPoint.y
                  + form3d[2][0] * inPoint.z;

    outPoint.y =    form3d[0][1] * inPoint.x
                  + form3d[1][1] * inPoint.y
                  + form3d[2][1] * inPoint.z;

    outPoint.z =    form3d[0][2] * inPoint.x
                  + form3d[1][2] * inPoint.y
                  + form3d[2][2] * inPoint.z;
    }


/*-----------------------------------------------------------------*//**
* Multiplies the row vector point times the matrix
* part of this instance, and replaces point with the result.
* Symbolically, given transform [R t] and row vector p,
* the returned point is p*R.
*
* @param [in,out] point point to be updated
* @indexVerb
* @bsimethod                                                    DavidAssaf      7/98
+----------------------------------------------------------------------*/
void Transform::MultiplyTransposeMatrixOnly
(
DPoint3dR point
) const
    {
    if (point.IsDisconnect ())
        return;
    DPoint3d  inPoint;

    inPoint = point;

    point.x =    form3d[0][0] * inPoint.x
               + form3d[1][0] * inPoint.y
               + form3d[2][0] * inPoint.z;

    point.y =    form3d[0][1] * inPoint.x
               + form3d[1][1] * inPoint.y
               + form3d[2][1] * inPoint.z;

    point.z =    form3d[0][2] * inPoint.x
               + form3d[1][2] * inPoint.y
               + form3d[2][2] * inPoint.z;
    }


/*-----------------------------------------------------------------*//**
* Multiplies this instance times each column vector in pointArray
* and replaces pointArray with the resulting points.
* Symbolically, given transform [R t],
* each returned point is of the form R*p + t, where p is a column vector.
*
* @param [in,out] pointArray array of points to be multiplied
* @param [in] numPoint The number of points
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::Multiply
(
DPoint3dP pointArray,
int       numPoint
) const
    {
    for (int i = 0; i < numPoint; i++)
        Multiply (pointArray[i]);
    }

#ifdef CompileMultiplyTranspose
/*-----------------------------------------------------------------*//**
* Multiplies this instance times each column vector in pointArray,
* using the transpose of the matrix part of this instance in the multiplications,
* and replaces pointArray with the resulting points.
* Symbolically, given transform [R t], each returned point has the equivalent
* form p*R + t, where p is a row vector.
*
* @param [in,out] pointArray array of points to be multiplied
* @param [in] numPoint The number of points
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::MultiplyTranspose
(

DPoint3dP pointArray,
int       numPoint

) const
    {
    for (int i = 0; i < numPoint; i++)
        MultiplyTranspose (pointArray[i]);
    }
#endif

/*-----------------------------------------------------------------*//**
* Multiplies this instance times each column vector in inPoint and places the
* resulting points in outPoint.
* inPoint and outPoint may be the same.
* Symbolically, given transform [R t], each returned point has the
* form R*p + t*w (with weight w), where p is a column vector and w is its
* weight.
*
* @param [out] outPoint transformed points
* @param [in] inPoint The input points
* @param [in] numPoint The number of points
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::Multiply (DPoint4dP outPoint, DPoint4dCP inPoint, int numPoint) const
    {
    for (int i = 0; i < numPoint; i++)
        {
        Multiply (outPoint[i], inPoint[i]);
        }
    }

void Transform::Multiply (bvector<DPoint4d> &out, bvector<DPoint4d> const &in) const
    {
    if (ResizeDestination (out, in))
        for (size_t i = 0, n = in.size (); i < n; i++)
            Multiply (out[i], in[i]);
    }

/*-----------------------------------------------------------------*//**
* Multiplies this instance times each column vector in inPoint and places the
* resulting points in outPoint.
* inPoint and outPoint may be the same.
* Symbolically, given transform [R t], each returned point has the
* form R*p + t, where p is a column vector.
*
* @param [out] outPoint transformed points
* @param [in] inPoint The input points
* @param [in] numPoint The number of points
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::Multiply (DPoint3dP outPoint, DPoint3dCP inPoint,  int numPoint) const
    {
    for (int i = 0; i < numPoint; i++)
        {
        Multiply (outPoint[i], inPoint[i]);
        }
    }

void Transform::Multiply (bvector<DPoint3d> &out, bvector<DPoint3d> const &in) const
    {
    if (ResizeDestination (out, in))
        for (size_t i = 0, n = in.size (); i < n; i++)
            Multiply (out[i], in[i]);
    }

void Transform::Multiply (bvector<DPoint3d> &inout) const
    {
    for (auto &xyz: inout)
        Multiply (xyz);
    }

#ifdef CompileMultiplyTranspose
/*-----------------------------------------------------------------*//**
* Multiplies this instance times each column vector in inPoint,
* using the transpose of the matrix part of this instance in the multiplications,
* and places the resulting points in outPoint.
* Symbolically, given transform [R t], each returned point has the equivalent
* form p*R + t, where p is a row vector.
* inPoint and outPoint may be the same.
*
* @param [out] outPoint transformed points
* @param [in] inPoint The input points
* @param [in] numPoint The number of points
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::MultiplyTranspose
(

DPoint3dP outPoint,
DPoint3dCP inPoint,
int            numPoint

) const
    {
    for (int i = 0; i < numPoint; i++)
        {
        MultiplyTranspose (outPoint[i], inPoint[i].x, inPoint[i].y, inPoint[i].z);
        }
    }
#endif

/*-----------------------------------------------------------------*//**
* Multiplies this instance times each column vector in inPoint and places the
* resulting points in outPoint.
* inPoint and outPoint may be the same.
* All z parts (i.e. last row and column) of this instance are ignored.
*
* @param [out] outPoint transformed points
* @param [in] inPoint The input points
* @param [in] numPoint The number of points
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::Multiply (DPoint2dP outPoint, DPoint2dCP inPoint, int numPoint) const
    {
    for (int i = 0; i < numPoint; i++)
        Multiply (outPoint[i], inPoint[i]);
    }

void Transform::Multiply (bvector<DPoint2d> & out, bvector<DPoint2d> const &in) const
    {
    if (ResizeDestination (out, in))
        for (size_t i = 0, n = in.size (); i < n; i++)
            Multiply (out[i], in[i]);
    }
    
void Transform::Multiply (bvector<DPoint3d> & out, bvector<DPoint2d> const &in) const
    {
    if (ResizeDestination (out, in))
        for (size_t i = 0, n = in.size (); i < n; i++)
            Multiply (out[i], in[i]);
    }    
void Transform::Multiply (bvector<DPoint2d> & out, bvector<DPoint3d> const &in) const
    {
    if (ResizeDestination (out, in))
        for (size_t i = 0, n = in.size (); i < n; i++)
            Multiply (out[i], in[i]);
    }    
/*-----------------------------------------------------------------*//**
* Multiplies this instance times each column vector in inPoint and places the
* resulting points in outPoint.
* Each input point is given a z=0.
*
* @param [in] pTransform The transformation to apply
* @param [out] outPoint transformed points
* @param [in] inPoint The input points
* @param [in] numPoint The number of points
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::Multiply
(

DPoint3dP outPoint,
DPoint2dCP inPoint,
      int            numPoint

) const
    {
    for (int i = 0; i < numPoint; i++)
        {
        Multiply (outPoint[i], inPoint[i].x, inPoint[i].y, 0.0);
        }
    }


/*-----------------------------------------------------------------*//**
* Multiplies this instance times each column vector in inPoint and places the
* x and y components of the resulting points in outPoint.
*
* @param [out] outPoint transformed points
* @param [in] inPoint The input points
* @param [in] numPoint The number of points
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::Multiply
(

DPoint2dP outPoint,
DPoint3dCP inPoint,
      int            numPoint

) const
    {
    for (int i = 0; i < numPoint; i++)
        {
        Multiply (outPoint[i], inPoint[i]);
        }
    }


/*-----------------------------------------------------------------*//**
* @description Returns the product of two transformations.
* Symbolically, given transforms [R t] and [S u], return the product transform
* [R t][S u] = [R*S t+R*u].
*
* @param [in] transform1 The first factor
* @param [in] transform2 The second factor
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::InitProduct
(
TransformCR transformA,
TransformCR transformB
)
    {
    Transform AB;
    int j;
    AB.form3d[0][3] = transformA.form3d[0][3]
                     + transformA.form3d[0][0] * transformB.form3d[0][3]
                     + transformA.form3d[0][1] * transformB.form3d[1][3]
                     + transformA.form3d[0][2] * transformB.form3d[2][3];

    AB.form3d[1][3] = transformA.form3d[1][3]
                     + transformA.form3d[1][0] * transformB.form3d[0][3]
                     + transformA.form3d[1][1] * transformB.form3d[1][3]
                     + transformA.form3d[1][2] * transformB.form3d[2][3];

    AB.form3d[2][3] = transformA.form3d[2][3]
                     + transformA.form3d[2][0] * transformB.form3d[0][3]
                     + transformA.form3d[2][1] * transformB.form3d[1][3]
                     + transformA.form3d[2][2] * transformB.form3d[2][3];

    for (j = 0; j < 3; j++)
        {

        AB.form3d[0][j]
                       = transformA.form3d[0][0] * transformB.form3d[0][j]
                       + transformA.form3d[0][1] * transformB.form3d[1][j]
                       + transformA.form3d[0][2] * transformB.form3d[2][j];

        AB.form3d[1][j]
                       = transformA.form3d[1][0] * transformB.form3d[0][j]
                       + transformA.form3d[1][1] * transformB.form3d[1][j]
                       + transformA.form3d[1][2] * transformB.form3d[2][j];

        AB.form3d[2][j]
                       = transformA.form3d[2][0] * transformB.form3d[0][j]
                       + transformA.form3d[2][1] * transformB.form3d[1][j]
                       + transformA.form3d[2][2] * transformB.form3d[2][j];
        }
    *this = AB;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the product of a matrix and a transformation.
* The matrix acts like a transformation with a zero point as its translation part.

* @param [in] matrix The first factor (matrix)
* @param [in] transform The second factor (transform)
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::InitProduct
(

RotMatrixCR matrix,
TransformCR transform

)
    {
    Transform AB;
    int j;
    AB.form3d[0][3] = matrix.form3d[0][0] * transform.form3d[0][3]
                     + matrix.form3d[0][1] * transform.form3d[1][3]
                     + matrix.form3d[0][2] * transform.form3d[2][3];

    AB.form3d[1][3] = matrix.form3d[1][0] * transform.form3d[0][3]
                     + matrix.form3d[1][1] * transform.form3d[1][3]
                     + matrix.form3d[1][2] * transform.form3d[2][3];

    AB.form3d[2][3] = matrix.form3d[2][0] * transform.form3d[0][3]
                     + matrix.form3d[2][1] * transform.form3d[1][3]
                     + matrix.form3d[2][2] * transform.form3d[2][3];

    for (j = 0; j < 3; j++)
        {

        AB.form3d[0][j]
                       = matrix.form3d[0][0] * transform.form3d[0][j]
                       + matrix.form3d[0][1] * transform.form3d[1][j]
                       + matrix.form3d[0][2] * transform.form3d[2][j];

        AB.form3d[1][j]
                       = matrix.form3d[1][0] * transform.form3d[0][j]
                       + matrix.form3d[1][1] * transform.form3d[1][j]
                       + matrix.form3d[1][2] * transform.form3d[2][j];

        AB.form3d[2][j]
                       = matrix.form3d[2][0] * transform.form3d[0][j]
                       + matrix.form3d[2][1] * transform.form3d[1][j]
                       + matrix.form3d[2][2] * transform.form3d[2][j];
        }
    *this = AB;
    }


/*-----------------------------------------------------------------*//**
* @description returns the product of a transformation times a matrix, treating
*   the matrix as a transformation with zero as its translation components.
* @param [in] transform The first facpatentor (transform)
* @param [in] matrix The second factor (matrix)
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::InitProduct
(

TransformCR transform,
RotMatrixCR matrix

)
    {
    Transform AB;

    int j;
    AB.form3d[0][3] = transform.form3d[0][3];
    AB.form3d[1][3] = transform.form3d[1][3];
    AB.form3d[2][3] = transform.form3d[2][3];

    for (j = 0; j < 3; j++)
        {

        AB.form3d[0][j]
                       = transform.form3d[0][0] * matrix.form3d[0][j]
                       + transform.form3d[0][1] * matrix.form3d[1][j]
                       + transform.form3d[0][2] * matrix.form3d[2][j];

        AB.form3d[1][j]
                       = transform.form3d[1][0] * matrix.form3d[0][j]
                       + transform.form3d[1][1] * matrix.form3d[1][j]
                       + transform.form3d[1][2] * matrix.form3d[2][j];

        AB.form3d[2][j]
                       = transform.form3d[2][0] * matrix.form3d[0][j]
                       + transform.form3d[2][1] * matrix.form3d[1][j]
                       + transform.form3d[2][2] * matrix.form3d[2][j];
        }
    *this = AB;

    }



//! Multiply by a translation "from the left": result = (Identity , scaled translationIn) * transformIn
//! @param [in] translationIn translation vector for left term
//! @param [in] scale factor for translation
//! @param [in] transformIn full transform for right factor.
void Transform::MultiplyTranslationTransform
(
DVec3dCR translationIn,
double s,
TransformCR transformIn
)
    {
    *this = transformIn;
    this->form3d[0][3] += s * translationIn.x;
    this->form3d[1][3] += s * translationIn.y;
    this->form3d[2][3] += s * translationIn.z;
    }



//! Multiply by a translation "from the left": result = transformIn * (Identity , scaled translationIn)
//! @param [in] transformIn full transform for right factor.
//! @param [in] translationIn translation vector for right term
//! @param [in] translation scale scale factor for translation


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void Transform::MultiplyTransformTranslation
(
TransformCR transformIn,
DVec3dCR translationIn,
double s
)
    {
    DVec3d QB;
    transformIn.MultiplyMatrixOnly (QB, translationIn);
    *this = transformIn;
    this->form3d[0][3] += s * QB.x;
    this->form3d[1][3] += s * QB.y;
    this->form3d[2][3] += s * QB.z;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      10/2012
+--------------------------------------------------------------------------------------*/
bool Transform::SolveArray (bvector<DPoint3d> &out, bvector<DPoint3d> const &in) const
    {
    if (ResizeDestination (out, in))
        return SolveArray (&out[0], &in[0], (int) in.size ());
    return false;
    }
    
bool RotMatrix::SolveArray (bvector<DPoint3d> &out, bvector<DPoint3d> const &in) const
    {
    if (ResizeDestination (out, in))
        return SolveArray (&out[0], &in[0], (int) in.size ());
    return false;    
    }

// Remark:  put RotMatrix array mutlipliers here to consolidate size handling.
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      10/2012
+--------------------------------------------------------------------------------------*/
void RotMatrix::Multiply (bvector<DPoint3d> &out, bvector<DPoint3d> const &in) const
    {
    if (ResizeDestination (out, in))
        for (size_t i = 0, n = in.size (); i < n; i++)
            Multiply (out[i], in[i]);
    }
    
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      10/2012
+--------------------------------------------------------------------------------------*/
void RotMatrix::Multiply (bvector<DPoint2d> &out, bvector<DPoint2d> const &in) const
    {
    if (ResizeDestination (out, in))
        Multiply (&out[0], &in[0], (int)in.size ());
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      10/2012
+--------------------------------------------------------------------------------------*/
void RotMatrix::Multiply (bvector<DPoint4d> &out, bvector<DPoint4d> const &in) const
    {
    if (ResizeDestination (out, in))
        Multiply (&out[0], &in[0], (int)in.size ());
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      10/2012
+--------------------------------------------------------------------------------------*/
void RotMatrix::MultiplyTranspose (bvector<DPoint3d> &out, bvector<DPoint3d> const &in) const
    {
    if (ResizeDestination (out, in))
        for (size_t i = 0, n = in.size (); i < n; i++)
            MultiplyTranspose (out[i], in[i]);
    }

void Transform::Multiply (DEllipse3dR ellipse) const
    {
    Multiply (ellipse.center);
    MultiplyMatrixOnly (ellipse.vector0);
    MultiplyMatrixOnly (ellipse.vector90);
    }
    
void Transform::Multiply (DEllipse3dR dest, DEllipse3dCR source) const
    {
    Multiply (dest.center, source.center);
    MultiplyMatrixOnly (dest.vector0, source.vector0);
    MultiplyMatrixOnly (dest.vector90, source.vector90);
    dest.start = source.start;
    dest.sweep = source.sweep;
    }


bool Transform::Multiply (DPlane3dR plane) const
    {
    RotMatrix matrix;
    GetMatrix (matrix);
    /* Origin transforms as a point */
    Multiply (plane.origin);
    /* Normal transforms as the inverse transpose of the matrix part. */
    /* BTW: If the matrix is orthogonal, this is a long way to multiply by the
         matrix part.  UGH. */
    return matrix.SolveTranspose (plane.normal, plane.normal);
    }

bool Transform::Multiply (DPlane3dR dest, DPlane3dCR source) const
    {
    dest = source;
    return Multiply (dest);
    }

void Transform::Multiply (DSegment3dR segment) const
    {
    Multiply (segment.point[0]);
    Multiply (segment.point[1]);
    }
    
void Transform::Multiply (DSegment3dR dest, DSegment3dCR source) const
    {
    Multiply (dest.point[0], source.point[0]);
    Multiply (dest.point[1], source.point[1]);
    }

void Transform::Multiply (DRay3dR ray) const
    {
    Multiply (ray.origin);
    MultiplyMatrixOnly (ray.direction);
    }
    
void Transform::Multiply (DRay3dR dest, DRay3dCR source) const
    {
    Multiply (dest.origin, source.origin);
    MultiplyMatrixOnly (dest.direction, source.direction);
    }


END_BENTLEY_NAMESPACE
