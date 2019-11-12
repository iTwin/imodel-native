/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

#define FIX_MIN(value, min)          if (value < min) min = value
#define FIX_MAX(value, max)          if (value > max) max = value
#define FIX_MINMAX(value, min, max)  FIX_MIN(value, min); FIX_MAX(value, max);

#define MSVECTOR_
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*-----------------------------------------------------------------*//**
* From/InitFrom/Extend --- bvector<DPoint3d>, bvector<weights>
+----------------------------------------------------------------------*/
DRange3d DRange3d::From (bvector<DPoint3d> const &points, bvector<double> const * weights)
    {
    DRange3d range = NullRange ();
    range.Extend (points, weights);
    return range;
    }

void DRange3d::InitFrom (bvector<DPoint3d> const &points, bvector<double> const * weights)
    {
    Init ();
    Extend (points, weights);
    }

void DRange3d::Extend (bvector<DPoint3d> const &points, bvector<double> const * weights)
    {
    if (points.size () > 0)
        {
        if (NULL == weights || weights->size () == 0)
            Extend (points);
        else
            {
            size_t numPoints = points.size ();
            size_t numWeights = weights->size ();
            if (numPoints > numWeights)
                numPoints = numWeights;
            for (size_t i = 0; i < numPoints; i++)
                Extend (points[i], weights->at (i));
            }
        }
    }

/*-----------------------------------------------------------------*//**
* From/InitFrom/Extend --- Transform, bvector<DPoint3d>
+----------------------------------------------------------------------*/
DRange3d DRange3d::From (TransformCR transform, bvector<DPoint3d> const &points)
    {
     DRange3d range = NullRange ();
     range.Extend (transform, points);
     return range;
    }

DRange3d DRange3d::From (TransformCR transform, bvector<DPoint4d> const &points)
    {
     DRange3d range = NullRange ();
     range.Extend (transform, points);
     return range;
    }

void DRange3d::InitFrom (TransformCR transform, bvector<DPoint3d> const &points)
    {
    Init ();
    Extend (transform, points);
    }

void DRange3d::Extend(TransformCR transform, bvector<DPoint3d> const &points)
    {
    DPoint3d xyz;
    for (size_t i = 0, n = points.size (); i < n; i++)
        {
        if (!points[i].IsDisconnect ())
            transform.Multiply (xyz, points[i]);
        Extend (xyz);
        }
    }
    
void DRange3d::Extend(TransformCR transform, bvector<DPoint4d> const &points)
    {
    DPoint3d xyz;
    for (size_t i = 0, n = points.size (); i < n; i++)
        {
        if (!points[i].GetProjectedXYZ (xyz))
            {
            transform.Multiply (xyz);
            Extend (xyz);
            }
        }
    }

/*-----------------------------------------------------------------*//**
* From/InitFrom/Extend --- bvector<DPoint3d>
+----------------------------------------------------------------------*/
DRange3d DRange3d::From (bvector<DPoint3d> const &points)
    {
     DRange3d range = NullRange ();
     range.Extend (points);
     return range;
    }

DRange3d DRange3d::From (bvector<bvector<DPoint3d>> const &points)
    {
     DRange3d range = NullRange ();
     for (auto &points1 : points)
         range.Extend (points1);
     return range;
    }

DRange3d DRange3d::From (bvector<bvector<bvector<DPoint3d>>> const &points)
    {
     DRange3d range = NullRange ();
     for (auto &points1 : points)
         for (auto &points2 : points1)
             range.Extend (points2);
     return range;
    }


void DRange3d::InitFrom (bvector<DPoint3d> const &points)
    {
    Init ();
    Extend (points);
    }

void DRange3d::Extend(bvector<DPoint3d> const &points)
    {
    for (size_t i = 0, n = points.size (); i < n; i++)
        Extend (points[i]);
    }

void DRange3d::Extend (DPoint3dCP points, int n)
    {
    for (int i = 0; i < n; i++)
        Extend (points[i]);
    }

    
/*-----------------------------------------------------------------*//**
* From/InitFrom/Extend --- bvector<DPoint3d>
+----------------------------------------------------------------------*/
DRange3d DRange3d::From (bvector<DPoint4d> const &points)
    {
     DRange3d range = NullRange ();
     range.Extend (points);
     return range;
    }

void DRange3d::InitFrom (bvector<DPoint4d> const &points)
    {
    Init ();
    Extend (points);
    }

void DRange3d::Extend(bvector<DPoint4d> const &points)
    {
    for (size_t i = 0, n = points.size (); i < n; i++)
        Extend (points[i]);
    }

void DRange3d::Extend (DPoint4dCP points, int n)
    {
    for (int i = 0; i < n; i++)
        Extend (points[i]);
    }
/*-----------------------------------------------------------------*//**
* From/InitFrom/Extend --- bvector<DPoint2d>, z
+----------------------------------------------------------------------*/
DRange3d DRange3d::From (bvector<DPoint2d> const &points, double zValue)
    {
     DRange3d range = NullRange ();
     range.Extend (points, zValue);
     return range;
    }

/*-----------------------------------------------------------------*//**
* From/InitFrom/Extend --- bvector<DPoint2d>, z
+----------------------------------------------------------------------*/
DRange3d DRange3d::From (bvector< bvector<DPoint2d> >const &points, double zValue)
    {
     DRange3d range = NullRange ();
     for (size_t i = 0; i < points.size (); i++)
         range.Extend (points[i], zValue);
     return range;
    }


void DRange3d::InitFrom (bvector<DPoint2d> const &points, double zValue)
    {
    Init ();
    Extend (points, zValue);
    }

void DRange3d::Extend (bvector<DPoint2d> const &points, double zValue)
    {
     for (size_t i = 0, n = points.size (); i < n; i++)
        Extend (points[i].x, points[i].y, zValue);
    }


/*-----------------------------------------------------------------*//**
*
* Initializes the range to contain the range of the given array of 2D points,
* with a single given z value for both the min and max points.
* If there are no points in the array, the range is initialized by
* DRange3d#init()
*
* @param [in] point array of points to search
* @param [in] n number of points in array
* @param [in] zVal default z value
+----------------------------------------------------------------------*/
DRange3d DRange3d::From (DPoint2dCP point, int n, double zVal)
    {
     DRange3d range;
     range.InitFrom (point, n, zVal);
     return range;
    }




/*-----------------------------------------------------------------*//**
*
* Initizlizes the range to contain the range of the given array of points.
* If there are no points in the array, the range is initialized by
* DRange3d#init()
*
* @param [in] point array of points to search
* @param [in] n number of points in array
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DRange3d::InitFrom (DPoint3dCP point, int n)
    {
    Init ();
    for (int i = 0; i < n; i++)
        Extend (point[i]);
    }


/*-----------------------------------------------------------------*//**
*
* Initizlizes the range to contain the range of the given array of points.
* If there are no points in the array, the range is initialized by
* DRange3d#init()
*
* @param [in] point array of points to search
* @param [in] pWeight array of corresponding weights
* @param [in] n number of points in array
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DRange3d::InitFrom (DPoint3dCP point, const double *pWeight, int n)
    {
    if (NULL == pWeight)
        InitFrom (point, n);
    else
        {
        Init ();
        for (int i = 0; i < n; i++)
            {
            Extend (point[i], pWeight[i]);
            }
        }
    }


/*-----------------------------------------------------------------*//**
*
* Initializes the range to contain the range of the given array of 2D points,
* with a single given z value for both the min and max points.
* If there are no points in the array, the range is initialized by
* DRange3d#init()
*
* @param [in] point array of points to search
* @param [in] n number of points in array
* @param [in] zVal default z value
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DRange3d::InitFrom (DPoint2dCP point, int n, double zVal)
    {
    Init ();
    for (int i = 0; i < n; i++)
        {
        Extend (point[i].x, point[i].y, zVal);
        }
    }



/*-----------------------------------------------------------------*//**
*
* extends the coordinates of the range cube by transformed points
*
* @param [in] transform transform to apply to points.
* @param [in] data new points to be included in minmax ranges
* @param [in] n number of points
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DRange3d::Extend (TransformCR transform, DPoint3dCP data, int n)
    {
    DPoint3d xyzLocal;
    for (int i = 0; i < n; i++)
        {
        if (!data[i].IsDisconnect ())
            {
            transform.Multiply (xyzLocal, data[i]);
            Extend (xyzLocal);
            }
        }
    }


/*-----------------------------------------------------------------*//**
*
* Initizlizes the range to contain the range of the given array of points.
* If there are no points in the array, the range is initialized by
* DRange3d#init()
*
* @param [in] point array of points to search
* @param [in] n number of points in array
+----------------------------------------------------------------------*/
DRange3d DRange3d::From (DPoint3dCP point, int n)
    {
     DRange3d range;
     range.InitFrom (point, n);
     return range;
    }

/*-----------------------------------------------------------------*//**
*
* Initizlizes the range to contain the range of the given array of points.
* If there are no points in the array, the range is initialized by
* DRange3d#init()
*
* @param [in] point array of points to search
* @param [in] pWeight array of corresponding weights
* @param [in] n number of points in array
+----------------------------------------------------------------------*/
DRange3d DRange3d::From (DPoint3dCP point, const double *pWeight, int n)
    {
     DRange3d range;
     range.InitFrom (point, pWeight, n);
     return range;
    }

/*-----------------------------------------------------------------*//**
*
* Initizlizes the range to contain the range of the given array of points.
* If there are no points in the array, the range is initialized by
* DRange3d#init()
*
* @param [in] point array of points to search
* @param [in] pWeight array of corresponding weights
* @param [in] n number of points in array
+----------------------------------------------------------------------*/
DRange3d DRange3d::From (TransformCR transform, DPoint3dCP points, const double *weights, int n)
    {
    DRange3d range;
    range.Init ();
    range.Extend (transform, points, weights, n);
    return range;
    }

void DRange3d::Extend (TransformCR transform, DPoint3dCP points, const double *weights, int n)
    {
    if (weights != nullptr)
        {
        DPoint3d xyz;
        for (int i = 0; i < n; i++)
            {
            xyz = points[i];
            transform.MultiplyWeighted (xyz, weights[i]);
            Extend (xyz, weights[i]);
            }
        }
    else
        {
        Extend (transform, points, n);
        }
    }


END_BENTLEY_GEOMETRY_NAMESPACE
