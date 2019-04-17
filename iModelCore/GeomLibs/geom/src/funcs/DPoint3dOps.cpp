/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>


BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static double s_defaultAbsTol = 1.0e-8;
static double s_defaultRelTol = 1.0e-8;

void Interpolate (double &result, double a, double f, double b)
    {
    result = a + f * (b - a);
    }

void Interpolate (DPoint3d &result, DPoint3d a, double f, DPoint3d b)
    {
    result.x = a.x + f * (b.x - a.x);
    result.y = a.y + f * (b.y - a.y);
    result.z = a.z + f * (b.z - a.z);
    }

void Interpolate (DPoint2d &result, DPoint2d a, double f, DPoint2d b)
    {
    result.x = a.x + f * (b.x - a.x);
    result.y = a.y + f * (b.y - a.y);
    }

void Interpolate (DVec3d &result, DVec3d a, double f, DVec3d b)
    {
    result.x = a.x + f * (b.x - a.x);
    result.y = a.y + f * (b.y - a.y);
    result.z = a.z + f * (b.z - a.z);
    }

void Interpolate (DVec2d &result, DVec2d a, double f, DVec2d b)
    {
    result.x = a.x + f * (b.x - a.x);
    result.y = a.y + f * (b.y - a.y);
    }

template <typename T>
T *VectorOps<T>::MallocAndCopy (bvector<T> &source)
    {
    size_t n = source.size ();
    if (n == 0)
        return NULL;
    size_t byteCount = n * sizeof (T);
    T * dest = (T*)BSIBaseGeom::Malloc (byteCount);
    memcpy (dest, &source[0], byteCount);
    return dest;
    }

template <typename T>
size_t VectorOps<T>::Size (bvector<T> *dest)
    {
    if (dest == NULL)
        return 0;
    return dest->size ();
    }

template <typename T>
bool VectorOps<T>::Set (bvector<T> *dest, T const &data, size_t index)
    {
    if (dest == NULL || index >= dest->size ())
        return false;
    dest->at (index) = data;
    return true;
    }

template <typename T>
size_t VectorOps<T>::Append (bvector<T> *dest, T const &data)
    {
    if (dest != NULL)
        {
        size_t index = dest->size ();
        dest->push_back (data);
        return index;
        }
    return 0;
    }

template <typename T>
bool VectorOps<T>::Get (bvector<T> *dest, T &data, size_t index)
    {
    if (dest == NULL || index >= dest->size ())
        return false;
    data = dest->at (index);
    return true;
    }

template <typename T>
void VectorOps<T>::Copy (bvector<T> *dest, bvector<T> const *source)
    {
    if (dest != NULL)
        {
        dest->clear ();
        if (source != NULL)
            {
            size_t n = source->size ();
            for (size_t i = 0; i < n; i++)
                dest->push_back (source->at(i));
            }
        }
    }

template <typename T>
void VectorOps<T>::Copy (bvector<T> *dest, T const *source, size_t n)
    {
    if (dest != NULL)
        {
        dest->clear ();
        if (source != NULL)
            {
            for (size_t i = 0; i < n; i++)
                dest->push_back (source[i]);
            }
        }
    }


template <typename T>
size_t VectorOps<T>::Append (bvector<T> *dest, T const *source, size_t n)
    {
    if (dest != NULL)
        {
        if (source != NULL)
            {
            size_t index = dest->size ();
            for (size_t i = 0; i < n; i++)
                dest->push_back (source[i]);
            return index;
            }
        }
    return 0;
    }

template <typename T>
void VectorOps<T>::AppendInterpolated (bvector<T> &dest, T const &first, T const &last, size_t count, bool includeFirst)
    {
    // always use direct push (no multiplications that lose bits) for first, last.
    if (count >= 2)
        {
        if (includeFirst)
            dest.push_back (first);
        double df = 1.0 / (double)(count-1);
        for (size_t i = 1; i + 1 < count; i++)
            {
            T value;
            double f = i * df;
            Interpolate (value, first, f, last);
            dest.push_back(value);
            }
        dest.push_back (last);
        }
    else if (count == 1)
        {
        if (includeFirst)
            dest.push_back (first);
        else
            dest.push_back (last);
        }
    }


template <typename T>
void VectorOps<T>::InterpolateAll (bvector<T> &dest, bvector <T> const &dataA, double f, bvector <T> const &dataB)
    {
    dest.clear ();
    size_t nA = dataA.size ();
    size_t nB = dataB.size ();
    size_t n = nA;
    if (nB < n)
        n = nB;
    for (size_t i = 0; i < nB; i++)
        {
        T result;
        Interpolate (result, dataA[i], f, dataB[i]);
        dest.push_back (result);
        }
    }


template <typename T>
size_t VectorOps<T>::Append (bvector<T> *dest, bvector<T> const *source)
    {
    if (dest != NULL)
        {
        if (source != NULL)
            {
            size_t index = dest->size ();
            size_t n = source->size ();
            for (size_t i = 0; i < n; i++)
                dest->push_back (source->at(i));
            return index;
            }
        }
    return 0;
    }


void initDisconnect (DPoint3dR data){data.x = data.y = data.z = DISCONNECT;}
void initDisconnect (DPoint2dR data){data.x = data.y = DISCONNECT;}
void initDisconnect (double &data)  {data = DISCONNECT;}

template <typename T>
void VectorOps<T>::AppendDisconnect (bvector<T> *dest)
    {
    if (dest != NULL)
        {
        T data;
        initDisconnect (data);
        dest->push_back (data);
        }
    }

template <typename T>
void VectorOps<T>::Reverse (bvector<T>& data)
    {
    if (data.empty ())
        return;
    for (size_t i = 0, j = data.size() - 1; i < j; i++, j--)
        {
        T a = data[i];
        data[i] = data[j];
        data[j] = a;
        }
    }

static bool equal (double a, double b) {return a == b;}
//static bool equal (int a, int b) {return a == b;}
//static bool equal (unsigned int a, unsigned int b) {return a == b;}
static bool equal (DPoint3d const &a, DPoint3d const &b) {return a.x == b.x && a.y == b.y && a.z == b.z;}
static bool equal (DPoint2d const &a, DPoint2d const &b) {return a.x == b.x && a.y == b.y;}

template <typename T>
bool VectorOps <T>::Equal (bvector<T> const &dataA, bvector<T>const &dataB)
    {
    size_t nA = dataA.size ();
    size_t nB = dataB.size ();
    if (nA != nB)
        return false;
    for (size_t i = 0; i < nA; i++)
        {
        if (!equal (dataA[i], dataB[i]))
            return false;
        }
    return true;
    }

template <typename T>
bool VectorOps <T>::Equal (T const *dataA, size_t nA, T const *dataB, size_t nB)
    {
    if (nA != nB)
        return false;
    for (size_t i = 0; i < nA; i++)
        {
        if (!equal (dataA[i], dataB[i]))
            return false;
        }
    return true;
    }


static void initDefault (DPoint3dR data)  { data.x = data.y = data.z = 0.0;}
static void initDefault (DPoint2dR data)  { data.x = data.y = 0.0;}
static void initDefault (DVec3dR   data)  { data.x = data.y = data.z = 0.0;}
//static void initDefault (double &data)  { data = 0.0;}

#define VECTOROPS_EXPLICIT_INSTANTIATION(T)\
template void       VectorOps<T>::Copy (bvector<T> *dest, bvector<T> const *source); \
template void       VectorOps<T>::Copy (bvector<T> *dest, T const *source, size_t count); \
template size_t     VectorOps<T>::Append (bvector<T> *dest, T const *source, size_t count); \
template size_t     VectorOps<T>::Append (bvector<T> *dest, bvector<T> const *source); \
template size_t     VectorOps<T>::Size (bvector<T> *dest); \
template size_t     VectorOps<T>::Append (bvector<T> *dest, T const &data); \
template void       VectorOps<T>::AppendDisconnect (bvector<T> *dest); \
template void       VectorOps<T>::AppendClosure(bvector<T> &dest, double tolerance); \
template bool       VectorOps<T>::Set (bvector<T> *dest, T const &data, size_t index); \
template bool       VectorOps<T>::Get (bvector<T> *dest, T &data, size_t index); \
template void       VectorOps<T>::AppendInterpolated (bvector<T> &dest, T const &first, T const &last, size_t count, bool includeFirst); \
template void       VectorOps<T>::InterpolateAll (bvector<T> &dest, bvector <T> const &dataA, double f, bvector <T> const &dataB); \
template T *        VectorOps<T>::MallocAndCopy (bvector<T> &source); \
template void       VectorOps<T>::Compress (bvector<T> &data, double tolerance); \
template void       VectorOps<T>::CompressCyclic (bvector<T> &data, double tolerance); \
template void       VectorOps<T>::Compress (bvector<T> const &source, bvector<T> &dest, double tolerance); \
template void       VectorOps<T>::Reverse (bvector<T>& xyz); \
template double     VectorOps<T>::LargestCoordinate (bvector<T> const &data); \
template double     VectorOps<T>::LargestCoordinate (T const *data, size_t count); \
template bool       VectorOps<T>::AlmostEqual(T const *valueA, size_t numA, T const *valueB, size_t numB, double tolerance);\
template size_t     VectorOps<T>::MostDistantIndex (T const *data, size_t n, T const &baseValue); \
template size_t     VectorOps<T>::MostDistantIndex (bvector<T> const &source, T const &baseValue); \
template double     VectorOps<T>::Tolerance (bvector<T> const &data, double absTol, double relTol); \
template double     VectorOps<T>::Tolerance (T const *data, size_t count, double absTol, double relTol); \
template bool       VectorOps<T>::Equal (bvector<T> const &dataA, bvector<T> const &dataB); \
template bool       VectorOps<T>::Equal (T const *dataA, size_t countA, T const *dataB, size_t countB); \
template bool       VectorOps<T>::FindNotAlmostEqualAtOrAfter (bvector<T>const &data, T const &baseValue, size_t i0, size_t &i1, T &value); \
template bool       VectorOps<T>::FindNotAlmostEqualBefore (bvector<T>const &data, T const &baseValue, size_t i0, size_t &i1, T &value);

// These particular methods are specialized or instantiated later
//template bool       VectorOps<T>::AlmostEqual (T const &valueA, T const &valueB); \
//template bool       VectorOps<T>::AlmostEqual (T const &valueA, T const &valueB, double tolerance);

VECTOROPS_EXPLICIT_INSTANTIATION(DPoint3d)
VECTOROPS_EXPLICIT_INSTANTIATION(DPoint2d)
VECTOROPS_EXPLICIT_INSTANTIATION(DVec3d)
VECTOROPS_EXPLICIT_INSTANTIATION(DVec2d)
VECTOROPS_EXPLICIT_INSTANTIATION(double)

double largestCoordinate (DPoint3dCR data){return data.MaxAbs ();}
double largestCoordinate (DPoint2dCR data){return data.MaxAbs ();}
double largestCoordinate (DVec3dCR   data){return data.MaxAbs ();}
double largestCoordinate (double const &data){return fabs (data);}

void initSortVector (DVec3dR data)  {data.x = 1.45; data.y = 0.13; data.z = 0.20; data.Normalize ();}
void initSortVector (DVec2dR data)  {data.x = 1.45; data.y = 0.13; data.Normalize ();}

double computeSortCoordinate (DPoint3dCR target, DPoint3dCR origin, DVec3dCR vector)
    {
    return 
          (target.x - origin.x) * vector.x
        + (target.y - origin.y) * vector.y
        + (target.z - origin.z) * vector.z
        ;
    }
double computeSortCoordinate (DVec3dCR target, DVec3dCR origin, DVec3dCR vector)
    {
    return 
          (target.x - origin.x) * vector.x
        + (target.y - origin.y) * vector.y
        + (target.z - origin.z) * vector.z
        ;
    }
double computeSortCoordinate (DPoint2dCR target, DPoint2dCR origin, DVec2dCR vector)
    {
    return 
          (target.x - origin.x) * vector.x
        + (target.y - origin.y) * vector.y
        ;
    }


static double distanceSquared (DPoint3dCR dataA, DPoint3dCR dataB) {return dataA.Distance (dataB);}
static double distanceSquared (DPoint2dCR dataA, DPoint2dCR dataB) {return dataA.Distance (dataB);}
static double distanceSquared (double const &dataA, double const & dataB) {return fabs (dataA - dataB);}

template <typename T>
void VectorOps<T>::AppendClosure (bvector<T> &dest, double tolerance)
    {
    if (dest.size () > 1)
        {
        double dd = distanceSquared (dest.front (), dest.back ());
        if (dd > tolerance * tolerance)
            {
            T data = dest.front ();   // extract before push to ensure no memory reference after reallocation
            dest.push_back (data);
            }
        else if (dd != 0.0)
            dest.back () = dest.front ();     // enforce exact closure.
        }
    }

template <typename T>
double VectorOps<T>::LargestCoordinate (bvector<T> const &data)
    {
    double aMax = 0.0;
    size_t n = data.size ();
    for (size_t i = 0; i < n; i++)
        {
        double a = largestCoordinate (data[i]);
        if (a > aMax)
            aMax = a;
        }
    return aMax;
    }

double DPoint3dOps::LargestXYCoordinate (DPoint3dCP data, size_t n)
    {
    double aMax = 0.0;
    for (size_t i = 0; i < n; i++)
        {
        double a = fabs (data[i].x);
        if (a > aMax)
            aMax = a;
        a = fabs (data[i].y);
        if (a > aMax)
            aMax = a;
        }
    return aMax;
    }
    
    
template <typename T>
bool VectorOps<T>::AlmostEqual (T const *valueA, size_t numA, T const *valueB, size_t numB, double tolerance)
    {
    if (numA != numB)
        return false;
    for (size_t i = 0; i < numA; i++)
        {
        if (!AlmostEqual (valueA[i], valueB[i], tolerance))
            return false;
        }
    return true;
    }


    
template <typename T>
size_t VectorOps<T>::MostDistantIndex (T const *data, size_t n, T const &baseValue)
    {
    double aMax = -1.0;
    size_t iMax = SIZE_MAX;
    for (size_t i = 0; i < n; i++)
        {
        double a = distanceSquared (baseValue, data[i]);
        if (a > aMax)
            {
            iMax = i;
            aMax = a;
            }
        }
    return iMax;
    }


template <typename T>
size_t VectorOps<T>::MostDistantIndex (bvector<T> const &data, T const &baseValue)
    {
    if (data.size () == 0)
        return SIZE_MAX;
    return MostDistantIndex (&data[0], data.size (), baseValue);
    }



template <typename T>
double VectorOps<T>::LargestCoordinate (T const *data, size_t n)
    {
    double aMax = 0.0;
    for (size_t i = 0; i < n; i++)
        {
        double a = largestCoordinate (data[i]);
        if (a > aMax)
            aMax = a;
        }
    return aMax;
    }


template <typename T>
double VectorOps<T>::Tolerance (bvector<T> const &data, double absTol, double relTol)
    {
    if (absTol < 0.0)
        absTol = s_defaultAbsTol;
    if (relTol < 0.0)
        relTol = s_defaultRelTol;
    double tol = absTol;
    if (relTol > 0.0)
        tol += relTol * VectorOps<T>::LargestCoordinate (data);
    return tol;
    }

template <typename T>
double VectorOps<T>::Tolerance (T const *data, size_t count, double absTol, double relTol)
    {
    if (absTol < 0.0)
        absTol = s_defaultAbsTol;
    if (relTol < 0.0)
        relTol = s_defaultRelTol;
    double tol = absTol;
    if (relTol > 0.0)
        tol += relTol * VectorOps<T>::LargestCoordinate (data, count);
    return tol;
    }


/** append a point to the bvector  */
void DPoint3dOps::AppendXYZ (bvector<DPoint3d> *dest, double x, double y, double z)
    {
    if (dest != NULL)
        {
        DPoint3d xyz;
        xyz.Init (x,y,z);
        dest->push_back (xyz);
        }
    }


void DPoint3dOps::AppendXY0 (bvector<DPoint3d> &dest, bvector<DPoint2d> const &source)
    {
    DPoint3d xy;
    size_t n = source.size ();
    dest.reserve (dest.size () + n);
    for (size_t i = 0; i < n; i++)
        {
        xy.x = source[i].x;
        xy.y = source[i].y;
        xy.z = 0.0;
        dest.push_back (xy);
        }
    }

void DPoint2dOps::AppendXY (bvector<DPoint2d> &dest, bvector<DPoint3d> const &source)
    {
    DPoint2d xy;
    size_t n = source.size ();
    dest.reserve (dest.size () + n);
    for (size_t i = 0; i < n; i++)
        {
        xy.x = source[i].x;
        xy.y = source[i].y;
        dest.push_back (xy);
        }
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     02/95
+---------------+---------------+---------------+---------------+---------------+------*/
bool DPoint2dOps::LexicalYExtrema
(
int &iMin,             /* <= index of point with min y (min x ) */
int &iMax,             /* <= index of point with max y (max x ) */
DPoint2dCP pointP,       /* => point array */
int nPoint              /* => number of points */
)
    {
    double yMin,yMax;
    DPoint2dCP currP;
    DPoint2dCP minP;
    DPoint2dCP maxP;
    int i;
    /* y lexical tests */
    iMin = 0;
    iMax = 0;
    if (nPoint <= 0)
        return false;    
    yMin = yMax = pointP[0].y;
    currP = minP = maxP = pointP;
    for ( i = 1 , currP = pointP + 1 ; i < nPoint ; i++, currP++ )
        {
        if  ( currP->y < yMin ||
                ( currP->y == yMin && currP->x < minP->x )
            )
            {
            iMin = i;
            yMin = currP->y;
            minP = currP;
            }
        else if ( currP->y > yMax ||
                ( currP->y == yMax && currP->x > maxP->x )
            )
            {
            iMax = i;
            yMax = currP->y;
            maxP = currP;
            }
        }
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* Find points with min and max x, using -y to resolve equal x (i.e. find lower left when viewing +X as 'up', +Y as 'LEFT')
* @bsimethod                                                    Earlin.Lutz     02/95
+---------------+---------------+---------------+---------------+---------------+------*/
bool DPoint2dOps::LexicalXExtrema
(
int &iMin,             /* <= index of point with min x (max y ) */
int &iMax,             /* <= index of point with max x (min y ) */
DPoint2dCP pointP,       /* => point array */
int nPoint              /* => number of points */
)
    {
    double xMin,xMax;
    DPoint2dCP currP;
    DPoint2dCP minP;
    DPoint2dCP maxP;
    int i;
    /* y lexical tests */
    iMin  = 0;
    iMax = 0;
    if (nPoint <= 0)
        return false;
    xMin = xMax = pointP[0].x;
    currP = minP = maxP = pointP;
    for ( i = 1 , currP = pointP + 1 ; i < nPoint ; i++, currP++ )
        {
        if  ( currP->x < xMin ||
                ( currP->x == xMin && currP->y > minP->y )
            )
            {
            iMin = i;
            xMin = currP->x;
            minP = currP;
            }
        else if ( currP->x > xMax ||
                ( currP->x == xMax && currP->y < maxP->y )
            )
            {
            iMax = i;
            xMax = currP->x;
            maxP = currP;
            }
        }
    return true;
    }




/** append a point to the bvector  */
void DVec3dOps::AppendXYZ (bvector<DVec3d> *dest, double x, double y, double z)
    {
    if (dest != NULL)
        {
        DVec3d xyz;
        xyz.Init (x,y,z);
        dest->push_back (xyz);
        }
    }

/** append a point to the bvector  */
void DPoint2dOps::AppendXY (bvector<DPoint2d> *dest, double x, double y)
    {
    if (dest != NULL)
        {
        DPoint2d xy;
        xy.x = x;
        xy.y = y;
        dest->push_back (xy);
        }
    }

/** append a point to the bvector  */
void DVec2dOps::AppendXY (bvector<DVec2d> *dest, double x, double y)
    {
    if (dest != NULL)
        {
        DVec2d xy;
        xy.x = x;
        xy.y = y;
        dest->push_back (xy);
        }
    }


void DPoint3dOps::Multiply
(
bvector<DPoint3d> *xyz,
TransformCR transform
)
    {
    if (xyz != NULL)
        {
        size_t n = xyz->size ();
        for (size_t i = 0; i < n; i++)
            if (!xyz->at(i).IsDisconnect ())
                transform.Multiply (xyz->at(i));
        }
    }

void DPoint3dOps::Add
(
bvector<DPoint3d> &xyz,
DVec3dCR delta
)
    {
    for (auto & item : xyz)
        if (!item.IsDisconnect ())
            item = item + delta;
    }

void DPoint2dOps::Multiply
(
bvector<DPoint2d> *xy,
TransformCR transform
)
    {
    if (xy != NULL)
        {
        size_t n = xy->size ();
        for (size_t i = 0; i < n; i++)
            if (!xy->at(i).IsDisconnect ())
                {
                DPoint2d xy0 = xy->at(i), xy1;
                transform.Multiply (&xy1, &xy0, 1);
                xy->at(i) = xy1;
                }
        }
    }

size_t DPoint3dOps::CountDisconnects (bvector<DPoint3d> &points)
    {
    size_t count = 0;
    for (size_t i = 0, n = points.size (); i < n; i++)
        {
        if (points[i].IsDisconnect ())
            count++;
        }
    return count;
    }

void DPoint3dOps::CompressDisconnects (bvector<DPoint3d> &points)
    {
    size_t numAccept = 0;
    for (size_t i = 0, n = points.size (); i < n; i++)
        {
        if (!points[i].IsDisconnect ())
            points[numAccept++] = points[i];
        }
    points.resize (numAccept);
    }


template <typename T>
bool VectorOps<T>::FindNotAlmostEqualAtOrAfter (bvector<T>const &data, T const &baseValue, size_t i0, size_t &i1, T &value)
    {
    size_t n = data.size ();
    i1 = i0 + 1;
    while (i1 < n)
        {
        if (!AlmostEqual (baseValue, data[i1]))
            {
            value = data[i1];
            return true;
            }
        i1++;
        }
    return false;
    }


template <typename T>
bool VectorOps<T>::FindNotAlmostEqualBefore (bvector<T>const &data, T const &baseValue, size_t i0, size_t &i1, T &value)
    {
    size_t n = data.size ();
    i1 = i0;
    if (i1 >= n)
        i1 = n;
    while (i1 > 0)
        {
        i1--;
        if (!AlmostEqual (baseValue, data[i1]))
            {
            value = data[i1];
            return true;
            }
        }
    return false;
    }

// Specializations of AlmostEqual
template<> bool VectorOps<double>::AlmostEqual (double const &a, double const &b)
    {
    double refValue = 1.0 + fabs (a) + fabs (b);
    return fabs (b-a) <= DoubleOps::SmallCoordinateRelTol () * refValue;
    }

template<> bool VectorOps<double>::AlmostEqual (double const &a, double const &b, double tol)
    {
    if (tol <= 0.0)
        return AlmostEqual (a, b);
    return fabs (b-a) <= tol;
    }
template<> bool VectorOps<DPoint3d>::AlmostEqual (DPoint3d const &a, DPoint3d const &b, double tol)
    {
    if (tol <= 0.0)
        return AlmostEqual (a, b);
    return a.DistanceSquared (b) <= tol * tol;
    }
template<> bool VectorOps<DPoint2d>::AlmostEqual (DPoint2d const &a, DPoint2d const &b, double tol)
    {
    if (tol <= 0.0)
        return AlmostEqual (a, b);
    return a.DistanceSquared (b) <= tol * tol;
    }
template<> bool VectorOps<DVec3d>::AlmostEqual (DVec3d const &a, DVec3d const &b, double tol)
    {
    if (tol <= 0.0)
        return AlmostEqual (a, b);
    return a.DistanceSquared (b) <= tol * tol;
    }
template<> bool VectorOps<DVec2d>::AlmostEqual (DVec2d const &a, DVec2d const &b, double tol)
    {
    if (tol <= 0.0)
        return AlmostEqual (a, b);
    return a.DistanceSquared (b) <= tol * tol;
    }

// Default tolerance version of AlmostEqual
template <typename T>
bool VectorOps<T>::AlmostEqual (T const &a, T const &b)
    {
    return a.AlmostEqual (b);
    }

// Explicit instantitions for default tolerance versions
template bool VectorOps<DPoint3d>::AlmostEqual (DPoint3d const &a, DPoint3d const &b);
template bool VectorOps<DPoint2d>::AlmostEqual (DPoint2d const &a, DPoint2d const &b);
template bool VectorOps<DVec3d>::AlmostEqual (DVec3d const &a, DVec3d const &b);
template bool VectorOps<DVec2d>::AlmostEqual (DVec2d const &a, DVec2d const &b);

template <typename T>
void VectorOps<T>::Compress (bvector<T> const &source, bvector<T> &dest, double tolerance)
    {
    dest.clear ();
    size_t nSource = source.size ();
    if (nSource > 0)
        {
        T xyzA, xyzB;
        xyzA = source[0];
        dest.push_back (xyzA);
        for (size_t i = 1; i < nSource; i++)
            {
            xyzB = source[i];
            if (!AlmostEqual (xyzA, xyzB, tolerance))
                dest.push_back (xyzB);
            xyzA = xyzB;
            }
        }
    }

template <typename T>
void VectorOps<T>::CompressCyclic (bvector<T> &data, double tolerance)
    {
    Compress (data, tolerance);
    T point0 = data.front ();
    for (size_t k = data.size (); --k > 0;)
        {
        if (!AlmostEqual (point0, data[k], tolerance))
            break;
        }
    }

template <typename T>
void VectorOps<T>::Compress (bvector<T> &data, double tolerance)
    {
    size_t nSource = data.size ();
    if (nSource > 0)
        {
        size_t numOut = 1;
        T xyzA, xyzB;
        xyzA = data[0];
        for (size_t i = 1; i < nSource; i++)
            {
            xyzB = data[i];
            if (!AlmostEqual (xyzA, xyzB, tolerance))
                data[numOut++] = xyzB;
            xyzA = xyzB;
            }
        data.resize (numOut);
        }
    }


bool DPoint3dOps::MinMaxDistance (bvector<DPoint3d> const &xyzA, bvector<DPoint3d> const &xyzB,
            double &minDistance, size_t &minIndex,
            double &maxDistance, size_t &maxIndex
            )
    {
    size_t countA = xyzA.size ();
    size_t countB = xyzB.size ();
    minIndex = maxIndex = 0;
    maxDistance = minDistance = 0.0;
    if (countA == 0 || countB == 0)
        return false;
    size_t count = countA < countB ? countA : countB;
    for (size_t i = 1; i < count; i++)
        {
        double a = xyzA[i].Distance (xyzB[i]);
        if (a < minDistance)
            {
            minDistance = a;
            minIndex = i;
            }
        if (a > maxDistance)
            {
            maxDistance = a;
            maxIndex = i;
            }
        }
    return true;
    }

bool DVec3dOps::MinMaxAngle (bvector<DVec3d> const &dataA, bvector<DVec3d> const &dataB,
            double &minAngle, size_t &minIndex,
            double &maxAngle, size_t &maxIndex
            )
    {
    size_t countA = dataA.size ();
    size_t countB = dataB.size ();
    minIndex = maxIndex = 0;
    minAngle = maxAngle = 0.0;
    if (countA == 0 || countB == 0)
        return false;
    size_t count = countA < countB ? countA : countB;
    for (size_t i = 1; i < count; i++)
        {
        double a = dataA[i].AngleTo (dataB[i]);
        if (a < minAngle)
            {
            minAngle = a;
            minIndex = i;
            }
        if (a > maxAngle)
            {
            maxAngle = a;
            maxIndex = i;
            }
        }
    return true;
    }



bool DPoint3dOps::MaxDistanceFromUnboundedRay (bvector<DPoint3d> const &xyz, DRay3dCR ray, size_t &index, double &maxDistance)
    {
    size_t count = xyz.size ();
    size_t numPoint = 0;
    maxDistance = 0.0;
    index = 0;
    double dd = -1.0;
    for (size_t i = 0; i < count; i++)
        {
        if (!xyz[i].IsDisconnect ())
            {
            DPoint3d xyzA;
            double   paramA;
            if (ray.ProjectPointUnbounded (xyzA, paramA, xyz[i]))
                {
                numPoint++;
                double ddi = xyzA.DistanceSquared (xyz[i]);
                if (ddi > dd)
                    {
                    dd = ddi;
                    index = i;
                    }
                }
            }
        }
    maxDistance = dd > 0.0 ? sqrt (dd) : 0.0;
    return numPoint > 0;
    }

bool DPoint3dOps::ClosestPoint (bvector<DPoint3d> const &xyz, DPoint3dCR spacePoint, size_t &closestIndex, double &minDist)
    {
    size_t count = xyz.size ();
    if (count == 0)
        {
        closestIndex = 0;
        minDist = 0.0;
        return false;
        }
    minDist = xyz[0].DistanceSquared (spacePoint);
    closestIndex = 0;
    // (Good sanitary habits: Search on squared distance, take one root at end.)
    for (size_t i = 1; i < count; i++)
        {
        double b = xyz[i].DistanceSquared (spacePoint);
        if (b < minDist)
            {
            minDist = b;
            closestIndex = i;
            }
        }
    minDist = sqrt (minDist);
    return true;
    }

bool DPoint3dOps::ClosestPoint
(
bvector<bvector<DPoint3d>> const &xyz,
DPoint3dCR spacePoint,
size_t &outerIndex,
size_t &innerIndex,
double &minDist
)
    {
    bool found = false;
    innerIndex = outerIndex = 0;
    minDist = DBL_MAX;
    for (size_t outer = 0; outer < xyz.size (); outer++)
        {
        double d;
        size_t inner;
        if (ClosestPoint (xyz[outer], spacePoint, inner, d)
            && d < minDist)
            {
            minDist = d;
            outerIndex = outer;
            innerIndex = inner;
            found = true;
            }
        }
    return found;
    }


bool DPoint3dOps::ClosestPointXY (bvector<DPoint3d> const &xyz, DMatrix4dCP worldToLocal, DPoint3dCR spacePoint, size_t &closestIndex, double &minDist)
    {
    size_t count = xyz.size ();
    closestIndex = 0;
    minDist = 0.0;
    if (count == 0)
        return false;
    minDist = DBL_MAX;
    DPoint3d xyPoint = spacePoint;
    if (NULL != worldToLocal)
        worldToLocal->MultiplyAndRenormalize (&xyPoint, &spacePoint, 1);
    // (Good sanitary habits: Search on squared distance, take one root at end.)
    for (size_t i = 0; i < count; i++)
        {
        DPoint3d xyz1 = xyz[i];
        if (worldToLocal)
            worldToLocal->MultiplyAndRenormalize (&xyz1, &xyz1, 1);
        double b = xyz1.DistanceSquaredXY (xyPoint);
        if (b < minDist)
            {
            minDist = b;
            closestIndex = i;
            }
        }
    minDist = sqrt (minDist);
    return true;
    }



DRange3d DPoint3dOps::Range
(
bvector<DPoint3d> const *pXYZIn
)
    {
    DRange3d range;
    range.Init ();
    size_t n = pXYZIn->size ();
    for (size_t i = 0; i < n; i++)
        {
        range.Extend (pXYZIn->at(i));
        }
    return range;
    }

DRange3d DPoint3dOps::Range
(
bvector<DPoint3d> const *pXYZIn,
TransformCR worldToLocal
)
    {
    DRange3d range;
    range.Init ();
    size_t n = pXYZIn->size ();
    for (size_t i = 0; i < n; i++)
        {
        DPoint3d xyz = pXYZIn->at(i);
        worldToLocal.Multiply (xyz);
        range.Extend (xyz);
        }
    return range;
    }

DRange1d DPoint3dOps::ProjectedParameterRange (bvector<DPoint3d> const &points, DRay3dCR ray)
    {
    DRange1d range;
    range.InitNull ();
    size_t n = points.size ();
    double divAA = ray.direction.SafeOneOverMagnitudeSquared (0.0);
    for (size_t i = 0; i < n; i++)
        {
        range.Extend (divAA * ray.DirectionDotVectorToTarget (points[i]));
        }
    return range;
    }





//! Query object used by cluster analysis.
struct ClusterQueries
{
//! Return total number of cluster candidates ...
GEOMAPI_VIRTUAL size_t GetNumCandidate () = 0;
//! Return a first-pass sort coordinate for a candidate ...
GEOMAPI_VIRTUAL double GetSortCoordinate (size_t candidateIndex) = 0;
//! Indicate if specified sort coordinates are close enough to warrant true candidate proximity test.
GEOMAPI_VIRTUAL double AreSortCoordinatesClose (double sortCoordinateA, double sortCoordinateB) = 0;

//! test if two candidates are really close (after first pass sort has determined that their sort coordinates are close)
GEOMAPI_VIRTUAL bool   AreCandidatesClose (size_t candidateIndexA, size_t candidateIndexB) = 0;

//! Announce beginning of a cluster
GEOMAPI_VIRTUAL void BeginCluster (size_t clusterIndex, size_t baseCandidate) {}
//! Announce accepted candidate pair.
//! numInCluster is inclusive of the base candidate and the pairedCandidates so far.
GEOMAPI_VIRTUAL void   AddToCluster (size_t clusterIndex, size_t clusterCount, size_t baseCandidate, size_t pairedCandidate) {}
//! Announce end of a cluster.
GEOMAPI_VIRTUAL void   EndCluster (size_t clusterIndex, size_t clusterCount, size_t baseCandidate) {}
//! Announce end of all clusters
GEOMAPI_VIRTUAL void EndClusterSweep (size_t numCluster) {}
};

struct ClusterCandidate
{
double m_sortCoordinate;
size_t m_externalIndex;
size_t m_parentPosition;
ClusterCandidate (size_t externalIndex, size_t parentPosition, double sortCoordinate)
    : m_sortCoordinate (sortCoordinate),
      m_externalIndex  (externalIndex),
      m_parentPosition (parentPosition)
    {
    }

bool operator < (ClusterCandidate const &other) const
    {
    return m_sortCoordinate < other.m_sortCoordinate;
    }
};

//! Algorithmic logic for clustering, refering to candidate data only through generic query object.
size_t ClusterBySortSweep (ClusterQueries &source)
    {
    bvector<ClusterCandidate> candidates;
    size_t numCandidate = source.GetNumCandidate ();
    for (size_t i = 0; i < numCandidate; i++)
        {
        candidates.push_back (ClusterCandidate (i, i, source.GetSortCoordinate(i)));
        }
    std::sort(candidates.begin (), candidates.end ());
    for (size_t i = 0; i < numCandidate; i++)
        candidates[i].m_parentPosition = i;
    size_t clusterIndex = 0;
    for (size_t basePosition = 0; basePosition < numCandidate; basePosition++)
        {
        if (candidates[basePosition].m_parentPosition == basePosition) // This candidate has NOT been clustered to one earlier in the sort order
            {
            size_t clusterCount = 1;
            source.BeginCluster (clusterIndex, candidates[basePosition].m_externalIndex);
            for (size_t testPosition = basePosition + 1;
                       testPosition < numCandidate
                    && source.AreSortCoordinatesClose (candidates[basePosition].m_sortCoordinate, candidates[testPosition].m_sortCoordinate);
                    testPosition++
                )
                {
                if (source.AreCandidatesClose (candidates[basePosition].m_externalIndex, candidates[testPosition].m_externalIndex))
                    {
                    clusterCount++;
                    candidates[testPosition].m_parentPosition = basePosition;   // later visit by basePosition loop will skip testPosition
                    source.AddToCluster (clusterIndex, clusterCount, candidates[basePosition].m_externalIndex, candidates[testPosition].m_externalIndex);
                    }
                }
            source.EndCluster (clusterIndex, clusterCount, candidates[basePosition].m_externalIndex);
            clusterIndex++;
            }
        }
    source.EndClusterSweep (clusterIndex);
    return clusterIndex;
    }


struct NonDimensionalClusterContext : ClusterQueries
{
bvector<size_t>   &m_oldIndexToPackedIndex;
size_t m_numIn;
size_t m_numOut;

double m_sortTolerance;

protected:
void SetSortTolerance (double tol)  {m_sortTolerance = tol;}

protected:
NonDimensionalClusterContext
(
size_t numOldIndex,
bvector<size_t>   &oldIndexToPackedIndex
)
: m_oldIndexToPackedIndex(oldIndexToPackedIndex)
    {
    m_numIn = numOldIndex;
    m_numOut = 0;
    SetSortTolerance (0.0);
    m_oldIndexToPackedIndex.clear();
    m_oldIndexToPackedIndex.reserve (m_numIn);
    for (size_t i = 0; i < m_numIn; i++)
        m_oldIndexToPackedIndex.push_back (i);
    }


//! Return total number of cluster candidates ...
size_t GetNumCandidate () override { return m_numIn;}
//! Return a first-pass sort coordinate for a candidate ...
// --- GEOMAPI_VIRTUAL double GetSortCoordinate (size_t candidateIndex) = 0;

//! Indicate if specified sort coordinates are close enough to warrant true candidate proximity test.
double AreSortCoordinatesClose (double sortCoordinateA, double sortCoordinateB) override
    {
    return fabs (sortCoordinateA - sortCoordinateB) <= m_sortTolerance;
    }

//! test if two candidates are really close (after first pass sort has determined that their sort coordinates are close)
//! Derived class must implement .....
bool   AreCandidatesClose (size_t candidateIndexA, size_t candidateIndexB) override { return false;}

//! Announce beginning of a cluster
void BeginCluster (size_t clusterIndex, size_t baseCandidate) override
    {
    m_oldIndexToPackedIndex[baseCandidate] = clusterIndex;
    }
//! Announce accepted candidate pair.
//! numInCluster is inclusive of the base candidate and the pairedCandidates so far.
void   AddToCluster (size_t clusterIndex, size_t clusterCount, size_t baseCandidate, size_t pairedCandidate) override 
    {
    m_oldIndexToPackedIndex[pairedCandidate] = clusterIndex;
    }
//! Announce end of a cluster.
void   EndCluster (size_t clusterIndex, size_t clusterCount, size_t baseCandidate) override 
    {
    }
};


//===============================================================================================================================
//! Cluster query specialized for array of DatumType, with VectorType used for computing a sort coordinate.
//! At time of expansion, various statically overloaded functions must be available for the templatized types:
//!    VectorOps<DatumType>::LargestCoordinate
//!     initDefault (DatumType &)
//!     initSortVector (VectorType &)
//!     computeSortCoordinate (DatumType &, VectorType &)
//! Avaialable <DatumType, VectorType> combinations (Sept. 4, 2009) are:
//!    <DPoint3d, DVec3d>
//!    <DVec3d, DVec3d>
//!    <DPoint2d, DVec2d>
//===============================================================================================================================
template <typename DatumType, typename VectorType>
struct ClusterContextTemplate : NonDimensionalClusterContext
{
bvector <DatumType> const &m_xyz;
DatumType   m_refPoint;
VectorType   m_sortDirection;

double m_xyzTolerance;
double m_xyzToleranceSquared;
void SetTolerances (double abstol, double reltol)
    {
    m_xyzTolerance = VectorOps<DatumType>::Tolerance (m_xyz, abstol, reltol);
    m_xyzToleranceSquared = m_xyzTolerance * m_xyzTolerance;
    SetSortTolerance (2.0 * m_xyzTolerance);
    }


ClusterContextTemplate
(
bvector<DatumType> const &xyz,
bvector<size_t>   &oldIndexToPackedIndex,
double absTol,
double relTol
)
: m_xyz(xyz),
    NonDimensionalClusterContext (xyz.size (), oldIndexToPackedIndex)
    {
    initDefault (m_refPoint);
    initSortVector (m_sortDirection);
    if (m_numIn > 0)
        {
        m_refPoint = m_xyz[0];
        SetTolerances (absTol, relTol);
        }
    }

double GetSortCoordinate (size_t index)  override {return computeSortCoordinate (m_xyz[index], m_refPoint, m_sortDirection);}
bool AreCandidatesClose (size_t indexA, size_t indexB) override {return m_xyz[indexA].DistanceSquared (m_xyz[indexB]) <= m_xyzToleranceSquared;}
};
typedef ClusterContextTemplate <DPoint3d, DVec3d>   DPoint3dClusterContext;
typedef ClusterContextTemplate <DVec3d, DVec3d>     DVec3dClusterContext;
typedef ClusterContextTemplate <DPoint2d, DVec2d>   DPoint2dClusterContext;

//!
//! @param [in] source original data
//! @param [out] receiver array.  (Possibly NULL)
//! @param [in] oldToNew At each position in source, index of target in dest.
//! @param [in] defaultValue Default value for destination
//! @param [in] numDest Number of positions in destination.
template <typename T>
static void PackAfterCluster (bvector<T> const &source, bvector<T> *dest, bvector<size_t> sourceToDest, T const &defaultValue, size_t numDest)
    {
    if (NULL != dest)
        {
        dest->clear ();
        if (numDest > 0)
            {
            dest->clear ();
            dest->reserve (numDest);

            for (size_t i = 0; i < numDest; i++)
                dest->push_back (defaultValue);
            size_t numIndex = sourceToDest.size ();
            for (size_t i = 0; i < numIndex; i++)
                dest->at(sourceToDest[i]) = source[i];
            }
        }
    }

size_t DPoint3dOps::Cluster
(
bvector<DPoint3d> const &xyzIn,
bvector<DPoint3d> *xyzOut,
bvector<size_t>&oldIndexToPackedIndex,
double absTol,
double relTol
)
    {
    DPoint3dClusterContext context (xyzIn, oldIndexToPackedIndex, absTol, relTol);
    size_t numCluster = ClusterBySortSweep (context);
    DPoint3d zero;
    zero.Zero ();
    PackAfterCluster <DPoint3d> (xyzIn, xyzOut, oldIndexToPackedIndex, zero, numCluster);
    return numCluster;
    }

size_t DPoint2dOps::Cluster
(
bvector<DPoint2d> const &xyzIn,
bvector<DPoint2d> *xyzOut,
bvector<size_t>&oldIndexToPackedIndex,
double absTol,
double relTol
)
    {
    DPoint2dClusterContext context (xyzIn, oldIndexToPackedIndex, absTol, relTol);
    size_t numCluster = ClusterBySortSweep (context);
    DPoint2d zero;
    zero.Zero ();
    PackAfterCluster <DPoint2d> (xyzIn, xyzOut, oldIndexToPackedIndex, zero, numCluster);
    return numCluster;
    }

size_t DVec3dOps::Cluster
(
bvector<DVec3d> const &xyzIn,
bvector<DVec3d> *xyzOut,
bvector<size_t>&oldIndexToPackedIndex,
double absTol,
double relTol
)
    {
    DVec3dClusterContext context (xyzIn, oldIndexToPackedIndex, absTol, relTol);
    size_t numCluster = ClusterBySortSweep (context);
    DVec3d zero;
    zero.Zero ();
    PackAfterCluster <DVec3d> (xyzIn, xyzOut, oldIndexToPackedIndex, zero, numCluster);
    return numCluster;
    }

bool DPoint3dOps::PrincipalAxes (bvector<DPoint3d> const &points, TransformR localToWorld, TransformR worldToLocal, DVec3dR moments)
    {
    RotMatrix axes;
    DVec3d centroid;
    if (!PrincipalAxes (points, centroid, axes, moments))
        {
        localToWorld.InitIdentity ();
        worldToLocal.InitIdentity ();
        return false;
        }
    localToWorld.InitFrom (axes, centroid);
    worldToLocal.InvertRigidBodyTransformation (localToWorld);
    return true;        
    }

bool DPoint3dOps::PrincipalAxes (bvector<DPoint3d> const &pointsA, bvector<DPoint3d> const &pointsB, TransformR localToWorld, TransformR worldToLocal, DVec3dR moments)
    {
    RotMatrix axes;
    DVec3d centroid;
    if (!PrincipalAxes (pointsA, pointsB, centroid, axes, moments))
        {
        localToWorld.InitIdentity ();
        worldToLocal.InitIdentity ();
        return false;
        }
    localToWorld.InitFrom (axes, centroid);
    worldToLocal.InvertRigidBodyTransformation (localToWorld);
    return true;        
    }



bool    DPoint3dOps::PrincipalExtents (bvector<DPoint3d> const &points, TransformR originWithExtentVectors, TransformR localToWorldOut, TransformR worldToLocalOut)
    {
    Transform localToWorld0, worldToLocal0;
    DVec3d moments;
    if (DPoint3dOps::PrincipalAxes (points, localToWorld0, worldToLocal0, moments))
        {
        DRange3d localRange = DPoint3dOps::Range (&points, worldToLocal0);
        DRange3d localRange1;
        return LocalRangeToOrderedExtents (localToWorld0, localRange, originWithExtentVectors, localToWorldOut, worldToLocalOut, localRange1);
        }
    localToWorldOut.InitIdentity ();
    worldToLocalOut.InitIdentity ();
    originWithExtentVectors.InitIdentity ();
    return false;
    }
bool DPoint3dOps::LocalRangeToOrderedExtents
(
TransformCR localToWorld0, //!< [in] local coordinate frame (assumed rigid)
DRange3dCR localRange,    //!< [in] range cube in local coordinates
TransformR extentTransform,  //!< [out] transform with origin at lower left of range, xyz columns as full extent of the ranges, with x largest, then y, and z smallest.
TransformR localToWorldOut,  //!< [out] rigid frame
TransformR worldToLocalOut,   //!< [out] inverse of rigid frame.
DRange3dR  localRangeOut     //!< [out] range in sorted system.
)
    {
    RotMatrix axes;
    localToWorld0.GetMatrix (axes);
    if (axes.IsOrthogonal ())
        {
        DVec3d diagonal = DVec3d::FromStartEnd (localRange.low, localRange.high);
        int ix, iy, iz;
        Angle::Cyclic3dAxes (iz, ix, iy, diagonal.MinAbsIndex ());
        DVec3d xAxis, yAxis, zAxis;
        DPoint3d centroid;
        localToWorld0.GetTranslation (centroid);
        localToWorld0.GetMatrixColumn (xAxis, ix);
        localToWorld0.GetMatrixColumn (yAxis, iy);
        localToWorld0.GetMatrixColumn (zAxis, iz);
        if (zAxis.z < 0.0)
            {
            zAxis.Negate ();
            xAxis.Negate ();
            }
        if (xAxis.x < 0.0)
            {
            xAxis.Negate ();
            yAxis.Negate ();
            }

        localToWorldOut.InitFromOriginAndVectors (centroid, xAxis, yAxis, zAxis);
        worldToLocalOut.InverseOf (localToWorldOut);
        Transform oldLocalToNewLocal = worldToLocalOut * localToWorld0;
        oldLocalToNewLocal.Multiply (localRangeOut, localRange);
        DVec3d diagonal1 = DVec3d::FromStartEnd (localRangeOut.low, localRangeOut.high);
        DPoint3d worldOrigin;
        localToWorldOut.Multiply (worldOrigin, localRangeOut.low);
        xAxis.Scale (diagonal1.x);
        yAxis.Scale (diagonal1.y);
        zAxis.Scale (diagonal1.z);
        if (diagonal.y > diagonal.x)
            {
            worldOrigin = DPoint3d::FromSumOf (worldOrigin, yAxis);
            std::swap (xAxis, yAxis);
            xAxis.Negate ();
            }
        extentTransform.InitFromOriginAndVectors (worldOrigin, xAxis, yAxis, zAxis);
        }
    return true;
    }

bool    DPoint3dOps::PrincipalExtents (bvector<DPoint3d> const &points, TransformR originWithExtentVectors)
    {
    Transform localToWorld, worldToLocal;
    return PrincipalExtents (points, originWithExtentVectors, localToWorld, worldToLocal);
    }

static void SwapLargeWForward (DPoint4d data[], int i0, int i1)
    {
    if (data[i0].w < data[i1].w)
        {
        std::swap (data[i0], data[i1]);
        }
    }
static void SetMoment (RotMatrixR dest, double &a, int j, DPoint4dCR data)
    {
    a = data.w;
    dest.form3d[0][j] = data.x;
    dest.form3d[1][j] = data.y;
    dest.form3d[2][j] = data.z;
    }

static void SortMoments (RotMatrixR axes, DVec3dR moments)
    {
    DVec3d axisVector[3];
    DPoint4d sortPoint[3];
    axes.GetColumns (axisVector[0], axisVector[1], axisVector[2]);
    sortPoint[0].InitFrom (axisVector[0], moments.x);
    sortPoint[1].InitFrom (axisVector[1], moments.y);
    sortPoint[2].InitFrom (axisVector[2], moments.z);
    SwapLargeWForward (sortPoint, 0, 1);
    SwapLargeWForward (sortPoint, 0, 2);
    SwapLargeWForward (sortPoint, 1, 2);
    SetMoment (axes, moments.x, 0, sortPoint[0]);
    SetMoment (axes, moments.y, 1, sortPoint[1]);
    SetMoment (axes, moments.z, 2, sortPoint[2]);
    if (axes.Determinant () < 0.0)
        axes.ScaleColumns (1.0, 1.0, -1.0);
    }

void DPoint3dOps::AccumulateToMomentSumUpperTriangle (DMatrix4dR sums, DPoint3dCR origin, DPoint3dCR xyz)
    {
    DVec3d uvw = xyz - origin;
    sums.coff[0][0] += uvw.x * uvw.x;
    sums.coff[0][1] += uvw.x * uvw.y;
    sums.coff[0][2] += uvw.x * uvw.z;
    sums.coff[0][3] += uvw.x;
    sums.coff[1][1] += uvw.y * uvw.y;
    sums.coff[1][2] += uvw.y * uvw.z;
    sums.coff[1][3] += uvw.y;
    sums.coff[2][2] += uvw.z * uvw.z;
    sums.coff[2][3] += uvw.z;
    sums.coff[3][3] += 1.0;
    }
DMatrix4d DPoint3dOps::MomentSums (DPoint3dCR origin, bvector<DPoint3d> const &points)
    {
    DPoint3d uvw;
    DMatrix4d sums = DMatrix4d::FromZero ();
    for (DPoint3d xyz : points)
        {
        uvw.DifferenceOf (xyz, origin);
        sums.coff[0][0] += uvw.x * uvw.x;
        sums.coff[0][1] += uvw.x * uvw.y;
        sums.coff[0][2] += uvw.x * uvw.z;
        sums.coff[0][3] += uvw.x;
        sums.coff[1][1] += uvw.y * uvw.y;
        sums.coff[1][2] += uvw.y * uvw.z;
        sums.coff[1][3] += uvw.y;
        sums.coff[2][2] += uvw.z * uvw.z;
        sums.coff[2][3] += uvw.z;
        }

    // fill in symmetric and count terms.
    sums.coff[1][0] = sums.coff[0][1];
    sums.coff[2][0] = sums.coff[0][2];
    sums.coff[2][1] = sums.coff[1][2];

    sums.coff[3][0] = sums.coff[0][3];
    sums.coff[3][1] = sums.coff[1][3];
    sums.coff[3][2] = sums.coff[2][3];
    
    sums.coff[3][3] = (double)points.size ();
    return sums;
    }
    
bool DPoint3dOps::PrincipalAxes (bvector<DPoint3d> const &points, DVec3dR centroid, RotMatrixR axes, DVec3dR moments)
    {
    if (points.size () == 0)
        return false;
    // Precompute centroid as local origin
    DVec3d centroid0;
    centroid0.Zero ();
    DVec3d delta;
    for (size_t i = 0; i < points.size (); i++)
        {
        delta.Init (points[i]);
        centroid0.Add (delta);
        }
    centroid0.Scale (1.0 / (double) points.size ());
#define suppressShiftnot
#ifdef suppressShift
/*
Effect of precentering on moment calculations.
8 point data set on slab corners, rotated and translated by increasing amounts.
  relativeShift        shiftedError                  unshiftedError
 (shiftMagnitude 1) (momentDiff 1.13687e-013)(momentDiff 1.13687e-013)
 (shiftMagnitude 10) (momentDiff 5.68434e-014)(momentDiff 1.19371e-012)
 (shiftMagnitude 100) (momentDiff 5.11591e-013)(momentDiff 2.60911e-011)
 (shiftMagnitude 1000) (momentDiff 4.43379e-012)(momentDiff 4.1174e-009)
 (shiftMagnitude 10000) (momentDiff 5.67297e-011)(momentDiff 1.37837e-006)
 (shiftMagnitude 100000) (momentDiff 4.84079e-010)(momentDiff 2.41563e-005)
 (shiftMagnitude 1e+006) (momentDiff 3.95971e-009)(momentDiff 0.00719827)
 (shiftMagnitude 1e+007) (momentDiff 3.77652e-008)(momentDiff 0.164517)
*/
     centroid0.Zero ();
#endif
    DMatrix4d sums = MomentSums (centroid0, points);
    double volume;
    Transform localToWorld;
    localToWorld.InitIdentity ();
    DVec3d centroid1;
    bool stat =  sums.ConvertInertiaProductsToPrincipalMoments (localToWorld,
            volume, centroid1, axes, moments);
    centroid.SumOf (centroid0, centroid1);
    SortMoments (axes, moments);
    return stat;
    }

bool DPoint3dOps::PrincipalAxes (bvector<DPoint3d> const &pointsA, bvector<DPoint3d> const &pointsB, DVec3dR centroid, RotMatrixR axes, DVec3dR moments)
    {
    size_t numPoints = pointsA.size () + pointsB.size ();
    if (numPoints == 0)
        return false;
    // Precompute centroid as local origin
    DVec3d centroid0;
    centroid0.Zero ();
    DVec3d delta;
    for (size_t i = 0; i < pointsA.size (); i++)
        {
        delta.Init (pointsA[i]);
        centroid0.Add (delta);
        }
    for (size_t i = 0; i < pointsB.size (); i++)
        {
        delta.Init (pointsB[i]);
        centroid0.Add (delta);
        }
    centroid0.Scale (1.0 / (double) numPoints);
    
    
    DMatrix4d sums = MomentSums (centroid0, pointsA);
    DMatrix4d sumsB = MomentSums (centroid0, pointsB);
    sums.Add (sumsB);
    double volume;
    Transform localToWorld;
    localToWorld.InitIdentity ();
    DVec3d centroid1;
    bool stat =  sums.ConvertInertiaProductsToPrincipalMoments (localToWorld,
            volume, centroid1, axes, moments);
    centroid.SumOf (centroid0, centroid1);
    SortMoments (axes, moments);
    return stat;
    }




bool DPoint3dOps::PrincipalAxes (bvector<DPoint4d> const &points, DVec3dR centroid, RotMatrixR axes, DVec3dR moments)
    {
    if (points.size () == 0)
        return false;
    // Precompute centroid as local origin
    DVec3d centroid0;
    centroid0.Zero ();
    DVec3d delta;
    for (size_t i = 0; i < points.size (); i++)
        {
        DPoint3d xyz;
        if (!points[i].GetProjectedXYZ (xyz))
            continue;
        delta.Init (xyz);
        centroid0.Add (delta);
        }
    centroid0.Scale (1.0 / (double) points.size ());

    DMatrix4d sums = DMatrix4d::FromZero ();
    for (size_t i = 0;  i < points.size (); i++)
        {
        DPoint3d xyz;
        if (!points[i].GetProjectedXYZ (xyz))
            continue;
            
        DVec3d uvw;
        uvw.DifferenceOf (xyz, centroid0);
        sums.coff[0][0] += uvw.x * uvw.x;
        sums.coff[0][1] += uvw.x * uvw.y;
        sums.coff[0][2] += uvw.x * uvw.z;
        sums.coff[0][3] += uvw.x;
        sums.coff[1][1] += uvw.y * uvw.y;
        sums.coff[1][2] += uvw.y * uvw.z;
        sums.coff[1][3] += uvw.y;
        sums.coff[2][2] += uvw.z * uvw.z;
        sums.coff[2][3] += uvw.z;
        }
    // fill in symmetric and count terms.
    sums.coff[1][0] = sums.coff[0][1];
    sums.coff[2][0] = sums.coff[0][2];
    sums.coff[2][1] = sums.coff[1][2];

    sums.coff[3][0] = sums.coff[0][3];
    sums.coff[3][1] = sums.coff[1][3];
    sums.coff[3][2] = sums.coff[2][3];
    
    sums.coff[3][3] = (double)points.size ();
    double volume;
    Transform localToWorld;
    localToWorld.InitIdentity ();
    DVec3d centroid1;
    bool stat =  sums.ConvertInertiaProductsToPrincipalMoments (localToWorld,
            volume, centroid1, axes, moments);
    centroid.SumOf (centroid0, centroid1);
    return stat;
    }

void CompressByChordRec (bvector<DPoint3d>& result, bvector<DPoint3d> const& source, double chordTolerance)
    {
    //Note that loops are supported
    if (source.size () <= 2 || chordTolerance <= 0.0)
        {
        result = source;
        return;
        }

    DSegment3d segment = DSegment3d::From (source[0], source.back ());

    double const quadraticTolerance = chordTolerance * chordTolerance;

    DPoint3d closestPt;
    double paramBuffer;
    size_t indexMaxError = 0;
    double maxQuadError = 0.0;
    for (size_t i = 1; i < source.size () - 1; ++i)
        {
        segment.ProjectPointBounded (closestPt, paramBuffer, source[i]);
        double curQuadError = closestPt.DistanceSquared (source[i]);
        if (curQuadError > quadraticTolerance && curQuadError > maxQuadError)
            {
            maxQuadError = curQuadError;
            indexMaxError = i;
            }
        }

    if (maxQuadError > quadraticTolerance && indexMaxError != 0)
        {
        bvector<DPoint3d> leftPoints, rightPoints;
        leftPoints.insert (leftPoints.end (), source.begin (), source.begin () + indexMaxError + 1);
        CompressByChordRec (leftPoints, leftPoints, chordTolerance);

        rightPoints.insert (rightPoints.end (), source.begin () + indexMaxError, source.end ());
        CompressByChordRec (rightPoints, rightPoints, chordTolerance);

        result = leftPoints;
        if (rightPoints.size () >= 2)
            result.insert (result.end (), rightPoints.begin () + 1, rightPoints.end ());
        }
    else
        {
        result = { source[0], source.back () };
        }
    }

bool IsLoop (bvector<DPoint3d> const& points)
    {
    return points.size () >= 2 && points[0].AlmostEqual (points.back ());
    }

void DPoint3dOps::CompressByChordError (bvector<DPoint3d>& result, bvector<DPoint3d> const& source, double chordTolerance)
    {
    bool const isInitLoop = IsLoop (source);

    CompressByChordRec (result, source, chordTolerance);
    
    //cleanup in case of loops
    if (isInitLoop && result.size () >= 2 && !result[0].AlmostEqual (result.back ()))
        result.push_back (result[0]);
    }

END_BENTLEY_GEOMETRY_NAMESPACE
