/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/bspconv.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/BspPrivateApi.h>
#include "msbsplinemaster.h"
#include <Geom/MstnOnly/GeomPrivateApi.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#define BOUNDARY_JUMP_TOLERANCE 0.90
#define TOLERANCE_BoundaryCornerAngle           .5

/*----------------------------------------------------------------------+
|                                                                       |
|    Curve Conversion Routines                                          |
|                                                                       |
+----------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             03/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspconv_convertLstringToCurve
(
int             *type,                  /* <= curve type */
int             *rational,              /* <= rational (weights included) */
BsplineDisplay  *display,               /* <= display parameters */
BsplineParam    *params,                /* <= number of poles etc. */
DPoint3d        **poles,                /* <= pole coordinates */
double          **knots,                /* <= knot vector */
double          **weights,              /* <= weights (if (Rational) */
DPoint3d        *pointP,                /* => points */
int             numPoints               /* => number of points */
)
    {
    int             status, allocSize;
    BsplineParam    tmpParams;

    if (type)
        *type = BSCURVE_LINE;

    if (rational)
        *rational = false;

    if (display)
        {
        display->curveDisplay = true;
        display->polygonDisplay = false;
        }

    tmpParams.order    = 2;
    tmpParams.closed   = false;
    tmpParams.numPoles = numPoints;
    tmpParams.numKnots = 0;

    if (params)
        *params = tmpParams;

    if (poles)
        {
        allocSize = numPoints*sizeof(DPoint3d);
        if (NULL == (*poles = (DPoint3d*)BSIBaseGeom::Malloc (allocSize)))
            return ERROR;
        else
            memcpy (*poles, pointP, allocSize);
        }

    if (knots)
        {
        allocSize = bspknot_numberKnots (numPoints, 2, false) * sizeof (double);
        if (NULL == (*knots = (double*)BSIBaseGeom::Malloc (allocSize)))
            return ERROR;
        else
            if (SUCCESS != (status = bspknot_computeKnotVector (*knots, &tmpParams, NULL)))

                return status;
        }

    if (weights)
        *weights = NULL;


    return (SUCCESS);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             03/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspconv_computeCurveFromArc
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

    bool completeCircle = Angle::IsFullCircle (sweep);

    /* Test arc angle : if greater than 120 degrees use multiple segments */
    numSections = (fabs(sweep) <= msGeomConst_2pi /fc_3 ? 1:
                  (fabs(sweep) <= 2.0* msGeomConst_2pi /fc_3 ? 2:
                   3));

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

        poles[4].x = axis1*cos (start + fc_4*delta);
        poles[4].y = axis2*sin (start + fc_4*delta);

        midPoint.x = axis1*cos (start + fc_3*delta);
        midPoint.y = axis2*sin (start + fc_3*delta);

        poles[3].x = (midPoint.x * c - (poles[2].x + poles[4].x))/wght;
        poles[3].y = (midPoint.y * c - (poles[2].y + poles[4].y))/wght;

        midPoint.x = axis1*cos (start + fc_5*delta);
        midPoint.y = axis2*sin (start + fc_5*delta);

        poles[5].x = (midPoint.x * c - (poles[4].x + poles[6].x))/wght;
        poles[5].y = (midPoint.y * c - (poles[4].y + poles[6].y))/wght;

        /* Knot vector to force Bezier curve segments */
        if (completeCircle)
            {
            knots[0] = 0.0;
            knots[1] = knots[2] = 1.0/fc_3;
            knots[3] = knots[4] = 2.0/fc_3;
            knots[5] = 1.0;
            }
        else
            {
            knots[0] = knots[1] = 1.0/fc_3;
            knots[2] = knots[3] = 2.0/fc_3;
            }
        }
    else if (numSections == 2)
        {
        delta = sweep/fc_4;
        poles[4] = poles[2];

        poles[2].x = axis1*cos (start + 2.0*delta);
        poles[2].y = axis2*sin (start + 2.0*delta);

        midPoint.x = axis1*cos (start + delta);
        midPoint.y = axis2*sin (start + delta);

        poles[1].x = (midPoint.x * c - (poles[0].x + poles[2].x))/wght;
        poles[1].y = (midPoint.y * c - (poles[0].y + poles[2].y))/wght;

        midPoint.x = axis1*cos (start + fc_3*delta);
        midPoint.y = axis2*sin (start + fc_3*delta);

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

    params->order = 3;
    params->closed = completeCircle ? 1 : 0;
    params->numPoles = 2 * numSections + 1;
    params->numKnots = params->closed ? MAX_ARCKNOTS : params->numPoles - 3;

    return (SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bspconv_convertArcToCurve
(
int             *type,                  /* <= curve type */
int             *rational,              /* <= rational (weights included) */
BsplineDisplay  *display,               /* <= display parameters */
BsplineParam    *params,                /* <= number of poles etc. */
DPoint3d        **poles,                /* <= pole coordinates */
double          **knots,                /* <= knot vector */
double          **weights,              /* <= weights (if (Rational) */
double          start,
double          sweep,
double          axis1,
double          axis2,
RotMatrix       *rotMatrixP,
DPoint3d        *centerP
)
    {
    int             status, curveType = BSCURVE_CIRCULAR_ARC, numPoles, allocSize;
    double          tWeights[MAX_ARCPOLES], tKnots[MAX_ARCKNOTS];
    DPoint3d        tPoles[MAX_ARCPOLES];
    BsplineParam    tParams;
    Transform       tMatrix;

    tMatrix.InitIdentity ();
    tMatrix.TranslateInLocalCoordinates(tMatrix, centerP->x, centerP->y, centerP->z);
    tMatrix.InitProduct (tMatrix, *rotMatrixP);

    bspconv_computeCurveFromArc (tPoles, tKnots, tWeights, &tParams,
                                 start, sweep, axis1, axis2);
    numPoles = tParams.numPoles;
    // typecode sequence (partial circle, full circle, partial ellipse, full ellipse)
    if (tParams.closed)
        curveType++;
    if (!DoubleOps::AlmostEqual (axis1, axis2))
        curveType += 2;

    if (type)       *type = curveType;
    if (rational)   *rational = true;
    if (display)
        {
        display->curveDisplay = true;
        display->polygonDisplay = false;
        }

    if (params)
        *params = tParams;

    if (poles)
        {
        allocSize = numPoles * sizeof(DPoint3d);
        if (NULL == (*poles = (DPoint3d*)BSIBaseGeom::Malloc (allocSize)))
            return ERROR;
        else
            memcpy (*poles, tPoles, allocSize);

        tMatrix.Multiply (*poles, *poles, numPoles);
        }

    if (knots)
        {
        allocSize = bspknot_numberKnots (tParams.numPoles, tParams.order, tParams.closed)
                    * sizeof(double);
        if (NULL == (*knots = (double*)BSIBaseGeom::Malloc (allocSize)))
            return ERROR;
        else
            if (SUCCESS != (status = bspknot_computeKnotVector (*knots, &tParams, tKnots)))
                return status;
        }

    if (weights)
        {
        allocSize = numPoles * sizeof(double);
        if (NULL == (*weights = (double*)BSIBaseGeom::Malloc (allocSize)))
            return ERROR;
        else
            memcpy (*weights, tWeights, allocSize);
        }

    return (SUCCESS);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EDL             08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt   bspconv_convertDEllipse3dToCurve
(
MSBsplineCurve  *pCurve,                /* <= curve structure to be initialized with newly allocated poles, knots, and weights. */
const DEllipse3d    *pEllipse           /* => ellipse */
)
    {
    return pCurve->InitFromDEllipse3d ((DEllipse3dR) *pEllipse);
    }

static ICurvePrimitivePtr bspconv_appendOffsetEllipseSegmentToCurveChain
(
DEllipse3dCP pEllipse,          /* => ellipse */
double       f0,
double       f1,
double       offsetDistance,     /* => SIGNED offset. Positive is outward. */
double       maxStep,
bool        bClosed
)
    {
#define MAX_OFFSET_POINT 257
    double theta0 = pEllipse->FractionToAngle (f0);
    double theta1 = pEllipse->FractionToAngle (f1);
    //double adtheta = fabs (theta1 - theta0);
    static double sMaxStep = 0.5;
    static double sMinStep = 0.01;
    DPoint3d xyz[MAX_OFFSET_POINT];
    int numPoint = 4;
    double absSweep, dtheta;
    DVec3d normal, UxM, VxM;
    int i;

    normal.NormalizedCrossProduct (pEllipse->vector0, pEllipse->vector90);
    UxM.CrossProduct (pEllipse->vector0, normal);
    VxM.CrossProduct (pEllipse->vector90, normal);
    if (maxStep > sMaxStep)
        maxStep = sMaxStep;
    if (maxStep < sMinStep)
        maxStep = sMinStep;
    absSweep = fabs (theta1 - theta0);
    if (absSweep > (numPoint - 1) * maxStep)
        {
        numPoint = 1 + (int)floor ((absSweep + 0.999 * maxStep) / maxStep);
        }
    if (numPoint > MAX_OFFSET_POINT)
        numPoint = MAX_OFFSET_POINT;
    dtheta = (theta1 - theta0) / (double)(numPoint - 1);
    for (i = 0; i < numPoint; i++)
        {
        double theta = theta0;
        double c, s;
        DPoint3d xyz0;
        DVec3d perpVector;
        if (i >= numPoint - 1)
            theta = theta1;
        else if (i > 0)
            theta += i * dtheta;
        c = cos (theta);
        s = sin (theta);
        xyz0 = DPoint3d::FromSumOf (pEllipse->center, pEllipse->vector0, c, pEllipse->vector90, s);
        perpVector = UxM * (-s) + VxM * c;
        perpVector.Normalize ();
        xyz[i].SumOf (xyz0, perpVector, offsetDistance);
        }

    MSBsplineCurvePtr curvePtr = MSBsplineCurve::CreatePtr ();
    if (SUCCESS == bspcurv_c2CubicInterpolateCurve
                    (curvePtr.get (), xyz, NULL, bClosed ? numPoint - 1 : numPoint, false, 0.0, NULL, bClosed))
        return ICurvePrimitive::CreateBsplineCurve (curvePtr);

    return NULL;
    }

/*----------------------------------------------------------------------+
@return one or more curves, joining at cusps.
@param pEllipse IN base ellipse
@param offsetDistance IN signed offset distance. Positive is outwards.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP ICurvePrimitivePtr bspconv_convertDEllipse3dOffsetToCurveChain
(
DEllipse3dCP pEllipse,          /* => ellipse */
double       offsetDistance     /* => SIGNED offset. Positive is outward. */
)
    {
    DEllipse3d majorMinorEllipse;
    static double sMaxStep = 0.08; // About 5 degrees
    //StatusInt status = ERROR;
    int i;
    double cutFraction[6];
    int numCut = 2;
    double a, b;
    double thetaStar;
    bool    bClosed = false;
    majorMinorEllipse.InitWithPerpendicularAxes (*pEllipse);
    a = majorMinorEllipse.vector0.Magnitude ();
    b = majorMinorEllipse.vector90.Magnitude ();
    cutFraction[0] = 0.0;
    cutFraction[1] = 1.0;
    // Find the first quadrant angle at which a cut occurs ...
    if (offsetDistance < 0.0
        && bsiGeom_ellipseInverseRadiusOfCurvature (&thetaStar, a, b, -offsetDistance))
        {
        double cutAngle[4];
        cutAngle[0] = thetaStar;
        cutAngle[1] = -thetaStar;
        cutAngle[2] = msGeomConst_pi  - thetaStar;
        cutAngle[3] = - cutAngle[2];
        for (i = 0; i < 4; i++)
            {
            if (majorMinorEllipse.IsAngleInSweep (cutAngle[i]))
                cutFraction[numCut++] = pEllipse->AngleToFraction (cutAngle[i]);
            }
        }
    else
        {
        if (pEllipse->IsFullEllipse ())
            bClosed = true;
        }
    bsiDoubleArray_sort (cutFraction, numCut, true);
    if (numCut == 2)
        return bspconv_appendOffsetEllipseSegmentToCurveChain (pEllipse, cutFraction[0], cutFraction[1],
                            offsetDistance, sMaxStep, bClosed);
    else
        {
        CurveVectorPtr curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
        for (i = 1; i < numCut; i++)
            {
            if (DoubleOps::AlmostEqualFraction (cutFraction[i-1], cutFraction[i]))
                continue;
            ICurvePrimitivePtr prim = 
                bspconv_appendOffsetEllipseSegmentToCurveChain (pEllipse, cutFraction[i-1], cutFraction[i],
                                offsetDistance, sMaxStep, bClosed);
            if (prim.IsValid ())
                curves->push_back (prim);
            }
        if (curves->size () == 0)
            return NULL;
        if (curves->size () == 1)
            return curves->at(0);
        return ICurvePrimitive::CreateChildCurveVector (curves);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             03/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int     bspconv_arcToCurveStruct
(
MSBsplineCurve  *curveP,
double          start,
double          sweep,
double          axis1,
double          axis2,
RotMatrix       *rotMatrixP,
DPoint3d        *centerP
)
    {
    int         status;

    if (SUCCESS != (status = bspconv_convertArcToCurve (&curveP->type,
                                                        &curveP->rational,
                                                        &curveP->display,
                                                        &curveP->params,
                                                        &curveP->poles,
                                                        &curveP->knots,
                                                        &curveP->weights,
                                                        start, sweep, axis1, axis2,
                                                        rotMatrixP, centerP)))
        return status;

    bsputil_weightPoles (curveP->poles, curveP->poles, curveP->weights,
                         curveP->params.numPoles);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             03/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int     bspconv_lstringToCurveStruct
(
MSBsplineCurve  *curveP,
DPoint3d        *pointP,
int             nPoints
)
    {

    return bspconv_convertLstringToCurve (&curveP->type,
                                          &curveP->rational,
                                          &curveP->display,
                                          &curveP->params,
                                          &curveP->poles,
                                          &curveP->knots,
                                          &curveP->weights,
                                          pointP, nPoints);
    }

// SPLIT BBB
// SPLIT CCC
// SPLIT DDD
// SPLIT EEE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bspconv_coneToSurface
(
MSBsplineSurface    *surf,
double              topRad,
double              baseRad,
RotMatrix           *rotMatrixP,
DPoint3d            *topCenterP,
DPoint3d            *baseCenterP
)
    {
    int                 status;
    double              tKnots[MAX_ARCKNOTS];
    MSBsplineCurve      top, base;

    top.params.numPoles = 7;
    top.params.numKnots = 4;
    top.params.order    = 3;
    top.params.closed   = true;
    top.rational        = true;
    base = top;
    if (SUCCESS != (status = bspcurv_allocateCurve (&top)) ||
        SUCCESS != (status = bspcurv_allocateCurve (&base)))
        goto wrapup;

    bspconv_computeCurveFromArc (top.poles, tKnots, top.weights, &top.params,
                                 0.0, msGeomConst_2pi , topRad, topRad);
    if (SUCCESS != (status = bspknot_computeKnotVector (top.knots, &top.params, tKnots)))
        goto wrapup;

    bspconv_computeCurveFromArc (base.poles, tKnots, base.weights, &base.params,
                                 0.0, msGeomConst_2pi , baseRad, baseRad);
    if (SUCCESS != (status = bspknot_computeKnotVector (base.knots, &base.params, tKnots)))
        goto wrapup;

    rotMatrixP->Multiply (top.poles, top.poles, top.params.numPoles);
    rotMatrixP->Multiply (base.poles, base.poles, base.params.numPoles);
    DPoint3d::AddToArray (top.poles, top.params.numPoles, *topCenterP);
    DPoint3d::AddToArray (base.poles, base.params.numPoles, *baseCenterP);

    bsputil_weightPoles (top.poles, top.poles, top.weights, top.params.numPoles);
    bsputil_weightPoles (base.poles, base.poles, base.weights, base.params.numPoles);

    status = bspsurf_ruledSurface (surf, &top, &base);

    surf->display.curveDisplay = true;
    surf->uParams.numRules = surf->vParams.numPoles;
    surf->vParams.numRules = surf->uParams.numPoles;

wrapup:
    bspcurv_freeCurve (&top);
    bspcurv_freeCurve (&base);
    return status;
    }
// SPLIT FFF

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             08/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspconv_dropToCurves
(
MSBsplineCurve      ***curves,         /* <=> ptr to array of ptr's to */
int                 *numCurves,        /*  MSBsplineCurve structures */
MSBsplineSurface    *surface,
int                 direction
)
    {
    int                 status = ERROR, total;
    BsplineParam        params;
    MSBsplineCurve      *cvP;

    *numCurves = 0;
    if (direction == BSSURF_U)
        {
        total  = surface->vParams.numPoles;
        params = surface->uParams;
        }
    else
        {
        total  = surface->uParams.numPoles;
        params = surface->vParams;
        }

    /* Allocate the array of pointers to curves */
    if (NULL == (*curves = (MSBsplineCurve**)BSIBaseGeom::Malloc (total * sizeof(MSBsplineCurve *))))
        return ERROR;

    for (*numCurves=0; *numCurves < total; (*numCurves)++)
        {
        if (NULL == (cvP = (MSBsplineCurve*)BSIBaseGeom::Malloc (sizeof(MSBsplineCurve))))
            {
            status = ERROR;
            goto wrapup;
            }

        cvP->rational = surface->rational;
        cvP->display  = surface->display;
        cvP->params   = params;
        if (SUCCESS != (status = bspcurv_allocateCurve (cvP)))
            goto wrapup;

        bspconv_getCurveFromSurface (cvP, surface, direction, *numCurves);

        (*curves)[*numCurves] = cvP;
        }

wrapup:
    if (status)
        for ((*numCurves)--; *numCurves >=0; (*numCurves)--)
            bspcurv_freeCurve ((*curves)[*numCurves]);
    return status;
    }






/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             09/90
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineCurvePtr MSBsplineSurface::GetIsoVCurve (double v) const
    {
    bvector<DPoint3d> vPoles;
    bvector<double> vWeights;
    
    bvector<DPoint3d> newPoles;
    bvector<double> newWeights;

    size_t numUPoles = (size_t)uParams.numPoles;
    size_t numVPoles = (size_t)vParams.numPoles;
    if (numUPoles < 2 || numVPoles < 2)
        return nullptr;
    bool isRational = rational != 0;
    vPoles.reserve (numVPoles);
    if (rational)
        vWeights.clear ();
    for (size_t i=0; i < numUPoles; i++)
        {
        vPoles.clear ();
        for (size_t j=0; j < numVPoles; j++)
           vPoles.push_back (poles[i + j*numUPoles]);

        if (rational)
            {
            vWeights.clear ();
            for (size_t j=0; j < numVPoles; j++)
               vWeights.push_back (weights[i + j*numUPoles]);
            }
        DPoint3d xyz;
        double   w;
        bspcurv_computeCurvePoint (&xyz, NULL, &w,
                                   v, &vPoles[0],
                                   vParams.order,
                                   vParams.numPoles,
                                   vKnots,
                                   rational ? &vWeights[0] : NULL,
                                   rational,
                                   vParams.closed);
        if (isRational)
            xyz.Scale (w);
        newPoles.push_back (xyz);
        if (isRational)
            newWeights.push_back (w);
        }
    MSBsplineCurvePtr curve = MSBsplineCurve::CreatePtr ();
    curve->Populate (
                &newPoles[0],
                isRational ? &newWeights[0] : NULL,
                (int) numUPoles,
                uKnots,
                uParams.NumberAllocatedKnots (),
                uParams.order,
                uParams.closed ? true : false,
                true);
    return curve;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             09/90
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineCurvePtr MSBsplineSurface::GetIsoUCurve (double u) const
    {
    bvector<DPoint3d> newPoles;
    bvector<double> newWeights;

    size_t numUPoles = (size_t)uParams.numPoles;
    size_t numVPoles = (size_t)vParams.numPoles;
    if (numUPoles < 2 || numVPoles < 2)
        return nullptr;
    bool isRational = rational != 0;
    for (size_t j=0; j < numVPoles; j++)
        {
        DPoint3d xyz;
        double   w;
        bspcurv_computeCurvePoint (&xyz, NULL, &w,
                                   u, &poles[j * numUPoles],
                                   uParams.order,
                                   uParams.numPoles, uKnots,
                                   isRational ? &weights[j*numUPoles] : NULL,
                                   rational,
                                   uParams.closed);
        if (isRational)
            xyz.Scale (w);
        newPoles.push_back (xyz);
        if (isRational)
            newWeights.push_back (w);
        }
    MSBsplineCurvePtr curve = MSBsplineCurve::CreatePtr ();
    curve->Populate (
                &newPoles[0],
                isRational ? &newWeights[0] : NULL,
                (int) numVPoles,
                vKnots,
                vParams.NumberAllocatedKnots (),
                vParams.order,
                vParams.closed ? true : false,
                true);
    return curve;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineSurface::GetIsoULineVIntersections (double u, bvector<double> &vParams) const
    {
    bspsurf_intersectBoundariesWithUVLine (&vParams, u, this, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineSurface::GetIsoVLineUIntersections (double v, bvector<double> &uParams) const
    {
    bspsurf_intersectBoundariesWithUVLine (&uParams, v, this, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineSurface::GetIsoUCurveSegments (double u, bvector<MSBsplineCurvePtr> &segments) const
    {
    segments.clear ();
    MSBsplineCurvePtr curve = GetIsoUCurve (u);
    if (GetNumBounds () == 0)
        {
        segments.push_back (curve);
        }
    else
        {
        bvector<double> params;
        GetIsoULineVIntersections (u, params);
        for (size_t i = 0; i + 1 < params.size (); i += 2)
            {
            if (!MSBsplineCurve::AreSameKnots (params[i], params[i+1]))
                segments.push_back (curve->CreateCopyBetweenFractions (params[i], params[i+1]));
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineSurface::GetIsoVCurveSegments (double v, bvector<MSBsplineCurvePtr> &segments) const
    {
    segments.clear ();
    MSBsplineCurvePtr curve = GetIsoVCurve (v);
    if (GetNumBounds () == 0)
        {
        segments.push_back (curve);
        }
    else
        {
        bvector<double> params;
        GetIsoVLineUIntersections (v, params);
        for (size_t i = 0; i + 1 < params.size (); i+= 2)
            {
            if (!MSBsplineCurve::AreSameKnots (params[i], params[i+1]))
                segments.push_back (curve->CreateCopyBetweenFractions (params[i], params[i+1]));
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz 11/14
+---------------+---------------+---------------+---------------+---------------+------*/
static bool fastFitAppendAndClear
(
MSBsplineSurfaceCR surface,
CurveVectorPtr &chain,
DPoint2dCP  params,
int         i0,
int         numParams
)
    {
    static double s_smallParamChange = 1.0e-7;
    bool accept = false;
    if (numParams > 1)
        {
        int i1 = i0 + numParams - 1;
        DRange2d paramRange = DRange2d::From (params + i0, numParams);
        double dx = paramRange.high.x - paramRange.low.x;
        double dy = paramRange.high.y - paramRange.low.y;

        double x0 = params[i0].x;
        double y0 = params[i0].y;
        double x1 = params[i1].x;
        double y1 = params[i1].y;

        if (fabs (dx) < s_smallParamChange)
            {
            MSBsplineCurvePtr curveA = surface.GetIsoUCurve (x0);
            MSBsplineCurve curveB;
            if (SUCCESS == curveB.CopySegment (*curveA, y0, y1))
                {
                chain->push_back (ICurvePrimitive::CreateBsplineCurveSwapFromSource (curveB));
                accept = true;
                }
            }
        else if (fabs (dy) < s_smallParamChange)
            {
            MSBsplineCurvePtr curveA = surface.GetIsoVCurve (y0);
            MSBsplineCurve curveB;
            if (SUCCESS == curveB.CopySegment (*curveA, x0, x1))
                {
                chain->push_back (ICurvePrimitive::CreateBsplineCurveSwapFromSource (curveB));
                accept = true;
                }
            }
        }
    return accept;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/93
+---------------+---------------+---------------+---------------+---------------+------*/
static void fitAppendAndClear
(
CurveVectorPtr &chain,
bvector<DPoint3d> &points,
bool            cubicFit,
double          tolerance
)
    {
    int numPoints = (int)points.size ();
    MSBsplineCurve curve;
    curve.Zero ();
    if (numPoints > 1)
        {
        if (cubicFit)
            {
            bspconv_cubicFitAndReducePoints (chain, &points[0], numPoints, tolerance);
            }
        else
            {
            if (SUCCESS == bspconv_lstringToCurveStruct (&curve, &points[0], numPoints))
                chain->push_back (ICurvePrimitive::CreateBsplineCurveSwapFromSource (curve));
            }
        }
    points.clear ();
    }

// NEEDS_WORK look for existing curves !!!
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             03/13
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr MSBsplineSurface::GetUnstructuredBoundaryCurves (double tolerance, bool cubicFit, bool addOuterLoopIfActive) const
    {
    DPoint3d        surfacePoint;
    BsurfBoundary   *currBnd, *endB;
    CurveVectorPtr parent = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);  // Needs work -- should make loops !!!
    if (numBounds != 0)
        {
        bvector<DPoint3d>points;
        bvector<DPoint2d>params;
        static double s_edgeTolerance = 1.0e-10;
        for (currBnd=endB=this->boundaries, endB += this->numBounds;
             currBnd < endB; currBnd++)
            {
            int edgeCode;
            int numOnEdge;
            int i0 = 0;
            while (i0 + 1 < currBnd->numPoints
                  && bsputil_countPointsToEdgeBreak (currBnd->points, i0, currBnd->numPoints, s_edgeTolerance, edgeCode, numOnEdge)
                  )
                {
                if (fastFitAppendAndClear (*this, parent, currBnd->points, i0, numOnEdge))
                    {
                    }
                else
                    {
                    points.clear ();
                    params.clear ();
                    for (int i = i0; i < i0 + numOnEdge; i++)
                        {
                        DPoint2d uv = currBnd->points[i];
                        params.push_back (uv);
                        bspsurf_evaluateSurfacePoint (&surfacePoint, NULL, NULL, NULL, uv.x, uv.y, this);
                        points.push_back (surfacePoint);
                        }
                    fitAppendAndClear (parent, points, cubicFit, tolerance);
                    }
                i0 += numOnEdge - 1;
                }
            }
        }

    if (addOuterLoopIfActive && (numBounds == 0 ||!holeOrigin))
        {
        if (GetIsUClosed () && !GetIsVClosed ())
            {
            MSBsplineCurvePtr curveV0 = GetIsoVCurve (0.0);
            MSBsplineCurvePtr curveV1 = GetIsoVCurve (1.0);
            parent->push_back (ICurvePrimitive::CreateBsplineCurveSwapFromSource (*curveV0));
            curveV1->CopyReversed (*curveV1);
            parent->push_back (ICurvePrimitive::CreateBsplineCurveSwapFromSource (*curveV1));
            }
        else if (GetIsVClosed () && !GetIsUClosed ())
            {
            MSBsplineCurvePtr curveU0 = GetIsoUCurve (0.0);
            MSBsplineCurvePtr curveU1 = GetIsoUCurve (1.0);
            parent->push_back (ICurvePrimitive::CreateBsplineCurveSwapFromSource (*curveU1));
            curveU0->CopyReversed (*curveU0);
            parent->push_back (ICurvePrimitive::CreateBsplineCurveSwapFromSource (*curveU0));
            }
        else
            {
            MSBsplineCurvePtr curveU0 = GetIsoUCurve (0.0);
            MSBsplineCurvePtr curveU1 = GetIsoUCurve (1.0);
            MSBsplineCurvePtr curveV0 = GetIsoVCurve (0.0);
            MSBsplineCurvePtr curveV1 = GetIsoVCurve (1.0);
            parent->push_back (ICurvePrimitive::CreateBsplineCurveSwapFromSource (*curveV0));
            parent->push_back (ICurvePrimitive::CreateBsplineCurveSwapFromSource (*curveU1));
            curveV1->CopyReversed (*curveV1);
            parent->push_back (ICurvePrimitive::CreateBsplineCurveSwapFromSource (*curveV1));
            curveU0->CopyReversed (*curveU0);
            parent->push_back (ICurvePrimitive::CreateBsplineCurveSwapFromSource (*curveU0));
            }
        }
    if (parent->size () > 0)
        return parent;
    return NULL;
    }
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             03/13
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr MSBsplineSurface::GetUnstructuredBoundaryCurves (double tolerance, bool cubicFit) const
    {
    return GetUnstructuredBoundaryCurves (tolerance, cubicFit, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             03/13
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineCurvePtr MSBsplineSurface::GetPolygonRowAsCurve (int index) const
    {
    MSBsplineCurve curve;
    if (index < 0 || index >= vParams.numPoles - 1)
        index = vParams.numPoles - 1;
    int rowIndex = index * uParams.numPoles;
    curve.Populate (
        &poles[rowIndex],
        weights != NULL ? &weights[rowIndex] : NULL,
        uParams.numPoles,
        uKnots,
        uParams.NumberAllocatedKnots (),
        uParams.order,
        uParams.closed ? true : false,
        true
        );
    return curve.CreateCapture ();
    }

    

MSBsplineCurvePtr MSBsplineSurface::GetPolygonColumnAsCurve (int index) const
    {
    MSBsplineCurve curve;
    curve.Zero ();
    size_t baseIndex = index;
    if (index < 0 || index >= uParams.numPoles)
        baseIndex = uParams.numPoles - 1;
    size_t indexStep = (size_t)uParams.numPoles;
    curve.params = vParams;
    curve.Allocate ();
    curve.rational = rational;
    size_t numPoles = (size_t)vParams.numPoles;
    for (size_t j = 0; j < numPoles; j++)
        curve.poles[j] = poles[baseIndex + j * indexStep];
    if (rational)
        for (size_t j = 0; j < numPoles; j++)
            curve.weights[j] = weights[index + j];
    size_t numWeights = vParams.NumberAllocatedKnots ();
    for (size_t j = 0; j < numWeights; j++)
        curve.knots[j] = vKnots[j];
    return curve.CreateCapture ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bspconv_extractCapAsSurface
(
MSBsplineSurface    *capP,              /* <= cap surface (planar) */
MSBsplineSurface    *solidP,            /* => surface */
double              tolerance,          /* => choord height tolerance */
bool                lastRow             /* => false gives surface start */
)
    {
    int status = ERROR;
    MSBsplineCurvePtr curve = solidP->GetPolygonRowAsCurve (lastRow == 0 ? 0 : -1);
    // EDL 3/13 behavior change:  If the surface had disjoint parts (uh oh -- maybe it can happen on a swept grouped hole?)
    // the parts now go in as a single curve.
    if (curve.IsValid ())
        status = bspsurf_trimmedPlaneFromCurves (capP, curve.get (), 1, tolerance);
    return status;
    }

/*---------------------------------------------------------------------*//**
Convert an arc of center + vector0*cos(theta)+vector90*sin(theta)
Convert a portion of a DConic4d to a (single) bspline curve.
@param curveP OUT initialized curve structure
@param ellipseP OUT ellipse -- undefined if hyperbola, parabola, or line.
@param isEllipseP OUT true if the curve is (simply) an ellipse.
@param hConicP IN homogeneous conic.
@param s0 IN start parameter in conic angle space.
@param s1 IN end parameter in conic angle space
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bspcurv_convertHomogeneousConicToCurve
(
MSBsplineCurve  *curveP,        /* <= Curve structure to initialize and fill */
DEllipse3dP     ellipseP,
      bool      *isEllipseP,/* <= true if the spline has standard mapping to ellipse
                                        (e.g. false for any hyperbola or parabola) */
      DPoint4dCP centerP,
      DPoint4dCP vector0P,
      DPoint4dCP vector90P,
      double    s0,             /* => start parameter */
      double    s1              /* => end parameter */
)
    {
    DPoint3d    center;
    DVec3d     vector0, vector1, vector2;
    RotMatrix   rotMatrix;
    double rad0, rad1;
    static double zeroTol = 1.0e-9;
    double dot;
    StatusInt status = ERROR;

    centerP->GetXYZ (center);
    vector0P->GetXYZ (vector0);
    vector90P->GetXYZ (vector1);

    rad0 = vector0.Normalize ();
    rad1 = vector1.Normalize ();
    dot = vector0.DotProduct (vector1);  /* like cos(angle between vec0 and vec1) * rad0 * rad1) */
    if  (   fabs (vector0P->w)      < zeroTol * rad0
         && fabs (vector90P->w)      < zeroTol * rad1
         && fabs (1.0 - centerP->w) < zeroTol
         && fabs (dot) < zeroTol * rad0 * rad1
        )
        {
        double ds = s1 - s0;
        vector2.CrossProduct (vector0, vector1);
        rotMatrix.InitFromColumnVectors (vector0, vector1, vector2);
        if (isEllipseP)
            *isEllipseP = true;
        status = bspconv_arcToCurveStruct (curveP, s0, ds, rad0, rad1, &rotMatrix, &center);
        if (ellipseP)
            {
            vector0.Scale (rad0);
            vector1.Scale (rad1);
            ellipseP->InitFromVectors (center, vector0, vector1, s0, ds);
            }
        }
    else
        {
        DPoint4d hPoint;
        int i;
        static double zeroWeightTol = 1.0e-8;
        double refWeight;
        double maxWeight, minWeight;
        center.Zero ();
        rotMatrix.InitIdentity ();
        *isEllipseP = false;

        /* Any conic section is a unit circle mangled by a homogeneous transformation.  The center, vector0,
            and vector90 data in the DEllipse4d gives the transformation.  So we build a Bspline for
            the arc of the unit circle . . . */

        if (SUCCESS == bspconv_arcToCurveStruct (curveP, s0, s1 - s0, 1.0, 1.0, &rotMatrix, &center))
            {
            /* Apply the transformation to carry the unit circle out to real space . . . */
            minWeight = DBL_MAX;
            maxWeight = -DBL_MAX;
            for (i = 0; i < curveP->params.numPoles; i++)
                {
                /* All the poles have z=0 !!! */
                hPoint.SumOf(*vector0P, curveP->poles[i].x, *vector90P, curveP->poles[i].y, *centerP, curveP->weights[i]);
                curveP->poles[i].x = hPoint.x;
                curveP->poles[i].y = hPoint.y;
                curveP->poles[i].z = hPoint.z;
                curveP->weights[i] = hPoint.w;
                if (0 == i)
                    {
                    minWeight = maxWeight = hPoint.w;
                    }
                else
                   {
                    if (hPoint.w > maxWeight)
                        maxWeight = hPoint.w;
                    if (hPoint.w <minWeight)
                        minWeight = hPoint.w;
                    }
                }

            /* If the given angular range does NOT include an asymptote, the weights will all be
                the same sign.   Verify that this is the case, and set refWeight to the extremal
                weight  . .  */
            if (    maxWeight > 0.0 && minWeight > zeroWeightTol * maxWeight)
                {
                refWeight = maxWeight;
                }
            else if (minWeight < 0.0 && maxWeight < zeroWeightTol * minWeight)
                {
                refWeight = minWeight;
                }
            else
                {
                refWeight = 0.0;
                }

            if (refWeight != 0.0)
                {
                /* Dividing through by the extreme weight moves all the weights between 0 and 1 */
                double scale = 1.0 / refWeight;
                for (i = 0; i < curveP->params.numPoles; i++)
                    {
                    curveP->weights[i] *= scale;
                    curveP->poles[i].x *= scale;
                    curveP->poles[i].y *= scale;
                    curveP->poles[i].z *= scale;
                    }
                status = SUCCESS;
                }
            else
                {
                bspcurv_freeCurve (curveP);
                }
            }
        }
    return status;
    }

/*---------------------------------------------------------------------*//**
Convert a portion of a DConic4d to a (single) bspline curve.
@param curveP OUT initialized curve structure
@param ellipseP OUT ellipse form -- undefined if hyperbola or parabola.
@param isEllipseP OUT true if the ellipse is defined
@param segmentP OUT segment if curve is (simply) a segment.
@param isSegmentP OUT true if segment is defined
@param hConicP IN homogeneous conic.
@param s0 IN start parameter in conic angle space.
@param s1 IN end parameter in conic angle space
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bspcurv_simplifyHConic
(
MSBsplineCurve  *curveP,        /* <= Curve structure to initialize and fill */
DEllipse3dP      ellipseP,      /* <= ellipse (undefined if curve parabola or hyperbola) */
      bool      *isEllipseP, /* <= true if result is a standard ellipse */
DSegment3dP     segmentP,
      bool      *isSegmentP,
const HConic    *hConicP,       /* => conic to evaluate */
      double    s0,             /* => start parameter */
      double    s1              /* => end parameter */
)
    {
    DEllipse4d normalizedEllipse;
    DEllipse4d baseEllipse;
    StatusInt status = ERROR;
    double theta0, theta1;
    if (NULL != isEllipseP)
        *isEllipseP = false;
    if (NULL != isSegmentP)
        *isSegmentP = false;

    if (curveP)
        memset (curveP, 0, sizeof (MSBsplineCurve));

    switch (hConicP->type)
        {
        case HConic_Ellipse:
            baseEllipse = hConicP->coordinates;
            bsiRange1d_setUncheckedArcSweep (&baseEllipse.sectors, s0, s1 - s0);
            if (   bsiDEllipse4d_normalizeWeights(&normalizedEllipse, &baseEllipse)
                && bsiDEllipse4d_getSector(&normalizedEllipse, &theta0, &theta1, 0)
                )
                {
                status = bspcurv_convertHomogeneousConicToCurve
                                                (
                                                curveP,
                                                ellipseP,
                                                isEllipseP,
                                                &normalizedEllipse.center,
                                                &normalizedEllipse.vector0,
                                                &normalizedEllipse.vector90,
                                                theta0,
                                                theta1
                                                );
                }
            else if (bsiDEllipse4d_getSector(&baseEllipse, &theta0, &theta1, 0))
                {
                status = bspcurv_convertHomogeneousConicToCurve
                                                (
                                                curveP,
                                                ellipseP,
                                                isEllipseP,
                                                &baseEllipse.center,
                                                &baseEllipse.vector0,
                                                &baseEllipse.vector90,
                                                theta0,
                                                theta1
                                                );
                }
            break;
        case HConic_Line:
            {
            DPoint4d hStart, hEnd;
            DPoint3d pointList[2];

            hStart.SumOf(*(&hConicP->coordinates.vector0), 1.0 - s0, *(&hConicP->coordinates.vector90), s0);

            hEnd.SumOf(*(&hConicP->coordinates.vector0), 1.0 - s1, *(&hConicP->coordinates.vector90), s1);

            if (   hStart.GetProjectedXYZ (pointList[0])
                && hEnd.GetProjectedXYZ (pointList[1])
               )
                {
                status = bspconv_lstringToCurveStruct (curveP, pointList, 2);
                if (isSegmentP)
                    *isSegmentP = true;
                if (segmentP)
                    segmentP->Init (pointList[0], pointList[1]);
                }
            }
            break;
        case HConic_LinePair:
            break;
        }
    return status;
    }


END_BENTLEY_GEOMETRY_NAMESPACE
