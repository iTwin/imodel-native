/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static void (*sFreeCurve)(MSBsplineCurve *) = 0;
static int (*sAllocateCurve)(MSBsplineCurve *) = 0;

#define MDLERR_NOPOLES ERROR
#define MDLERR_INSFMEMORY ERROR

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool curveFromArc
(
DPoint3d        *poles,                /* <= poles of arc */
double          *knots,                /* <= interior knots only */
double          *weights,
BsplineParam    *params,
double          start,
double          sweep,
double          axis1,
double          axis2
)
    {
    int         numSections;
    double      delta, alpha, c, wght;
    DPoint3d    midPoint;

    poles[0].z = poles[1].z = poles[2].z =
    poles[3].z = poles[4].z = poles[5].z =
    poles[6].z = midPoint.z = 0.0;

    /* Test arc angle : if greater than 120 degrees use multiple segments */
    numSections = (fabs(sweep) <= msGeomConst_2pi/3.0 ? 1:
                  (fabs(sweep) <= 2.0*msGeomConst_2pi/3.0 ? 2:
                   3));

    int bClosed = Angle::IsFullCircle (sweep);

    /* Calculate weights */
    alpha = 0.5 * fabs(sweep) / numSections;
    wght = cos (alpha);
    weights[1] = weights[3] = weights[5] = wght;
    weights[0] = weights[2] = weights[4] = weights[6] = 1.0;

    /* Simplifies formula below */
    c = 2.0*(wght + 1.0);
    wght *= 2.0;

    /*Calculate poles of b-spline*/

    /* Start of arc */
    poles[0].x = axis1*cos (start);
    poles[0].y = axis2*sin (start);

    /* End of arc */
    delta = start + sweep;
    poles[2].x = axis1*cos (delta);
    poles[2].y = axis2*sin (delta);

    if (numSections == 3)
        {
        delta = sweep/6.0;
        poles[6] = poles[2];

        poles[2].x = axis1*cos (start + 2.0*delta);
        poles[2].y = axis2*sin (start + 2.0*delta);

        midPoint.x = axis1*cos (start + delta);
        midPoint.y = axis2*sin (start + delta);

        poles[1].x = (midPoint.x * c - (poles[0].x + poles[2].x))/wght;
        poles[1].y = (midPoint.y * c - (poles[0].y + poles[2].y))/wght;

        poles[4].x = axis1*cos (start + 4.0*delta);
        poles[4].y = axis2*sin (start + 4.0*delta);

        midPoint.x = axis1*cos (start + 3.0*delta);
        midPoint.y = axis2*sin (start + 3.0*delta);

        poles[3].x = (midPoint.x * c - (poles[2].x + poles[4].x))/wght;
        poles[3].y = (midPoint.y * c - (poles[2].y + poles[4].y))/wght;

        midPoint.x = axis1*cos (start + 5.0*delta);
        midPoint.y = axis2*sin (start + 5.0*delta);

        poles[5].x = (midPoint.x * c - (poles[4].x + poles[6].x))/wght;
        poles[5].y = (midPoint.y * c - (poles[4].y + poles[6].y))/wght;

        /* Knot vector to force Bezier curve segments */
        if (bClosed)
            {
            knots[0] = 0.0;
            knots[1] = knots[2] = 1.0/3.0;
            knots[3] = knots[4] = 2.0/3.0;
            knots[5] = 1.0;
            }
        else
            {
            knots[0] = knots[1] = 1.0/3.0;
            knots[2] = knots[3] = 2.0/3.0;
            }
        }
    else if (numSections == 2)
        {
        delta = sweep/4.0;
        poles[4] = poles[2];

        poles[2].x = axis1*cos (start + 2.0*delta);
        poles[2].y = axis2*sin (start + 2.0*delta);

        midPoint.x = axis1*cos (start + delta);
        midPoint.y = axis2*sin (start + delta);

        poles[1].x = (midPoint.x * c - (poles[0].x + poles[2].x))/wght;
        poles[1].y = (midPoint.y * c - (poles[0].y + poles[2].y))/wght;

        midPoint.x = axis1*cos (start + 3.0*delta);
        midPoint.y = axis2*sin (start + 3.0*delta);

        poles[3].x = (midPoint.x * c - (poles[2].x + poles[4].x))/wght;
        poles[3].y = (midPoint.y * c - (poles[2].y + poles[4].y))/wght;

        /* Knot vector to force Bezier curve segments */
        knots[0] = knots[1] = 0.5;
        }
    else
        {
        delta = sweep/2.0;

        midPoint.x = axis1*cos (start + delta);
        midPoint.y = axis2*sin (start + delta);

        poles[1].x = (midPoint.x * c - (poles[0].x + poles[2].x))/wght;
        poles[1].y = (midPoint.y * c - (poles[0].y + poles[2].y))/wght;
        }
#define MAX_ARCKNOTS 6
    params->order = 3;
    params->closed = bClosed;
    params->numPoles = 2 * numSections + 1;
    params->numKnots = params->closed ? MAX_ARCKNOTS : params->numPoles - 3;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef msbspline_constructors
MSBsplineCurve::MSBsplineCurve ()
    {
    memset (this, 0, sizeof (MSBsplineCurve));
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineCurve::ReleaseMem ()
    {
    MSBsplineCurve * curve = this;

    if (sFreeCurve)
        {
        sFreeCurve (curve);
        return;
        }

    if (NULL != curve->poles)
        BSIBaseGeom::Free (poles);
    if (NULL != curve->knots)
        BSIBaseGeom::Free (knots);
    if (NULL != curve->weights)
        BSIBaseGeom::Free (weights);
    curve->poles   = NULL;
    curve->weights = NULL;
    curve->knots   = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineCurve::Zero ()
    {
    memset (this, 0, sizeof (MSBsplineCurve));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineCurve::SwapContents (MSBsplineCurveR other)
    {
    std::swap (*this, other);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::Allocate ()
    {
    MSBsplineCurve * curve = this;

    if (sAllocateCurve)
        return sAllocateCurve (curve);

    int allocSize;

    /* Initialize all pointers to NULL */
    curve->poles = NULL;    curve->knots = curve->weights = NULL;
    curve->display.curveDisplay   = true;
    curve->display.polygonDisplay = false;

    if (curve->params.numPoles <= 0)
        return MDLERR_NOPOLES;

    /* Allocate poles memory */
    allocSize = curve->params.numPoles * sizeof (DPoint3d);
    if (NULL == (curve->poles = static_cast<DPoint3d *>(BSIBaseGeom::Malloc (allocSize))))
        return MDLERR_INSFMEMORY;

    /* Allocate knot vector memory */
    allocSize = params.NumberAllocatedKnots () * sizeof (double);

    if (NULL == (curve->knots = static_cast<double *>(BSIBaseGeom::Malloc (allocSize))))
        {
        ReleaseMem ();
        return MDLERR_INSFMEMORY;
        }

    /* Allocate weights (if necessary) */
    if (curve->rational)
        {
        allocSize = curve->params.numPoles * sizeof(double);
        if (NULL == (curve->weights = static_cast<double *>(BSIBaseGeom::Malloc (allocSize))))
            {
            ReleaseMem ();
            return MDLERR_INSFMEMORY;
            }
        }

    return MSB_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::Allocate (int numPoles, int order, bool closed, bool rational_)
    {
    Zero ();
    display.curveDisplay   = true;
    display.polygonDisplay = false;
    params.numPoles = numPoles;
    params.order    = order;
    params.closed   = closed;
    rational = rational_ ? 1 : 0;
    return Allocate ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::AllocatePoles (int count, DPoint3dCP data)
    {
    int allocSize = count * sizeof (DPoint3d);
    if (NULL == (poles = static_cast<DPoint3d *>(BSIBaseGeom::Malloc (allocSize))))
        return MDLERR_INSFMEMORY;
    if (NULL != data)
        memcpy (poles, data, allocSize);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::AllocateWeights (int count, double const *data)
    {
    int allocSize = count * sizeof (double);
    if (NULL == (weights = static_cast<double *>(BSIBaseGeom::Malloc (allocSize))))
        return MDLERR_INSFMEMORY;
    if (NULL != data)
        memcpy (weights, data, allocSize);
    rational = true;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::AllocateKnots (int count, double const * data)
    {
    int allocSize = count * sizeof (double);
    if (NULL == (knots = static_cast<double *>(BSIBaseGeom::Malloc (allocSize))))
        return MDLERR_INSFMEMORY;
    if (NULL != data)
        memcpy (knots, data, allocSize);
    return SUCCESS;
    }







/**
RowMajorBandMatrix
**/
void FindMaximalBasisFunctionKnots (bvector<double> const &knots, size_t order, bvector<double> &criticalKnots)
    {
    criticalKnots.clear ();
    double      b[MAX_BSORDER], dB[MAX_BSORDER], ddB[MAX_BSORDER], maxKnot;
    static int s_maxNewton = 6;
    static double s_tol = 1.0e-12;
    size_t degree = (size_t) (order - 1);
    int numKnots = (int)knots.size ();
    maxKnot = knots[numKnots - order];
    size_t numPoles = numKnots - order;
    size_t knot0 = 1;   // leftmost knot of first Greville average.
    for (size_t basisFunctionIndex = 0; basisFunctionIndex < numPoles; basisFunctionIndex++)
        {
        double s = 0.0;
        for (size_t k = 0; k < degree; k++)
            s += knots[knot0 + basisFunctionIndex + k];
        double knotValue = s / (double)degree;
        int left0;
        bsputil_knotToBlendingFuncs (b, dB, &left0, &knots[0], knotValue, maxKnot, (int)order, 0);
        size_t iMax = 0;
        for (size_t i = 0; i < order; i++)
            if (b[i] > b[iMax])
                iMax = i;
        if (basisFunctionIndex > 0 && basisFunctionIndex + 1 < numPoles)
            {
            double x = knotValue;
            // run some newton steps looking for max ...
            for (int newtonStep = 0; newtonStep < s_maxNewton; newtonStep++)
                {
                int left;
                bsputil_blendingsForSecondPars (b, dB, ddB, &left, &knots[0], x, maxKnot, (int)order, 0);
                int kk = (int)iMax + left0 - left;
                double dx;
                DoubleOps::SafeDivideParameter(dx, dB[kk], ddB[kk], 0.0);
                x -= dx;
                if (fabs (dx) < s_tol)
                    break;
                }
            knotValue = x;
            }
        criticalKnots.push_back (knotValue);
        }
    }

//static size_t s_dump = 0;
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus InterpolationByPointsParamsAndKnots
(
MSBsplineCurveP         pOut,
bvector<DPoint3d>   &P,     // overwritten !!!
bvector<double>     const &u,
bvector<double>     const &knots,
int                     order
);


GEOMDLLIMPEXP MSBsplineCurvePtr MSBsplineCurve::CreateFromInterpolationPointsWithKnots
(
bvector<DPoint3d>const &xyz,                //!< [in] xyz interpolation points
bvector<double> const &interpolationKnots,  //!< [in] knot values where interpolation occurs
bvector<double> curveKnots,                  //!< [in] knots for the curve
int order                                   //! curve order
)
    {
    MSBsplineCurve bcurve;
    bvector <DPoint3d> poles = xyz;
    if(SUCCESS == InterpolationByPointsParamsAndKnots(&bcurve, poles, interpolationKnots, curveKnots, (int)order))
        return bcurve.CreateCapture();
    return nullptr;
    }
MSBsplineCurvePtr MSBsplineCurve::CreateFromInterpolationAtBasisFunctionPeaks
(
bvector<DPoint3d>const &xyz,    //!< @param [in] xyz interpolation points
size_t order,                   //!< @param [in] order bspline order.
int selector                    //!< @param [in] knotType (0==>uniform)  (Others to be added)
)
    {
    size_t numPoints = xyz.size ();
    if (numPoints < order)
        return nullptr;
    if (order < 2)
        return nullptr;

    bvector<double> knots;
    size_t numKnotInternal = numPoints - order;
    double df = 1.0 / (double)(numKnotInternal + 1);
    for (size_t i = 0; i < order; i++)
        knots.push_back (0.0);
    for (size_t i = 1; i <= numKnotInternal; i++)
        knots.push_back (i * df);
    for (size_t i = 0; i < order; i++)
        knots.push_back (1.0);
    bvector<DPoint3d> poles;
    bvector<double> criticalKnots;


    if (selector == 1)
        DoubleOps::MovingAverages (criticalKnots, knots, order - 1, 1, 1);
    else
        FindMaximalBasisFunctionKnots (knots, order, criticalKnots);
    MSBsplineCurve curve;
    poles = xyz;
   if (SUCCESS == InterpolationByPointsParamsAndKnots (&curve, poles, criticalKnots, knots, (int)order))
        {
        return curve.CreateCapture ();
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::CreateFromPointsAndOrder (DPoint3dCP pPoints, int numPoints, int order, bool closed)
    {
    MSBsplineStatus status = MSB_ERROR;

    if (pPoints != NULL && numPoints >= order)
        {
        Zero ();
        params.order = order;
        params.closed = closed ? 1 : 0;
        params.numPoles = numPoints;
        display.curveDisplay   = true;
        display.polygonDisplay = false;

        if (SUCCESS == (status = Allocate ()))
            {
            memcpy (poles, pPoints, numPoints*sizeof(DPoint3d));
            status = bspknot_computeKnotVector (knots, &params, NULL);
            }
        }
    return  status;
    }
template <typename T>
T const *DataPointer (bvector<T> const *vectorPointer)
    {
    return vectorPointer != NULL && vectorPointer->size () > 0 ? &vectorPointer->at(0) : NULL;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::Populate
(
bvector<DPoint3d> const &pointVector,
bvector<double> const *weightVector,
bvector<double> const *knotVector,
int order,
bool closed,
bool inputPointsHaveWeightsAppliedAlready
)
    {
    int numPoints = (int)pointVector.size ();
    int numWeights = weightVector != NULL ? (int)weightVector->size () : 0;
    int numKnots = knotVector != NULL ? (int)knotVector->size () : 0;
    if (numPoints < 2 || (numWeights > 0 && numWeights != numPoints))
        return MSB_ERROR;
    return Populate (
            &pointVector[0],
            DataPointer<double> (weightVector),
            numPoints,
            DataPointer<double> (knotVector),
            numKnots,
            order, closed, inputPointsHaveWeightsAppliedAlready
            );
    }

int bspcurv_segmentCurve2(MSBsplineCurveP, MSBsplineCurveCP, double, double, bool);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::Populate
(
DPoint3dCP pPoints,
double const * pWeights,
int numPoints,
double const * pKnots,
int numKnots,
int order,
bool closed,
bool inputPointsHaveWeightsAppliedAlready
)
    {
    MSBsplineStatus status = MSB_ERROR;

    if (order > 1
        && numPoints >= order
        && (numKnots == 0 || numKnots == BsplineParam::NumberAllocatedKnots(numPoints, order, closed))
        )
        {
        Zero ();
        params.order = order;
        params.numPoles = (int)numPoints;
        params.closed = closed;
        display.curveDisplay   = true;
        display.polygonDisplay = false;
        rational = pWeights != NULL;

        if (SUCCESS == (status = Allocate ()))
            {
            for (int i = 0; i < numPoints; i++)
                poles[i] = pPoints[i];
            if (weights != NULL)
                for (int i = 0; i < numPoints; i++)
                    {
                    weights[i] = pWeights? pWeights[i]: 1.0;
                    if (!inputPointsHaveWeightsAppliedAlready)
                        poles[i].Scale (weights[i]);
                    }

            if (numKnots == 0)
                status = bspknot_computeKnotVector (knots, &params, NULL);
            else
                {
                params.numKnots = BsplineParam::NumberInteriorKnots (numPoints, order, closed);
                for (int i = 0; i < numKnots; i++)
                    knots[i] = pKnots[i];

                // Bug #816037: ensure clamped knots for open curve so it draws correctly
                if (!closed && !mdlBspline_curveHasClampedKnots(this))
                    status = bspcurv_segmentCurve2(this, this, 0.0, 1.0, true);
                }
            }
        }
    return  status;
    }

MSBsplineCurvePtr MSBsplineCurve::CreateCapture ()
    {
    MSBsplineCurvePtr result = MSBsplineCurve::CreatePtr ();
    if (!result.IsValid ())
        return result;
    result->SwapContents (*this);
    return result;
    }

MSBsplineSurfacePtr MSBsplineSurface::CreateCapture ()
    {
    MSBsplineSurfacePtr result = MSBsplineSurface::CreatePtr ();
    if (!result.IsValid ())
        return result;
    this->ExtractTo (*result);
    return result;
    }


MSBsplineCurvePtr MSBsplineCurve::CreateCopy () const
    {
    MSBsplineCurvePtr result = MSBsplineCurve::CreatePtr ();
    if (!result.IsValid ())
        return result;
    bspcurv_copyCurve (result.get(), const_cast <MSBsplineCurveP>(this));
    return result;
    }

MSBsplineCurvePtr MSBsplineCurve::CreateCopyOpenAtFraction (double fraction) const
    {
    MSBsplineCurvePtr result = MSBsplineCurve::CreatePtr ();
    if (!result.IsValid ())
        return result;
    bspcurv_openCurve (result.get(), const_cast <MSBsplineCurveP>(this), fraction);
    return result;
    }

MSBsplineCurvePtr MSBsplineCurve::CreateCopyOpenAtKnot (double knot) const
    {
    MSBsplineCurvePtr result = MSBsplineCurve::CreatePtr ();
    if (!result.IsValid ())
        return result;
    bspcurv_openCurve (result.get(), const_cast <MSBsplineCurveP>(this), this->KnotToFraction (knot));
    return result;
    }

MSBsplineCurvePtr MSBsplineCurve::CreateCopyClosed () const
    {
    MSBsplineCurvePtr result = MSBsplineCurve::CreatePtr ();
    if (!result.IsValid ())
        return result;
    bspcurv_closeCurve (result.get(), const_cast <MSBsplineCurveP>(this));
    return result;
    }

MSBsplineCurvePtr MSBsplineCurve::CreateCopyBetweenFractions (double fraction0, double fraction1) const
    {
    MSBsplineCurvePtr result = MSBsplineCurve::CreatePtr ();
    if (!result.IsValid ())
        return result;
    bspcurv_segmentCurve (result.get(), const_cast <MSBsplineCurveP>(this), fraction0, fraction1);
    return result;
    }

MSBsplineCurvePtr MSBsplineCurve::CreateCopyBetweenKnots (double knot0, double knot1) const
    {
    MSBsplineCurvePtr result = MSBsplineCurve::CreatePtr ();
    if (!result.IsValid ())
        return result;
    bspcurv_segmentCurve (result.get(), const_cast <MSBsplineCurveP>(this), this->KnotToFraction (knot0), this->KnotToFraction (knot1));
    return result;
    }

MSBsplineCurvePtr MSBsplineCurve::CreateCopyReversed () const
    {
    MSBsplineCurvePtr result = MSBsplineCurve::CreatePtr ();
    if (!result.IsValid ())
        return result;
    bspcurv_reverseCurve (const_cast <MSBsplineCurveP>(this), result.get());
    return result;
    }

MSBsplineCurvePtr MSBsplineCurve::CreateCopyTransformed (TransformCR transform) const
    {
    MSBsplineCurvePtr result = MSBsplineCurve::CreatePtr ();
    if (!result.IsValid ())
        return result;
    bspcurv_transformCurve (result.get (), this, &transform);
    return result;
    }

bool MSBsplineCurve::AreCompatible
(
MSBsplineCurveCR curveA,
MSBsplineCurveCR curveB
)
    {
    if (curveA.params.order != curveB.params.order)
        return false;
    if (curveA.params.numPoles != curveB.params.numPoles)
        return false;
    if (curveA.params.numKnots != curveB.params.numKnots)
        return false;

    for (int i = 0; i < curveA.params.numKnots; i++)
        {
        if (!AreSameKnots (curveA.knots[i], curveB.knots[i]))
            return false;
        }

    return true;
    }

MSBsplineCurvePtr MSBsplineCurve::CreateInterpolationBetweenCurves
(
MSBsplineCurveCR curveA,
double fraction,
MSBsplineCurveCR curveB
)
    {
    MSBsplineCurvePtr result;
    if (!AreCompatible (curveA, curveB))
        return result;
    result = curveA.CreateCopy ();
    if (!result.IsValid ())
        return result;
    DPoint3d *poles = result->poles;
    for (int i = 0; i < curveA.params.numPoles; i++)
        {
        poles[i].Interpolate (curveA.poles[i], fraction, curveB.poles[i]);
        }
    return result;
    }



MSBsplineCurvePtr MSBsplineCurve::CreateCopyBezier () const
    {
    MSBsplineCurvePtr result = MSBsplineCurve::CreatePtr ();
    bspcurv_makeBezier (result.get (), this);
    return result;
    }



MSBsplineCurvePtr MSBsplineCurve::CreateFromPolesAndOrder
(
bvector<DPoint3d> const &poles,
bvector<double> const *weights,
bvector<double> const *knots,
int order,
bool closed,
bool inputPolesAlreadyWeighted
)
    {
    // Indent to force destructor of result after failure.
        {
        MSBsplineCurvePtr result = MSBsplineCurve::CreatePtr ();
        if (SUCCESS == result.get ()->Populate (poles, weights, knots, order, closed, inputPolesAlreadyWeighted))
            return result;
        }
    MSBsplineCurvePtr nullPtr;
    return nullPtr;
    }

MSBsplineCurvePtr MSBsplineCurve::CreateFromPolesAndOrder
(
DPoint3dCP poles,
int numPoles,
int order,
bool closed
)
    {
    // Indent to force destructor of result after failure.
        {
        MSBsplineCurvePtr result = MSBsplineCurve::CreatePtr ();
        if (SUCCESS == result.get ()->Populate (poles, NULL, numPoles, NULL, 0, order, closed, true))
            return result;
        }
    MSBsplineCurvePtr nullPtr;
    return nullPtr;
    }

MSBsplineCurvePtr MSBsplineCurve::CreateFromPolesAndOrder
(
DPoint2dCP xyPoles,
int numPoles,
int order,
bool closed
)
    {
    // Indent to force destructor of result after failure.
        {
        bvector <DPoint3d> xyzPoles;
        for (size_t i = 0; i < (size_t)numPoles; i++)
            xyzPoles.push_back (DPoint3d::From (xyPoles[i].x, xyPoles[i].y, 0.0));
        MSBsplineCurvePtr result = MSBsplineCurve::CreatePtr ();
        if (SUCCESS == result.get ()->Populate (&xyzPoles[0], NULL, numPoles, NULL, 0, order, closed, true))
            return result;
        }
    MSBsplineCurvePtr nullPtr;
    return nullPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::CopyFrom (MSBsplineCurveCR input)
    {
    return bspcurv_copyCurve (this, (MSBsplineCurveCP)&input);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::CopyClosed (MSBsplineCurveCR input)
    {
    return bspcurv_closeCurve (this, const_cast<MSBsplineCurveP>(&input));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::CopyOpen (MSBsplineCurveCR input, double unnormalizedKnot)
    {
    return bspcurv_openCurve (this, (MSBsplineCurveCP)&input, input.KnotToFraction (unnormalizedKnot));
    }

MSBsplineStatus MSBsplineCurve::CopySegment (MSBsplineCurveCR input, double unnormalizedKnotA, double unnormalizedKnotB)
    {
    return bspcurv_segmentCurve (this, (MSBsplineCurveCP)&input,
                            input.KnotToFraction (unnormalizedKnotA), input.KnotToFraction (unnormalizedKnotB));
    }

MSBsplineStatus MSBsplineCurve::CopyFractionSegment (MSBsplineCurveCR input, double fractionA, double fractionB)
    {
    return bspcurv_segmentCurve (this, (MSBsplineCurveCP)&input, fractionA, fractionB);
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::CopyReversed (MSBsplineCurveCR input)
    {
    return bspcurv_reverseCurve (const_cast<MSBsplineCurveP>(&input), this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::CopyTransformed (MSBsplineCurveCR input, TransformCR trans)
    {
    return bspcurv_transformCurve (this, (MSBsplineCurveCP) &input, &trans);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::InitEllipticArc
(
DPoint3d &center,
double rX,
double rY,
double startRadians,
double sweepRadians,
RotMatrixP axes
)
    {
    int curveType = BSCURVE_CIRCULAR_ARC, numPoles;
    DPoint3d tmpPoles[20];
    double   tmpKnots[20];
    double tmpWeights[20];
    Transform transform;
    curveFromArc (tmpPoles, tmpKnots, tmpWeights, &params, startRadians, sweepRadians, rX, rY);
    rational = true;
    numPoles = params.numPoles;
    transform.InitFrom (axes != nullptr ? *axes : RotMatrix::FromIdentity (), center);

    Allocate ();

    // Full ellipse?
    if (Angle::IsFullCircle (sweepRadians))
        curveType++;
    // Circular?
    if (0 == DoubleOps::TolerancedComparison (rX, rY))
        curveType += 2;

    display.curveDisplay = true;
    display.polygonDisplay = false;
    transform.Multiply (poles, tmpPoles, numPoles);
    memcpy (weights, tmpWeights, numPoles * sizeof (double));
    bspknot_computeKnotVector (knots, &params, tmpKnots);
    DPoint3d::MultiplyArrayByScales (poles, poles, weights, params.numPoles);
    return MSB_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineCurvePtr MSBsplineCurve::CreateFromDConic4d (DConic4dCR conic)
    {
    DPoint3d    center;
    RotMatrix   rotMatrix;
    center.Zero ();
    rotMatrix.InitIdentity ();
    MSBsplineCurve result;
    result.Zero ();

    /* Any conic section is a unit circle mangled by a homogeneous transformation.  The center, vector0,
       and vector90 data in the DEllipse4d gives the transformation.  So we build a Bspline for
       the arc of the unit circle . . . */
    if (SUCCESS != bspconv_arcToCurveStruct (&result, conic.start, conic.sweep, 1.0, 1.0, &rotMatrix, &center))
        return nullptr;

    // Apply the transformation to carry the unit circle out to real space . . .
    for (int i=0; i < result.params.numPoles; i++)
        {
        DPoint4d    hPoint;
        DPoint3d xyz = result.poles[i];    // that's the circle point.
        // All the poles have z=0 !!!
        hPoint.SumOf(conic.vector0, xyz.x, conic.vector90, xyz.y, conic.center, result.weights[i]);
        result.poles[i]   = DPoint3d::From (hPoint.x, hPoint.y, hPoint.z);
        result.weights[i] = hPoint.w;
        }

    double      refWeight;
    DRange1d weightRange = result.GetWeightRange ();

    static double zeroWeightTol = 1.0e-8;

    /* If the given angular range does NOT include an asymptote, the weights will all be
       the same sign.   Verify that this is the case, and set refWeight to the extremal weight  . .  */
    if (weightRange.high > 0.0 && weightRange.low > zeroWeightTol * weightRange.high)
        refWeight = weightRange.high;
    else if (weightRange.low < 0.0 && weightRange.high < zeroWeightTol * weightRange.low)
        refWeight = weightRange.low;
    else
        refWeight = 0.0;

    if (0.0 == refWeight)
        {
        result.ReleaseMem ();
        return nullptr;
        }

    // Dividing through by the extreme weight moves all the weights between 0 and 1
    double  scale = 1.0 / refWeight;
    for (int i=0; i < result.params.numPoles; i++)
        {
        result.poles[i].Scale (scale);
        result.weights[i] *= scale;
        }

    return result.CreateCapture ();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::InitFromDEllipse3d (DEllipse3dCR ellipse)
    {
    // NOTE: The (non-orthogonal!!!) matrix transforms the unit circle to the ellipse.
    DPoint3d    zeroVec;
    RotMatrix   rMatrix;

    zeroVec.Zero ();
    rMatrix.InitIdentity ();

    Zero ();

    if (SUCCESS != InitEllipticArc (zeroVec, 1.0, 1.0, ellipse.start, ellipse.sweep, &rMatrix))
        return MSB_ERROR;

    for (int i=0; i < params.numPoles; i++)
        {
        double wc = weights[i];
        poles[i].x /= wc;
        poles[i].y /= wc;
        poles[i].z /= wc;
        double w = 1.0;

        poles[i].SumOf (ellipse.center, w, ellipse.vector0, w * poles[i].x, ellipse.vector90, w * poles[i].y);
        poles[i].x *= wc;
        poles[i].y *= wc;
        poles[i].z *= wc;
        }

    // increment type to BSCURVE_ELLIPSE (or BSCURVE_ELLIPTICAL_ARC) if noncircular
    if (ellipse.vector0.IsPerpendicularTo (ellipse.vector90)
        && DoubleOps::AlmostEqual (ellipse.vector0.Magnitude (), ellipse.vector90.Magnitude ())
        )
//    if (!ellipse.IsCircular ())
        type += 2;

    return MSB_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::InitFromPoints (DPoint3dCP points, int nPoints)
    {
    Zero ();

    type = BSCURVE_LINE;
    rational = false;

    display.curveDisplay   = true;
    display.polygonDisplay = false;

    params.order    = 2;
    params.closed   = false;
    params.numPoles = nPoints;
    params.numKnots = 0;

    int     pointAllocSize = nPoints * sizeof (DPoint3d);
    int     knotsAllocSize = bspknot_numberKnots (nPoints, 2, false) * sizeof (double);

    poles = static_cast <DPoint3d *> (BSIBaseGeom::Malloc (pointAllocSize));
    knots = static_cast <double *> (BSIBaseGeom::Malloc (knotsAllocSize));

    memcpy (poles, points, pointAllocSize);

    if (SUCCESS != bspknot_computeKnotVector (knots, &params, NULL))
        return ERROR;

    return MSB_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineCurvePtr MSBsplineCurve::CreateFromBeziers (bvector<MSBsplineCurvePtr> const &beziers)
    {
    int         i, j;
    size_t      numCurve = beziers.size ();
    if (numCurve < 1)           //Defect #7321
        return nullptr;

    double      *ratio = (double*)BSIBaseGeom::Malloc (numCurve * sizeof(double));
    double      length1, length2, sum;
    DPoint3d    point1, point2;
    DVec3d      tangent1, tangent2;

    ratio[0] = sum = 1.0;
    for (size_t k=0; k<numCurve-1; k++)
        {
        if (!mdlBspline_curveHasNormalizedKnots (beziers[k].get ()))
            bspknot_normalizeKnotVector (beziers[k]->knots, beziers[k]->params.numPoles, beziers[k]->params.order, beziers[k]->params.closed);

        if (!mdlBspline_curveHasNormalizedKnots (beziers[k+1].get ()))
            bspknot_normalizeKnotVector (beziers[k+1]->knots, beziers[k+1]->params.numPoles, beziers[k+1]->params.order, beziers[k+1]->params.closed);

        beziers[k]->FractionToPoint (point1, tangent1, 1.0);
        beziers[k+1]->FractionToPoint (point2, tangent2, 0.0);

        length1 = tangent1.Normalize();
        length2 = tangent2.Normalize();
        if (tangent1.DotProduct (tangent2)+mgds_fc_epsilon > 1.0 && length1 > mgds_fc_epsilon && length2 > mgds_fc_epsilon)
            ratio[k+1] = ratio[k]*length2/length1;
        else
            ratio[k+1] = ratio[k];
        sum += ratio[k+1];
        }

    sum = 1.0/sum;
    int     old, *increaseP = (int*)BSIBaseGeom::Malloc (numCurve * sizeof (int));

    increaseP[0] = increaseP[numCurve-1] = 0;

    MSBsplineCurve result;
    result.Zero ();
    result.CopyFrom (*beziers[0]);
    for (size_t k=1; k<numCurve; k++)
        {
        old = result.params.numPoles;
        result.AppendCurves (result, *beziers[k], false, false);
        if (result.params.order <= result.params.numPoles - old)
            increaseP[k] = 1;
        else
            increaseP[k] = 0;
        }

    int     k = 1, p = result.params.order - 1;
    int     numKnots = bspknot_numberKnots (result.params.numPoles, p + 1, result.params.closed);

    for (i=0; i<=p; i++)
        result.knots[i] = 0.0;
    for (i=p+1; i<numKnots-p-1; i+=(p+increaseP[k-1]))
        {
        for (j=0; j<p+increaseP[k]; j++)
            result.knots[i+j] = result.knots[i-1] + ratio[k-1]*sum;
        k++;
        }
    for (i=numKnots-1; i>=numKnots-p-1; i--)
        result.knots[i] = 1.0;

    BSIBaseGeom::Free (increaseP);
    BSIBaseGeom::Free (ratio);

    if (SUCCESS != result.RemoveKnotsBounded (1.0E-3, true, true))
        {
        result.ReleaseMem ();
        return nullptr;
        }
    return result.CreateCapture ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::InitFromDPoint4dArray (DPoint4dCP pPoleArray, int numPoles, int order)
    {
    Zero ();

    rational = !bsiBezierDPoint4d_isUnitWeight (pPoleArray, numPoles, -1.0);
    display.curveDisplay = true;
    params.order = order;
    params.numPoles = numPoles;
    params.numKnots = 0;

    if (SUCCESS != Allocate ())
        return MSB_ERROR;

    bspknot_computeKnotVector (knots, &params, NULL);

    for (int i = 0; i < numPoles; i++)
        {
        poles[i].x = pPoleArray[i].x;
        poles[i].y = pPoleArray[i].y;
        poles[i].z = pPoleArray[i].z;

        if (rational)
            weights[i] = pPoleArray[i].w;
        }

    return SUCCESS;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
