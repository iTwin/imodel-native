/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnBRep/PSolidUtil.h>
#include <Vu/VuApi.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN

static const double TOLERANCE_UVSpaceFitting    = .00001;
static const double TOLERANCE_UVSpaceAngle      = .4;
static const double TOLERANCE_UVSpaceLength     = 0.2;
static const double TOLERANCE_UVSpaceLinear     = 1.0e-8;

static const double COSINE_TOLERANCE            = .995;

static double       MIN_CURVATURE_DIVISOR       = 1.0E6;
static double       MIN_SP_CURVE_TOL            = 1.0e-7;
static double       MAX_SP_CURVE_TOL            = 1.0e-5;

static const int    MAX_U_NUM_APPLY_REDUCE      = 1000;
static const int    MAX_V_NUM_APPLY_REDUCE      = 1000;
static const int    MAX_UV_NUM_APPLY_REDUCE     = 1000;

typedef std::vector<PK_ENTITY_t> TAG_LIST;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DavidAssaf      10/05
+---------------+---------------+---------------+---------------+---------------+------*/
static void* pki_realloc(void* pOldPtr, int oldNumBytes, int newNumBytes)
    {
    void*   pMemory = NULL;

    if (newNumBytes >= 0 && oldNumBytes >= 0)
        {
        if (newNumBytes == oldNumBytes)
            return newNumBytes ? pOldPtr : NULL;

        if (newNumBytes > 0 && PK_ERROR_no_errors != PK_MEMORY_alloc (newNumBytes, &pMemory))
            return NULL;

        if (pOldPtr)
            {
            if (oldNumBytes > 0)
                memcpy (pMemory, pOldPtr, (oldNumBytes < newNumBytes) ? oldNumBytes : newNumBytes);

            PK_MEMORY_free (pOldPtr);
            }
        }

    return pMemory;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DavidAssaf      10/05
+---------------+---------------+---------------+---------------+---------------+------*/
static void* pki_calloc(int numRecords, int numRecordBytes)
    {
    void*   pMemory = NULL;
    int     numBytes = numRecords * numRecordBytes;

    if (numBytes > 0 && PK_ERROR_no_errors == PK_MEMORY_alloc (numBytes, &pMemory))
        memset (pMemory, 0, numBytes);

    return pMemory;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/97
+---------------+---------------+---------------+---------------+---------------+------*/
static void     extractKnots
(
double*         knotP,
BsplineParam*   paramP,
int             nKnotValues,
int*            multiplicityP,
double*         valueP,
bool            periodic
)
    {
    int         i, numKnots = bspknot_numberKnots (paramP->numPoles, paramP->order, paramP->closed);
    double      *pP;

    for (i=0, pP = knotP; i < nKnotValues; i++)
        {
        int     j, multiplicity = multiplicityP[i];
        double  knotValue = valueP[i];

        for (j=0; j<multiplicity; j++)
            *pP++ = knotValue;
        }

    if (periodic)
        {
        knotP[0] = knotP[1];
        knotP[numKnots-1] = knotP[numKnots-2];
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    LuHan           07/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void     fixDegenerateEnds
(
MSBsplineCurveP pCurve,     /* <=> curve */
double          tolerance   /* => tolerance used to check degenercy */
)
    {
    int         i, j, numPoles;
    double      dist, shift;
    DPoint3d    dir;

    pCurve->CopyOpen (*pCurve,  0.0);

    numPoles = pCurve->params.numPoles;

    if (pCurve->rational)
        bsputil_unWeightPoles (pCurve->poles, pCurve->poles, pCurve->weights, pCurve->params.numPoles);

    // Check first pole
    for (i = 1; i < numPoles; i++)
        {
        dir.DifferenceOf (*( &pCurve->poles[i]), *( &pCurve->poles[0]));
        dist = dir.Magnitude ();

        if ((dist = dir.Magnitude ()) > tolerance)
            {
            if (i > 1)
                {
                shift = dist / (numPoles+200.0);
                dir.Normalize ();

                for (j = 1; j < i; j++)
                    (pCurve->poles[j]).SumOf(*( &pCurve->poles[j]), dir,  j * shift);
                }

            break;
            }
        }

    // Check last pole
    for (i = numPoles-2; i >= 0; i--)
        {
        dir.DifferenceOf (*( &pCurve->poles[i]), *( &pCurve->poles[numPoles-1]));
        dist = dir.Magnitude ();

        if ((dist = dir.Magnitude ()) > tolerance)
            {
            if (i < numPoles-2)
                {
                shift = dist / (numPoles+200.0);
                dir.Normalize ();

                for (j = numPoles-2; j > i; j--)
                    (pCurve->poles[j]).SumOf(*( &pCurve->poles[j]), dir,  (numPoles-1-j) * shift);
                }

            break;
            }
        }

    if (pCurve->rational)
        bsputil_weightPoles (pCurve->poles, pCurve->poles, pCurve->weights, pCurve->params.numPoles);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            PSolidUtil::NormalizeBsplineCurve (MSBsplineCurveR curve)
    {
    // NOTE: Hopefully this does the same thing as the old implementation that used functions in nlib...
    bvector<MSBsplineCurvePtr> beziers;

    curve.CopyAsBeziers (beziers);

    for (size_t i = 0; i < beziers.size (); ++i)
        {
        DRange3d range;
        beziers[i]->GetPoleRange (range);
        // If all points are identical, remove from array.
        if (range.low.AlmostEqual (range.high))
            {
            beziers[i]->ReleaseMem ();    // maybe not needed? ctor dtor on Ptr would do it anyway?
            beziers.erase (beziers.begin () + i);
            continue;
            }
        // ??? (EDL) If this bezier has poles in a line, rebuild it as a 2-point order 2 curve (line segment) and raise the degree back to its original.
        //  This makes it the simplest parameterization.
        if (!bsputil_isLinearArray (beziers[i]->poles, beziers[i]->weights, beziers[i]->rational, beziers[i]->params.numPoles, COSINE_TOLERANCE))
            continue;

        DPoint3d    points[2];

        beziers[i]->ExtractEndPoints (points[0], points[1]);
        beziers[i]->ReleaseMem ();
        beziers[i]->InitFromPoints (points, 2);
        beziers[i]->ElevateDegree (curve.params.order - 1);

        if (curve.rational)
            beziers[i]->MakeRational ();
        }

    bool    closed = TO_BOOL (curve.params.closed);

    curve.ReleaseMem ();
    auto newCurve = MSBsplineCurve::CreateFromBeziers (beziers);
    curve.SwapContents (*newCurve);

    if (closed)
        curve.CopyClosed (curve);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      02/98
+---------------+---------------+---------------+---------------+---------------+------*/
double          PSolidUtil::CalculateToleranceFromMinCurvature (PK_CURVE_t curve, PK_INTERVAL_t* intervalP)
    {
    PK_CLASS_t  curveClass;

    if (SUCCESS != PK_ENTITY_ask_class (curve, &curveClass) && PK_CLASS_spcurve == curveClass)
        return MIN_SP_CURVE_TOL;

    PK_SPCURVE_sf_t sp_sf;
    PK_UVBOX_t      uvBox;
    PK_VECTOR_t     center;
    PK_VECTOR_t     axes[3];
    double          widths[3], retValue;
    int             dim;
    DPoint3d        orig, xA, yA, zA, diag;

    if (SUCCESS == PK_SPCURVE_ask (curve, &sp_sf) &&
        SUCCESS == PK_SURF_ask_uvbox (sp_sf.surf, &uvBox) &&
        SUCCESS == PK_SURF_find_non_aligned_box (sp_sf.surf, uvBox, &center, axes, widths, &dim))
        {
        orig.x = orig.y = orig.z = 0.0;
        xA.x = axes[0].coord[0];
        xA.y = axes[0].coord[1];
        xA.z = axes[0].coord[2];
        yA.x = axes[1].coord[0];
        yA.y = axes[1].coord[1];
        yA.z = axes[1].coord[2];
        zA.x = axes[2].coord[0];
        zA.y = axes[2].coord[1];
        zA.z = axes[2].coord[2];

        diag.SumOf(orig, xA,  widths[0]);
        diag.SumOf(diag, yA,  widths[1]);
        diag.SumOf(diag, zA,  widths[2]);
        retValue = orig.Distance (diag) / MIN_CURVATURE_DIVISOR;

        return (retValue > MAX_SP_CURVE_TOL) ? MAX_SP_CURVE_TOL : (retValue < MIN_SP_CURVE_TOL ? MIN_SP_CURVE_TOL : retValue);
        }

    return MAX_SP_CURVE_TOL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RichardTrefz    05/02
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidGeom::CreateMSBsplineCurveFromBCurve (MSBsplineCurveR curve, PK_BCURVE_t curveTag, bool normalizeKnotVector)
    {
    PK_PARAM_sf_t   param;

    // Convert periodic curve to non-periodic curve
    PK_CURVE_ask_param (curveTag, &param);

    if (param.periodic != PK_PARAM_periodic_no_c)
        {
        // Need to insert start parameter as knot. The last parameter is added automatically
        for (PK_ERROR_code_t errorCode = 0; !errorCode; )
            errorCode = PK_BCURVE_add_knot (curveTag, param.range.value[0]);
        }

    memset (&curve, 0, sizeof (curve));

    PK_BCURVE_sf_t  sfCurve;

    if (PK_ERROR_no_errors != PK_BCURVE_ask (curveTag, &sfCurve))
        return ERROR;

    curve.rational              = sfCurve.is_rational;
    curve.params.order          = sfCurve.degree + 1;
    curve.display.curveDisplay  = true;
    curve.params.numPoles       = sfCurve.n_vertices;
    curve.params.numKnots       = curve.params.numPoles - curve.params.order;

    BentleyStatus   status = ERROR;

    if (SUCCESS == curve.Allocate ())
        {
        int         i;
        double      *pW, *pPoles;
        DPoint3d    *pP, *endP;

        for (i=0, pP = curve.poles, endP = pP + curve.params.numPoles, pW = curve.weights, pPoles = sfCurve.vertex; pP < endP; i++, pP++)
            {
            pP->x = *pPoles++;
            pP->y = *pPoles++;

            if (curve.rational)
                {
                pP->z = (sfCurve.vertex_dim > 3) ? *pPoles++ : 0.0;
                *pW++ = *pPoles++;
                }
            else
                {
                pP->z = (sfCurve.vertex_dim > 2) ? *pPoles++ : 0.0;
                }
            }

        extractKnots (curve.knots, &curve.params, sfCurve.n_knots, sfCurve.knot_mult, sfCurve.knot, param.periodic != PK_PARAM_periodic_no_c);

        if (normalizeKnotVector)
            bspknot_normalizeKnotVector (curve.knots, curve.params.numPoles, curve.params.order, curve.params.closed);

        if (sfCurve.is_closed)
            curve.MakeClosed ();

        status = SUCCESS;
        }

    PK_MEMORY_free (sfCurve.vertex);
    PK_MEMORY_free (sfCurve.knot_mult);
    PK_MEMORY_free (sfCurve.knot);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      02/98
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidGeom::CreateBCurveFromSPCurve (PK_BCURVE_t& bCurveTag, PK_SPCURVE_t spCurve, PK_INTERVAL_t* intervalP, PK_LOGICAL_t& isExact, bool makeNonPeriodic)
    {
    PK_INTERVAL_t   interval;

    if (NULL == intervalP)
        PK_CURVE_ask_interval (spCurve, intervalP = &interval);

    PK_INTERVAL_t   curveRange;

    if (intervalP->value[1] < intervalP->value[0])
        {
        curveRange.value[0] = intervalP->value[1];
        curveRange.value[1] = intervalP->value[0];
        }
    else
        {
        curveRange.value[0] = intervalP->value[0];
        curveRange.value[1] = intervalP->value[1];
        }

    double      tolerance = PSolidUtil::CalculateToleranceFromMinCurvature (spCurve, &curveRange);

    if (SUCCESS != PK_CURVE_make_bcurve (spCurve, curveRange, PK_LOGICAL_false, PK_LOGICAL_false, tolerance, &bCurveTag, &isExact) &&
        SUCCESS != PK_CURVE_make_bcurve (spCurve, curveRange, PK_LOGICAL_false, PK_LOGICAL_false, 1.0e-3, &bCurveTag, &isExact))
        return ERROR;

    if (!makeNonPeriodic)
        return SUCCESS;

    PK_PARAM_sf_t   param;

    // Convert periodic curve to non-periodic curve
    PK_CURVE_ask_param (bCurveTag, &param);

    if (param.periodic != PK_PARAM_periodic_no_c)
        {
        // Need to insert start parameter as knot. The last parameter is added automatically
        for (PK_ERROR_code_t errorCode = 0; !errorCode; )
            errorCode = PK_BCURVE_add_knot (bCurveTag, param.range.value[0]);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      02/98
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidGeom::CreateMSBsplineCurveFromSPCurve (MSBsplineCurveR curve, PK_SPCURVE_t spCurve, PK_INTERVAL_t* intervalP, bool* isExactP)
    {
    PK_BCURVE_t     bCurveTag;
    PK_LOGICAL_t    isExact = PK_LOGICAL_true;

    if (SUCCESS != CreateBCurveFromSPCurve (bCurveTag, spCurve, intervalP, isExact, false))
        return ERROR;

    BentleyStatus   status = PSolidGeom::CreateMSBsplineCurveFromBCurve (curve, bCurveTag, true);

    PK_ENTITY_delete (1, &bCurveTag);

    if (isExactP)
        *isExactP = TO_BOOL (isExact);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/01
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidGeom::CreateMSBsplineCurveFromCurve (MSBsplineCurveR curve, PK_CURVE_t curveTag, PK_INTERVAL_t& interval, bool reverse, double tolerance, bool* isExactP)
    {
    PK_LOGICAL_t    isExact = PK_LOGICAL_true;
    PK_CLASS_t      curveClass;

    PK_ENTITY_ask_class (curveTag, &curveClass);

    switch (curveClass)
        {
        case PK_CLASS_line:
            {
            PK_LINE_sf_t    sfLine;

            if (SUCCESS != PK_LINE_ask (curveTag, &sfLine))
                return ERROR;

            DPoint3d    points[2];

            points[reverse ? 1 : 0].SumOf (*((DPoint3dP) sfLine.basis_set.location.coord),*((DVec3dP) sfLine.basis_set.axis.coord), interval.value[0]);
            points[reverse ? 0 : 1].SumOf (*((DPoint3dP) sfLine.basis_set.location.coord),*((DVec3dP) sfLine.basis_set.axis.coord), interval.value[1]);

            if (SUCCESS != curve.InitFromPoints (points, 2))
                return ERROR;

            break;
            }

        case PK_CLASS_circle:
            {
            PK_CIRCLE_sf_t  sfCircle;

            if (SUCCESS != PK_CIRCLE_ask (curveTag, &sfCircle))
                return ERROR;

            double      start, sweep;
            DVec3d      xVector, yVector;

            xVector.Scale (*((DVec3dP) &sfCircle.basis_set.ref_direction.coord), sfCircle.radius);
            yVector.CrossProduct (*((DVec3dP) sfCircle.basis_set.axis.coord), *((DVec3dP)  &xVector));

            PSolidUtil::ExtractStartAndSweepFromInterval (start, sweep, interval, reverse);

            DEllipse3d  ellipse;

            ellipse.InitFromVectors (*(DPoint3dP) sfCircle.basis_set.location.coord, xVector, yVector, start, sweep);

            if (SUCCESS != curve.InitFromDEllipse3d (ellipse))
                return ERROR;

            break;
            }

        case PK_CLASS_ellipse:
            {
            PK_ELLIPSE_sf_t  sfEllipse;

            if (SUCCESS != PK_ELLIPSE_ask (curveTag, &sfEllipse))
                break;

            double      start, sweep;
            DVec3d      xVector, yVector;

            yVector.CrossProduct (*((DVec3dP) sfEllipse.basis_set.axis.coord), *((DVec3dP)  &xVector));
            xVector.Scale (*((DVec3dP)  &sfEllipse.basis_set.ref_direction.coord), sfEllipse.R1);
            yVector.Scale (*((DVec3dP)  &sfEllipse.basis_set.ref_direction.coord), sfEllipse.R2);

            PSolidUtil::ExtractStartAndSweepFromInterval (start, sweep, interval, reverse);

            DEllipse3d  ellipse;

            ellipse.InitFromVectors (*(DPoint3dP) sfEllipse.basis_set.location.coord, xVector, yVector, start, sweep);

            if (SUCCESS != curve.InitFromDEllipse3d (ellipse))
                return ERROR;

            break;
            }

        case PK_CLASS_icurve:
        case PK_CLASS_bcurve:
            {
            PK_BCURVE_t     bCurveTag;
            PK_INTERVAL_t   curveRange;

            if (interval.value[1] < interval.value[0])
                {
                curveRange.value[0] = interval.value[1];
                curveRange.value[1] = interval.value[0];
                }
            else
                {
                curveRange.value[0] = interval.value[0];
                curveRange.value[1] = interval.value[1];
                }

            if (SUCCESS != PK_CURVE_make_bcurve (curveTag, curveRange, PK_LOGICAL_false, PK_LOGICAL_false, tolerance, &bCurveTag, &isExact))
                return ERROR;

            BentleyStatus   status = PSolidGeom::CreateMSBsplineCurveFromBCurve (curve, bCurveTag, true);

            PK_ENTITY_delete (1, &bCurveTag);

            if (SUCCESS != status)
                return status;

            if (reverse)
                curve.CopyReversed (curve);

            break;
            }

        case PK_CLASS_spcurve:
            {
            if (SUCCESS != PSolidGeom::CreateMSBsplineCurveFromSPCurve (curve, curveTag, &interval))
                return ERROR;

            if (reverse)
                curve.CopyReversed (curve);

            break;
            }

        default:
            {
//          BeAssert (false);

            return ERROR;
            }
        }

    if (isExactP)
        *isExactP = TO_BOOL (isExact);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      02/98
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidGeom::CreateCurveFromMSBsplineCurve (PK_CURVE_t* curveP, MSBsplineCurveCR msbCurve)
    {
    int             i, dim, numKnots, numPoles, index, periodic = false, closed;
    double          *pPoles = NULL, *pW = NULL;
    DPoint3d        *pP = NULL;
    PK_BCURVE_sf_t  bCurv;
    PK_ERROR_code_t failureCode;
    MSBsplineCurve  curve;

    curve.CopyFrom (msbCurve);
    fixDegenerateEnds (&curve, 1.0e-8);

    if (SUCCESS != curve.CleanKnots ())
        {
        curve.ReleaseMem ();

        return ERROR;
        }

    // compute spline size parameters 
    numKnots = bspknot_numberKnots (curve.params.numPoles, curve.params.order, curve.params.closed);

    numPoles = numKnots - curve.params.order;
    closed   = curve.IsPhysicallyClosed ( 1.0e-8);
    dim      = curve.rational ? 4 : 3;

    // malloc memory for poles
    if (NULL == (bCurv.vertex = (double *) malloc (numPoles * dim * sizeof (double))))
        {
        curve.ReleaseMem ();

        return ERROR;
        }

    index  = periodic ? curve.params.numPoles : numPoles;
    pPoles = bCurv.vertex;

    for (i = 0, pP = curve.poles, pW = curve.weights; i < index; i++, pP++)
        {
        *pPoles++ = pP->x;
        *pPoles++ = pP->y;
        *pPoles++ = pP->z;

        if (curve.rational)
            *pPoles++ = *pW++;
        }

    if (periodic)
        {
        for (i = index, pP=curve.poles, pW = curve.weights; i < numPoles; i++, pP++)
            {
            *pPoles++ = pP->x;
            *pPoles++ = pP->y;
            *pPoles++ = pP->z;

            if (curve.rational)
                *pPoles++ = *pW++;
            }
        }

    // assign knots
    if (NULL == (bCurv.knot = (double *) malloc (numKnots * sizeof (double))) ||
        NULL == (bCurv.knot_mult = (int *) malloc (numKnots * sizeof (int))))
        {
        curve.ReleaseMem ();

        return ERROR;
        }

    bsputil_getKnotMultiplicityExt (bCurv.knot, bCurv.knot_mult, &bCurv.n_knots, curve.knots, numPoles, curve.params.order, curve.params.closed, bspknot_knotTolerance (&curve));

    // assign bCurv structure
    bCurv.degree            = curve.params.order - 1;
    bCurv.n_vertices        = numPoles;
    bCurv.vertex_dim        = dim;
    bCurv.is_rational       = (Byte) curve.rational;
    bCurv.form              = PK_BCURVE_form_unset_c;
    bCurv.knot_type         = PK_knot_unset_c;
    bCurv.is_periodic       = periodic == 0 ? PK_LOGICAL_false : PK_LOGICAL_true;
    bCurv.is_closed         = closed == 0 ? PK_LOGICAL_false : PK_LOGICAL_true;    /* need to check physically closed */
    bCurv.self_intersecting = PK_self_intersect_unset_c;

    failureCode = PK_BCURVE_create (&bCurv, curveP);

    curve.ReleaseMem ();

    free (bCurv.vertex);
    free (bCurv.knot);
    free (bCurv.knot_mult);

    return (BentleyStatus) failureCode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Nikolay.Shulga  02/98
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidGeom::CreateCurveFromMSBsplineCurve2d (PK_CURVE_t* curveP, MSBsplineCurveCR msbCurve)
    {
    int         curveTag;

    if (SUCCESS != PSolidGeom::CreateCurveFromMSBsplineCurve (&curveTag, msbCurve))
        return ERROR;

    StatusInt       status = ERROR;
    int             bCurveTag = 0;
    double          tolerance = 1.0e-8;
    PK_LOGICAL_t    isExact = 0;
    PK_INTERVAL_t   range;
    PK_BCURVE_sf_t  tmpCurveStruct;

    // get parametric range; then convert to bspline curve (need range for that then get standard form and convert to 2d
    if (SUCCESS == PK_CURVE_ask_interval (curveTag, &range) &&
        SUCCESS == PK_CURVE_make_bcurve (curveTag, range, 0, 0, tolerance, &bCurveTag, &isExact) &&
        SUCCESS == PK_BCURVE_ask (bCurveTag, &tmpCurveStruct))
        {
        double  *pOldVertex, *pNewVertex, *pTmp;

        tmpCurveStruct.vertex_dim -= 1;
        pTmp = pOldVertex = tmpCurveStruct.vertex;
        pNewVertex = tmpCurveStruct.vertex = (double *) calloc (tmpCurveStruct.vertex_dim * tmpCurveStruct.n_vertices, sizeof (*pNewVertex));

        if (3 == tmpCurveStruct.vertex_dim)
            {
            for (int i = 0; i < tmpCurveStruct.n_vertices; i++)
                {
                *pNewVertex++ = *pOldVertex++;
                *pNewVertex++ = *pOldVertex++;
                ++pOldVertex; // skip z
                *pNewVertex++ = *pOldVertex++; // weight
                }
            }
        else
            {
            for (int i = 0; i < tmpCurveStruct.n_vertices; i++)
                {
                *pNewVertex++ = *pOldVertex++;
                *pNewVertex++ = *pOldVertex++;
                ++pOldVertex; // skip z
                }
            }

        status = PK_BCURVE_create (&tmpCurveStruct, curveP);

        free (pTmp);
        free (tmpCurveStruct.vertex);
        free (tmpCurveStruct.knot);
        free (tmpCurveStruct.knot_mult);
        }

    PK_ENTITY_delete (1, &curveTag);

    return (BentleyStatus) status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/01
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus createSurfaceFromMSBsplineSurface (PK_BSURF_t* surfaceTag, MSBsplineSurfaceR msbSurf)
    {
    StatusInt   status = SUCCESS;

    *surfaceTag = PK_ENTITY_null;

    // open closed surface...
    mdlBspline_cleanSurfaceKnots (&msbSurf, &msbSurf);
    bsprsurf_openSurface (&msbSurf, &msbSurf, 0.0, 0);
    bsprsurf_openSurface (&msbSurf, &msbSurf, 0.0, 1);

    // compute spline size parameters
    int     uNumKnots = bspknot_numberKnots (msbSurf.uParams.numPoles, msbSurf.uParams.order, msbSurf.uParams.closed);
    int     vNumKnots = bspknot_numberKnots (msbSurf.vParams.numPoles, msbSurf.vParams.order, msbSurf.vParams.closed);
    int     uNumPoles = uNumKnots - msbSurf.uParams.order;
    int     vNumPoles = vNumKnots - msbSurf.vParams.order;
    int     uPeriodic = msbSurf.uParams.closed;
    int     vPeriodic = msbSurf.vParams.closed;
    int     dim = msbSurf.rational ? 4 : 3;
    bool    uClosed, vClosed;
    double  *pPoles = NULL;

    bspsurf_surfacePhysicallyClosed (&uClosed, &vClosed, &msbSurf, 0.5);

    PK_BSURF_sf_t   bSurf;

    // malloc memory for poles
    if (NULL == (pPoles = (double *) malloc (uNumPoles * vNumPoles * dim * sizeof (double))) ||
        NULL == (bSurf.vertex = (double *) malloc (uNumPoles * vNumPoles * dim * sizeof (double))))
        return ERROR;

    int     uIndex = uPeriodic ? msbSurf.uParams.numPoles : uNumPoles;
    int     vIndex = vPeriodic ? msbSurf.vParams.numPoles : vNumPoles;
    int     i, j, iTmp, jTmp, djTmp, juTmp;

    for (i = 0; i < vIndex; i++)
        {
        iTmp = i*uNumPoles;

        for (j = 0; j < uIndex; j++)
            {
            jTmp  = j+iTmp;
            djTmp = dim * jTmp;

            pPoles[djTmp]   = msbSurf.poles[jTmp].x;
            pPoles[djTmp+1] = msbSurf.poles[jTmp].y;
            pPoles[djTmp+2] = msbSurf.poles[jTmp].z;

            if (msbSurf.rational)
                pPoles[djTmp+3] = msbSurf.weights[jTmp];
            }

        if (uPeriodic)
            {
            for (j = uIndex; j < uNumPoles; j++)
                {
                jTmp  = j+iTmp;
                djTmp = dim * jTmp;
                juTmp = jTmp - uIndex;

                pPoles[djTmp]   = msbSurf.poles[juTmp].x;
                pPoles[djTmp+1] = msbSurf.poles[juTmp].y;
                pPoles[djTmp+2] = msbSurf.poles[juTmp].z;

                if (msbSurf.rational)
                    pPoles[djTmp+3] = msbSurf.weights[juTmp];
                }
            }
        }

    if (vPeriodic)
        memcpy (pPoles+dim*uNumPoles*vIndex, pPoles, dim*uNumPoles * (msbSurf.vParams.order-1) * sizeof(double));

    double  *pP;

    // swap u and v direction since ParaSolid v-direction goes faster
    for (j = 0, pP = bSurf.vertex; j < uNumPoles; j++)
        {
        for (i = 0; i < vNumPoles; i++)
            {
            iTmp = dim*(j+i*uNumPoles);

            *pP++ = pPoles[iTmp];
            *pP++ = pPoles[iTmp+1];
            *pP++ = pPoles[iTmp+2];

            if (msbSurf.rational)
                *pP++ = pPoles[iTmp+3];
            }
        }

    // assign knots
    if (NULL == (bSurf.u_knot = (double *) malloc (uNumKnots * sizeof (double))) ||
        NULL == (bSurf.v_knot = (double *) malloc (vNumKnots * sizeof (double))) ||
        NULL == (bSurf.u_knot_mult = (int *) malloc (uNumKnots * sizeof (int))) ||
        NULL == (bSurf.v_knot_mult = (int *) malloc (vNumKnots * sizeof (int))))
        return ERROR;

    bsputil_getKnotMultiplicityExt (bSurf.u_knot, bSurf.u_knot_mult, &bSurf.n_u_knots, msbSurf.uKnots, uNumPoles, msbSurf.uParams.order, msbSurf.uParams.closed, 1.0e-10);
    bsputil_getKnotMultiplicityExt (bSurf.v_knot, bSurf.v_knot_mult, &bSurf.n_v_knots, msbSurf.vKnots, vNumPoles, msbSurf.vParams.order, msbSurf.vParams.closed, 1.0e-10);

    // assign bSurf structure
    bSurf.u_degree          = msbSurf.uParams.order - 1;
    bSurf.v_degree          = msbSurf.vParams.order - 1;
    bSurf.n_u_vertices      = uNumPoles;
    bSurf.n_v_vertices      = vNumPoles;
    bSurf.vertex_dim        = dim;
    bSurf.is_rational       = (Byte) msbSurf.rational;
    bSurf.form              = PK_BSURF_form_unset_c;
    bSurf.u_knot_type       = PK_knot_unset_c;
    bSurf.v_knot_type       = PK_knot_unset_c;
    bSurf.is_u_periodic     = uPeriodic == 0 ? PK_LOGICAL_false : PK_LOGICAL_true;
    bSurf.is_v_periodic     = vPeriodic == 0 ? PK_LOGICAL_false : PK_LOGICAL_true;
    bSurf.is_u_closed       = uClosed;
    bSurf.is_v_closed       = vClosed;
    bSurf.self_intersecting = PK_self_intersect_unset_c;
    bSurf.convexity         = PK_convexity_unset_c;

    PK_ERROR_code_t failureCode = PK_BSURF_create (&bSurf, surfaceTag);

    free (pPoles);
    free (bSurf.vertex);
    free (bSurf.u_knot);
    free (bSurf.v_knot);
    free (bSurf.u_knot_mult);
    free (bSurf.v_knot_mult);

    int                 nFaults = 0;
    PK_check_fault_t*   faults = NULL;
    PK_GEOM_check_o_t   option;

    PK_GEOM_check_o_m (option);

    if (PK_ERROR_no_errors != failureCode || SUCCESS != PK_GEOM_check (*surfaceTag, &option, &nFaults, &faults) || nFaults > 0)
        {
        PK_ENTITY_delete (1, surfaceTag);

        status = ERROR;
        }

    if (faults)
        PK_MEMORY_free (faults);

    return (BentleyStatus) status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/01
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidGeom::CreateSurfaceFromMSBsplineSurface (PK_BSURF_t* surfaceTag, MSBsplineSurfaceCR surface)
    {
    MSBsplineSurfacePtr tmpSurface = MSBsplineSurface::CreatePtr ();

    tmpSurface->CopyFrom (surface);

    return createSurfaceFromMSBsplineSurface (surfaceTag, *tmpSurface);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void     bspsurf_toNaturalParams
(
double*         pOutParams,     // <= natural params between start and end
double*         pInParams,      // => fraction params between 0.0 and 1.0
int             num,            // => size of array
double          start,          // => start of natural range
double          end             // => end of natural range
)
    {
    double      *pIn, *pOut, *pEnd, delta;

    delta = end - start;

    for (pIn = pInParams, pOut = pOutParams, pEnd = pInParams+num; pIn < pEnd; pIn++, pOut++)
        *pOut = *pIn * delta + start;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/97
+---------------+---------------+---------------+---------------+---------------+------*/
static void     addTrimCurveToLoop
(
TrimCurve**     loopPP,
PK_GEOM_t       spaceCurve,
PK_INTERVAL_t   spaceCurveInterval,
PK_SPCURVE_t    spCurve,
PK_INTERVAL_t*  intervalP,
double          tolerance
)
    {
    PK_SPCURVE_sf_t sfSpCurve;

    if (PK_ERROR_no_errors != PK_SPCURVE_ask (spCurve, &sfSpCurve))
        return;

    PK_BCURVE_t     psBCurveTag;
    PK_LOGICAL_t    isExact;

    if (PK_ERROR_no_errors != PK_CURVE_make_bcurve (sfSpCurve.curve, *intervalP, PK_LOGICAL_false, PK_LOGICAL_false, 1.0E-6, &psBCurveTag, &isExact))
        return;

    MSBsplineCurve  curve;

    if (SUCCESS == PSolidGeom::CreateMSBsplineCurveFromBCurve (curve, psBCurveTag))
        bspTrimCurve_allocateAndInsertCyclic (loopPP, &curve);

    PK_ENTITY_delete (1, &psBCurveTag);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/97
* Realloc the boundary array for the surface.
* Insert the TrimCurve chain as the new boundary.
* Note that the boundary does NOT yet have its linear image.
* @param surface IN OUT surface to receive additional boundary loop
* @param trimLoopPP IN OUT handle for head of new curve chain.
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt addLoopToSurface (MSBsplineSurfaceR surface, TrimCurve** trimLoopPP)
    {
    StatusInt   status = ERROR;

    bspTrimCurve_breakCyclicList (trimLoopPP);

    if (NULL != *trimLoopPP)
        {
        int     allocSize = (surface.numBounds + 1) * sizeof(BsurfBoundary);

        if (NULL != (surface.boundaries = surface.numBounds
                      ? (BsurfBoundary*) realloc (surface.boundaries, allocSize)
                      : (BsurfBoundary*) malloc (allocSize)))
            {
            BsurfBoundary   newBoundary;

            memset (&newBoundary, 0, sizeof (newBoundary));
            newBoundary.pFirst = *trimLoopPP;
            surface.boundaries[surface.numBounds] = newBoundary;
            surface.numBounds++;
            *trimLoopPP = NULL;

            status = SUCCESS;
            }
        }

    if (SUCCESS != status)
        bspTrimCurve_freeList (trimLoopPP);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/97
+---------------+---------------+---------------+---------------+---------------+------*/
static void     bspsurf_removePCurveTrim_local (MSBsplineSurfaceR surface)
    {
    for (int i = 0; i < surface.numBounds; i++)
        {
        BsurfBoundary*  boundary = &surface.boundaries[i];

        bspTrimCurve_freeList (&boundary->pFirst);

        boundary->pFirst    = NULL;
        boundary->points    = NULL;
        boundary->numPoints = 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/94
+---------------+---------------+---------------+---------------+---------------+------*/
static double   polygonArea (DPoint2dP points, int nPoints)
    {
    double      zNormal = 0.0;
    DPoint2d    *pointP, *endP, *nextP;

    for (pointP = points, endP = pointP + nPoints - 1; pointP <= endP; pointP++)
        {
        nextP = (pointP == endP) ? points : pointP + 1;
        zNormal += (pointP->x - nextP->x) * (pointP->y + nextP->y);
        }

    return 0.5 * zNormal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/94
+---------------+---------------+---------------+---------------+---------------+------*/
static void     setHoleOrigin (MSBsplineSurfaceR surface)
    {
    if (!surface.numBounds)
        {
        surface.holeOrigin = false;

        return;
        }

    int             i;
    double          minDist, dist;
    DPoint2d        *pointP, *endP;
    BsurfBoundary   *minBoundP = NULL;

    // Find boundary and vertex closest to the origin
    for (i=0, minDist = 1.0E8; i < surface.numBounds; i++)
        {
        for (pointP = surface.boundaries[i].points, endP = pointP + surface.boundaries[i].numPoints - 1; pointP < endP; pointP++)
            {
            if ((dist = sqrt (pointP->x * pointP->x + pointP->y * pointP->y)) < minDist)
                {
                minBoundP = &surface.boundaries[i];
                minDist = dist;
                }
            }
        }

    static double s_zeroAreaTol = 1.0e-12;
    // Determine if the closest boundary is a hole or a solid
    surface.holeOrigin = polygonArea (minBoundP->points, minBoundP->numPoints) > s_zeroAreaTol;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidGeom::CreateBSurfaceFromSurface (PK_BSURF_t& bSurfaceTag, PK_PARAM_sf_t param[2], PK_SURF_t surfTag, bool makeNonPeriodic)
    {
    PK_CLASS_t  surfType = 0;

    // first convert psSurfTag to Parasolid bspline surface
    if (PK_ERROR_no_errors != PK_ENTITY_ask_class (surfTag, &surfType))
        return ERROR;

    if (PK_CLASS_bsurf != surfType)
        {
        PK_UVBOX_t  uvbox;

        if (PK_ERROR_no_errors != PK_SURF_ask_uvbox (surfTag, &uvbox))
            return ERROR;

        // PK_CLASS_plane and PK_CLASS_cyl will be converted to a bspline surf bounded by the design cube and thus unusable. Make it smaller by resizing the uvbox.
        if (PK_CLASS_plane == surfType)
            {
            uvbox.param[0] = 0.2;
            uvbox.param[1] = 0.2;
            uvbox.param[2] = 0.8;
            uvbox.param[3] = 0.8;
            }
        else if (PK_CLASS_cyl == surfType)
            {
            // limit V parameter range only
            uvbox.param[1] = 0.2;
            uvbox.param[3] = 0.8;
            }

        PK_LOGICAL_t    exact = PK_LOGICAL_false;

        if (PK_ERROR_no_errors != PK_SURF_make_bsurf (surfTag, uvbox, PK_LOGICAL_false, PK_LOGICAL_false, 1E-06, &bSurfaceTag, &exact))
            return ERROR;
        }
    else
        {
        if (PK_ERROR_no_errors != PK_ENTITY_copy (surfTag, &bSurfaceTag))
            return ERROR;
        }

    // Convert periodic surface to non-periodic surface in U direction
    PK_SURF_ask_params (bSurfaceTag, param);

    if (!makeNonPeriodic)
        return SUCCESS;

    PK_ERROR_code_t errorCode;

    if (param[0].periodic != PK_PARAM_periodic_no_c)
        {
        // Need to insert start parameter as knot. The last parameter is added automatically
        for (errorCode = 0; !errorCode;)
            errorCode = PK_BSURF_add_u_knot (bSurfaceTag, param[0].range.value[0]);
        }

    // Convert periodic surface to non-periodic surface in V direction
    if (param[1].periodic != PK_PARAM_periodic_no_c)
        {
        // Need to insert start parameter as knot. The last parameter is added automatically
        for (errorCode=0;!errorCode;)
            errorCode = PK_BSURF_add_v_knot (bSurfaceTag, param[1].range.value[0]);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidGeom::CreateMSBsplineSurfaceFromSurface
(
MSBsplineSurfaceR    surface,
PK_SURF_t            surfTag,
PK_SURF_trim_data_t* trimData,
PK_GEOM_t*           spaceCurves,
PK_INTERVAL_t*       interval,
int                  uRules,
int                  vRules,
double               tolerance,
bool                 normalizeSurface
)
    {
    PK_BSURF_t      bSurfaceTag = NULTAG;
    PK_PARAM_sf_t   param[2];

    if (SUCCESS != CreateBSurfaceFromSurface (bSurfaceTag, param, surfTag))
        return ERROR;

    PK_BSURF_sf_t   sfSurface;

    if (PK_ERROR_no_errors != PK_BSURF_ask (bSurfaceTag, &sfSurface))
        {
        PK_ENTITY_delete (1, &bSurfaceTag);

        return ERROR;
        }

    memset (&surface, 0, sizeof (surface));

    // Assign U parameters
    surface.uParams.order    = sfSurface.u_degree + 1;
    surface.uParams.numPoles = sfSurface.n_u_vertices;
    surface.uParams.numKnots = surface.uParams.numPoles - surface.uParams.order;

    // Assign V parameters
    surface.vParams.order    = sfSurface.v_degree + 1;
    surface.vParams.numPoles = sfSurface.n_v_vertices;
    surface.vParams.numKnots = surface.vParams.numPoles - surface.vParams.order;

    surface.uParams.numRules = uRules;
    surface.vParams.numRules = vRules;

    surface.rational = sfSurface.is_rational;
    surface.display.curveDisplay = true;

    if (SUCCESS != surface.Allocate ())
        {
        PK_ENTITY_delete (1, &bSurfaceTag);

        return ERROR;
        }

    int     i, j;
    double  *pPoles = sfSurface.vertex;

    for (i = 0; i < sfSurface.n_u_vertices; i++)
        {
        for (j=0; j < sfSurface.n_v_vertices; j++)
            {
            int     index = i + j * sfSurface.n_u_vertices;

            surface.poles[index].x = *pPoles++;
            surface.poles[index].y = *pPoles++;
            surface.poles[index].z = *pPoles++;

            if (surface.rational)
                surface.weights[index] = *pPoles++;
            }
        }

    extractKnots (surface.uKnots, &surface.uParams, sfSurface.n_u_knots, sfSurface.u_knot_mult, sfSurface.u_knot, param[0].periodic != PK_PARAM_periodic_no_c);
    extractKnots (surface.vKnots, &surface.vParams, sfSurface.n_v_knots, sfSurface.v_knot_mult, sfSurface.v_knot, param[1].periodic != PK_PARAM_periodic_no_c);

    int     uPoles = surface.uParams.numPoles;
    int     vPoles = surface.vParams.numPoles;
    double  uKnot0, uKnot1, vKnot0, vKnot1;

    // TR#120843: save off natural knot ranges before removeSurfaceKnots normalizes the knots (we'll use these ranges to prepare for subsequent surface normalization)
    uKnot0 = vKnot0 = 0.0;
    uKnot1 = vKnot1 = 1.0;

    if ((uPoles > MAX_U_NUM_APPLY_REDUCE || vPoles > MAX_V_NUM_APPLY_REDUCE || (uPoles > MAX_UV_NUM_APPLY_REDUCE && vPoles > MAX_UV_NUM_APPLY_REDUCE)))
        {
        int     order = surface.uParams.order;
        int     nFullKnots = bspknot_numberKnots (uPoles, order, surface.uParams.closed);

        uKnot0 = surface.uKnots[order - 1];
        uKnot1 = surface.uKnots[nFullKnots - order];

        order = surface.vParams.order;
        nFullKnots = bspknot_numberKnots (vPoles, order, surface.vParams.closed);
        vKnot0 = surface.vKnots[order - 1];
        vKnot1 = surface.vKnots[nFullKnots - order];
        }

    // compress BEFORE we add boundaries b/c our NLIB wrapper strips them!
    if (uPoles > MAX_U_NUM_APPLY_REDUCE || (uPoles > MAX_UV_NUM_APPLY_REDUCE && vPoles > MAX_UV_NUM_APPLY_REDUCE))
        surface.RemoveKnotsBounded (1, tolerance);

    if (vPoles > MAX_V_NUM_APPLY_REDUCE || (uPoles > MAX_UV_NUM_APPLY_REDUCE && vPoles > MAX_UV_NUM_APPLY_REDUCE))
        surface.RemoveKnotsBounded (2, tolerance);

    // TR#120843: removeSurfaceKnots normalized our knots, but this prevents normalizeSurface from normalizing any boundary points! Thus we first unnormalize the knots. -DA4
    if (uKnot0 != 0.0 || uKnot1 != 1.0)
        {
        int     nFullUKnots = bspknot_numberKnots (surface.uParams.numPoles, surface.uParams.order, surface.uParams.closed);

        bspsurf_toNaturalParams (surface.uKnots, surface.uKnots, nFullUKnots, uKnot0, uKnot1);
        }

    if (vKnot0 != 0.0 || vKnot1 != 1.0)
        {
        int     nFullVKnots = bspknot_numberKnots (surface.vParams.numPoles, surface.vParams.order, surface.vParams.closed);

        bspsurf_toNaturalParams (surface.vKnots, surface.vKnots, nFullVKnots, vKnot0, vKnot1);
        }

    // Extract trim boundaries...if they exist
    if (NULL != trimData)
        {
        int         currentLoopTag = -1;
        TrimCurve*  trimLoopP = NULL;

        for (i=0; i < trimData->n_spcurves; i++)
            {
            if (trimData->trim_loop[i] != currentLoopTag)
                addLoopToSurface (surface, &trimLoopP);

            currentLoopTag = trimData->trim_loop[i];
            addTrimCurveToLoop (&trimLoopP, spaceCurves ? spaceCurves[i] : NULTAG, interval ? interval[i] : trimData->intervals[i], trimData->spcurves[i], &trimData->intervals[i], tolerance);
            }

        addLoopToSurface (surface, &trimLoopP);
        }

    DRange3d    poleRange;

    surface.GetPoleRange (poleRange);

    double          surfaceSize = poleRange.low.Distance (poleRange.high);
    double          surfaceTol = tolerance;
    static double   s_CurveTol = 0.01;
    static double   s_SurfaceMinRelTol = 1.0e-4;
    static double   s_SurfaceMaxRelTol = 5.0e-2;
    static int      s_removePCurveTrim = 0;

    if (surfaceTol < s_SurfaceMinRelTol * surfaceSize)
        surfaceTol = s_SurfaceMinRelTol * surfaceSize;

    if (surfaceTol > s_SurfaceMaxRelTol * surfaceSize)
        surfaceTol = s_SurfaceMaxRelTol * surfaceSize;

    if (normalizeSurface)
        bspsurf_normalizeSurface (&surface);

    bspsurf_restrokeTrimLoops (&surface, s_CurveTol, surfaceTol);

    if (s_removePCurveTrim)
        bspsurf_removePCurveTrim_local (surface);

    setHoleOrigin (surface);

    PK_ENTITY_delete (1, &bSurfaceTag);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidGeom::CreateMSBsplineSurfaceFromFace
(
MSBsplineSurfaceR   surface,
PK_FACE_t           faceTag,
int                 uRules,
int                 vRules,
double              tolerance
)
    {
    PK_SURF_t       surfaceTag;
    PK_LOGICAL_t    orientation;

    if (PK_ERROR_no_errors != PK_FACE_ask_oriented_surf (faceTag, &surfaceTag, &orientation))
        return ERROR;

    PK_FACE_output_surf_trimmed_o_t options;

    PK_FACE_output_surf_trimmed_o_m (options);

    options.want_geoms  = PK_LOGICAL_true;
    options.want_topols = PK_LOGICAL_false;

    PK_CLASS_t      surfaceClass = 0;

    PK_ENTITY_ask_class (surfaceTag, &surfaceClass);

    if (PK_CLASS_bsurf == surfaceClass) // no approximation needed if the surface is already a Bspline surface - Lu
        options.trim_surf = PK_FACE_trim_surf_own_c;
    else
        options.trim_surf = PK_FACE_trim_surf_bsurf_c;

    options.curve_tolerance = tolerance;
    options.surf_tolerance = 0.1 * tolerance;

    PK_SURF_t           bSurfaceTag;
    PK_LOGICAL_t        sense;
    PK_SURF_trim_data_t trimData;
    PK_GEOM_t*          geometryP = NULL;
    PK_INTERVAL_t*      intervalP = NULL;
    PK_TOPOL_t*         topologyP = NULL;

    if (PK_ERROR_no_errors != PK_FACE_output_surf_trimmed (faceTag, &options, &bSurfaceTag, &sense, &trimData, &geometryP, &intervalP, &topologyP))
        return ERROR;

    bool    isValid = false;

    if (SUCCESS == PSolidGeom::CreateMSBsplineSurfaceFromSurface (surface, bSurfaceTag, &trimData, geometryP, intervalP, uRules, vRules, tolerance))
        {
        if (PK_LOGICAL_false == orientation)
            surface.MakeReversed (0);

        isValid = true;
        }

    PK_ENTITY_delete (1, &bSurfaceTag);

    if (trimData.n_spcurves)
        {
        PK_ENTITY_delete (trimData.n_spcurves, trimData.spcurves);

        PK_MEMORY_free (trimData.spcurves);
        PK_MEMORY_free (trimData.intervals);
        PK_MEMORY_free (trimData.trim_loop);
        PK_MEMORY_free (trimData.trim_set);
        }

    if (geometryP)
        {
        PK_ENTITY_delete (trimData.n_spcurves, geometryP);
        PK_MEMORY_free (geometryP);
        }

    if (intervalP)
        PK_MEMORY_free (intervalP);

    return (isValid ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Youli.Bai       10/2007
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt repairFace (PK_FACE_t faceTag)
    {
    PK_FACE_repair_o_t  repairOption;
    PK_TOPOL_track_r_t  repairInfo;

    PK_FACE_repair_o_m (repairOption);

    memset (&repairInfo, 0, sizeof (repairInfo));
    StatusInt   status = PK_FACE_repair (faceTag, &repairOption, &repairInfo);

    PK_TOPOL_track_r_f (&repairInfo);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Youli.Bai       10/2007
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt repairEdge (PK_EDGE_t edgeTag)
    {
    PK_EDGE_repair_o_t  repairOption;
    PK_TOPOL_track_r_t  repairInfo;

    PK_EDGE_repair_o_m (repairOption);

    repairOption.max_tolerance = 1.0E-5; // for now, we just use this static tolerence for edge repairing.

    memset (&repairInfo, 0, sizeof (repairInfo));
    StatusInt   status = PK_EDGE_repair (1, &edgeTag, &repairOption, &repairInfo);

    PK_TOPOL_track_r_f (&repairInfo);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Nikolay.Shulga  03/98
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidUtil::FixupNonG1BodyGeometry (PK_BODY_t bodyTag)
    {
    PK_LOGICAL_t    continuityCheckFlag = PK_LOGICAL_false;

    // note that we need to reset continuity check to true, whatever it was
    PK_SESSION_ask_check_continuity (&continuityCheckFlag);
    PK_SESSION_set_check_continuity (PK_LOGICAL_true);

    // repair all edges first
    int         cntOldEdges = 0;
    PK_EDGE_t*  pOldEdges = NULL;

    if (PK_ERROR_no_errors == PK_BODY_ask_edges (bodyTag, &cntOldEdges, &pOldEdges) && NULL != pOldEdges)
        {
        for (int i = 0; i < cntOldEdges; i++)
            repairEdge (pOldEdges[i]);
        }

    PK_MEMORY_free (pOldEdges);

    // repair all faces at last
    int         cntOldFaces = 0;
    PK_FACE_t*  pOldFaces = NULL;

    if (PK_ERROR_no_errors == PK_BODY_ask_faces (bodyTag, &cntOldFaces, &pOldFaces) && NULL != pOldFaces)
        {
        for (int i = 0; i < cntOldFaces; i++)
            repairFace (pOldFaces[i]);
        }

    PK_MEMORY_free (pOldFaces);

    PK_SESSION_set_check_continuity (continuityCheckFlag);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Nikolay.Shulga  03/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int      countBoundaryCurves (int** ppCurveSets, int boundCnt)
    {
    if (NULL == ppCurveSets)
        return 0;

    int         cnt = 0;
    int*        pCurveSet;

    for (int i = 0; i < boundCnt; i++)
        {
        pCurveSet = ppCurveSets[i];

        if (NULL != pCurveSet)
            {
            while (0 != (*pCurveSet))
                {
                ++cnt;
                ++pCurveSet;
                }
            }
        }

    return cnt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Nikolay.Shulga  03/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void     defaultTrimDataRange (PK_SURF_trim_data_t* pTrimData)
    {
    if (NULL == pTrimData)
        return;

    PK_SPCURVE_sf_t spCurveSF;

    /* set parametric intervals in trim data to full range for each sp-curve */
    for (int i = 0; i < pTrimData->n_spcurves; i++)
        {
        if (SUCCESS != PK_SPCURVE_ask (pTrimData->spcurves[i], &spCurveSF))
            continue;

        if (SUCCESS == PK_CURVE_ask_interval (spCurveSF.curve, pTrimData->intervals + i))
            continue;

        // something's wrong - can't get parametric interval. Ignore the whole thing (make_sheet_trimmed will *gracefully* choke on this)
        pTrimData->intervals[i].value[0] = 0.0;
        pTrimData->intervals[i].value[1] = 0.0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Nikolay.Shulga  03/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int      testTrimData (PK_SURF_trim_data_t* pTrimData)
    {
    // trim data is a collection of sp_curves
    if (NULL != pTrimData && 
        NULL != pTrimData->spcurves && 
        NULL != pTrimData->intervals && 
        NULL != pTrimData->trim_loop &&
        NULL != pTrimData->trim_set)
        return PK_ERROR_no_errors;

    return !PK_ERROR_no_errors;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Nikolay.Shulga  03/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int      allocTrimData (PK_SURF_trim_data_t* pTrimData, int size)
    {
    pTrimData->spcurves  = (PK_CURVE_t*) pki_calloc (size, sizeof (*(pTrimData->spcurves)));
    pTrimData->intervals = (PK_INTERVAL_t*) pki_calloc (size, sizeof (*(pTrimData->intervals)));
    pTrimData->trim_loop = (int*) pki_calloc (size, sizeof (*(pTrimData->trim_loop)));
    pTrimData->trim_set  = (int*) pki_calloc (size, sizeof (*(pTrimData->trim_set)));

    return testTrimData (pTrimData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Nikolay.Shulga  03/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int      reallocTrimData (PK_SURF_trim_data_t* pTrimData, int oldCount, int newCount)
    {
    void*       pMemory;
    int         bytesPerRecord;

    bytesPerRecord = sizeof (*(pTrimData->spcurves));

    if (!(pMemory = pki_realloc (pTrimData->spcurves, oldCount * bytesPerRecord, newCount * bytesPerRecord)))
        return PK_ERROR_memory_full;

    pTrimData->spcurves = (PK_CURVE_t*) pMemory;

    bytesPerRecord = sizeof (*(pTrimData->intervals));

    if (!(pMemory = pki_realloc (pTrimData->intervals, oldCount * bytesPerRecord, newCount * bytesPerRecord)))
        return PK_ERROR_memory_full;

    pTrimData->intervals = (PK_INTERVAL_t*) pMemory;

    bytesPerRecord = sizeof (*(pTrimData->trim_loop));

    if (!(pMemory = pki_realloc (pTrimData->trim_loop, oldCount * bytesPerRecord, newCount * bytesPerRecord)))
        return PK_ERROR_memory_full;

    pTrimData->trim_loop = (int*) pMemory;

    bytesPerRecord = sizeof (*(pTrimData->trim_set));

    if (!(pMemory = pki_realloc (pTrimData->trim_set, oldCount * bytesPerRecord, newCount * bytesPerRecord)))
        return PK_ERROR_memory_full;

    pTrimData->trim_set = (int*) pMemory;

    return testTrimData (pTrimData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Nikolay.Shulga  03/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void     freeTrimData (PK_SURF_trim_data_t* pTrimData)
    {
    if (NULL != pTrimData->spcurves)
        {
        PK_ENTITY_delete (pTrimData->n_spcurves, pTrimData->spcurves);
        PK_MEMORY_free (pTrimData->spcurves);
        }

    if (NULL != pTrimData->intervals)
        PK_MEMORY_free (pTrimData->intervals);

    if (NULL != pTrimData->trim_loop)
        PK_MEMORY_free (pTrimData->trim_loop);

    if (NULL != pTrimData->trim_set)
        PK_MEMORY_free (pTrimData->trim_set);

    pTrimData->spcurves  = NULL;
    pTrimData->intervals = NULL;
    pTrimData->trim_loop = NULL;
    pTrimData->trim_set  = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Nikolay.Shulga  03/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int      updateTrimData
(
PK_SURF_trim_data_t *pTrimData,
int                 *pSPCurveCnt,       // <=> occupied size
int                 *pSPCArraySize,     // <=> allocated size
int                 tempSPCCnt,
int                 *pTempSPCTags,
int                 loopCnt
)
    {
    if (0 == tempSPCCnt || NULL == pTempSPCTags)
        return ERROR;

    int         spcArrayIncr = 10; // if spcArraySize is not enough, anticipate more

    // check if the total # of curves will be greater than spcArraySize; realloc, etc. as necessary
    if (*pSPCurveCnt + tempSPCCnt >= *pSPCArraySize)
        {
        *pSPCArraySize = spcArrayIncr + tempSPCCnt + *pSPCurveCnt;

        if (PK_ERROR_no_errors != reallocTrimData (pTrimData, *pSPCurveCnt, *pSPCArraySize))
            return ERROR;
        }

    for (int k = 0; k < tempSPCCnt; k++)
        {
        pTrimData->spcurves[*pSPCurveCnt] = pTempSPCTags[k];
        pTrimData->trim_loop[*pSPCurveCnt] = loopCnt;
        pTrimData->trim_set[*pSPCurveCnt] = 1; // Parasolid allows only one set...whatever it means
        ++(*pSPCurveCnt);
        }

    return PK_ERROR_no_errors;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Nikolay.Shulga  03/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int      trimSurfaceBySPCurves
(
PK_BODY_t*              pBodyTag,
PK_SURF_trim_data_t*    pTrimData,
int                     surfaceTag,
double                  tolerance,
int                     adaptive
)
    {
    // we should have a valid surface and some curves...at least
    if (0 == surfaceTag || NULL == pTrimData)
        return ERROR;

    PK_check_state_t                state = 0;
    PK_LOGICAL_t                    continuityCheckFlag = PK_LOGICAL_false;
    PK_ERROR_code_t                 status = PK_ERROR_no_errors;
    PK_SURF_make_sheet_trimmed_o_t  trimOptions;

    PK_SURF_make_sheet_trimmed_o_m (trimOptions);

    trimOptions.check_loops     = PK_LOGICAL_true;
    trimOptions.check_self_int  = PK_LOGICAL_true;
    trimOptions.check_wires     = PK_LOGICAL_true;

    // save old continuity check value, set current mode to false
    PK_SESSION_ask_check_continuity (&continuityCheckFlag);
    PK_SESSION_set_check_continuity (PK_LOGICAL_false);

    status = PK_SURF_make_sheet_trimmed (surfaceTag, *pTrimData, tolerance, &trimOptions, pBodyTag, &state);

    if (PK_ERROR_bad_tolerance == status && 0 != adaptive)
        {
        // tolerance too tight, try again with 2*tolerance
        status = PK_SURF_make_sheet_trimmed (surfaceTag, *pTrimData, 2 * tolerance, &trimOptions, pBodyTag, &state);

        if (PK_ERROR_bad_tolerance == status)
            {
            // tolerance still too tight, try again with 10 * tolerance
            status = PK_SURF_make_sheet_trimmed (surfaceTag, *pTrimData, 10 * tolerance, &trimOptions, pBodyTag, &state);
            }
        }

    if (PK_ERROR_no_errors == status)
        {
        if (0 != *pBodyTag)
            status = PSolidUtil::FixupNonG1BodyGeometry (*pBodyTag);
        else
            status = state;
        }

    PK_SESSION_set_check_continuity (continuityCheckFlag);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Nikolay.Shulga  03/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int      makeSPCurvesFromSpaceCurve
(
PK_SPCURVE_t    **ppSPCurves,
int             *pCntSPCurves,
PK_CURVE_t      spaceCurve,
PK_INTERVAL_t   range,
PK_SURF_t       surface,
double          tolerance,
int             adaptive
)
    {
    if (0 == spaceCurve)
        return ERROR;

    int         status = PK_CURVE_make_spcurves (spaceCurve, range, surface, true, true, tolerance, pCntSPCurves, ppSPCurves);

    if (PK_ERROR_tolerances_too_tight == status && 0 != adaptive)
        {
        // try again, tolerance *= 10...
        status = PK_CURVE_make_spcurves (spaceCurve, range, surface, true, true, 10 * tolerance, pCntSPCurves, ppSPCurves);

        if (PK_ERROR_tolerances_too_tight == status)
            {
            // try again, tolerance *= 100...
            status = PK_CURVE_make_spcurves (spaceCurve, range, surface, true, true, 100 * tolerance, pCntSPCurves, ppSPCurves);
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Nikolay.Shulga  02/98
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidGeom::CreateSheetBodyFromTrimmedSurface
(
PK_ENTITY_t*    bodyTagP,
PK_ENTITY_t**   spaceCurveEntitiesPP,
PK_ENTITY_t**   uvCurveEntitiesPP,
int             preferSpaceCurvesFlag,
double**        trimPP,
int             boundCount,
PK_ENTITY_t     surfaceTag,
double          tolerance,
int             adaptive
)
    {
    /*  This function takes a surface and a set of trim boundaries and
        tries to create a sheet body out of these.
        Each boundary may be represented either as uv_curve set, or
        as a space curve set + (optional) trim parameters.
        uv curve boundary is always tried first. If this does not succeed,
        uv_curve boundary is NULL, or ppUVCurveEntities is NULL,
        space curve boundary is used. If none are present, next one is processed
        up to boundCount.

        Both ppUVCurveEntities and ppSpaceCurveEntities are either NULL or hold
        boundCount pointers, some of them NULL.

        tolerance is used in generating sp_curves and the sheet body.
        If adaptive is non-zero and Parasolid returns 'tolerance too tight' error,
        10 * tolerance and if that fails then 100 * tolerance is tried.
    */
    if (0 == surfaceTag)
        return ERROR;

    PK_LOGICAL_t    continuityCheckFlag = PK_LOGICAL_false;

    // save old continuity check value, set current mode to false
    PK_SESSION_ask_check_continuity (&continuityCheckFlag);
    PK_SESSION_set_check_continuity (PK_LOGICAL_false);

    int             rollback = 0;

    // set mark; if screwed up, roll back
    PK_MARK_create  (&rollback);

    /*  Generate sp_curves. They have to be stored in trimData.spcurves
        We don't know how many curves will there be. Allocate something to begin with;
        while generating spcurves, check if the spcurves array is full,
        realloc if needed. See Parasolid docs for data structure description.
        Allocate twice as many as there's space curves - overestimate a little
        so that don't need to realloc too many times.
    */
    int                 spcArraySize = 2 * (countBoundaryCurves (spaceCurveEntitiesPP, boundCount) + countBoundaryCurves (uvCurveEntitiesPP, boundCount));
    PK_SURF_trim_data_t trimData;

    memset (&trimData, 0, sizeof (trimData));

    PK_ERROR_code_t status = PK_ERROR_no_errors;
    int         failedFlag = 0;
    int         spCurveCnt = 0;

    if (PK_ERROR_no_errors != allocTrimData (&trimData, spcArraySize))
        {
        goto clean_up;
        }

    for (int boundIndex = 0; boundIndex < boundCount; boundIndex++)
        {
        int*    pSpaceCurveSet = NULL;
        int*    pUVCurveSet = NULL;

        if (NULL != spaceCurveEntitiesPP)
            pSpaceCurveSet = spaceCurveEntitiesPP[boundIndex];
        else
            pSpaceCurveSet = NULL;

        if (NULL != uvCurveEntitiesPP)
            pUVCurveSet = uvCurveEntitiesPP[boundIndex];
        else
            pUVCurveSet = NULL;

        /*  Try 2 times.
            If preferSpaceCurvesFlag is set, then first try with pSpaceCurveSet. Then try with pUVCurveSet.
            If preferSpaceCurvesFlag is not set, then first try with pUVCurveSet. Then try with pSpaceCurveSet.
        */
        failedFlag = 1;

        for (int tryIndex = 0; tryIndex < 2 && failedFlag; tryIndex++)
            {
            int         tempSPCCnt = 0;
            int*        pTempSPCTags = NULL;

            if (NULL != pSpaceCurveSet && ((preferSpaceCurvesFlag && 0 == tryIndex) || (!preferSpaceCurvesFlag && 1 == tryIndex)))
                {
                failedFlag = 0;

                for (int curveIndex = 0; 0 != pSpaceCurveSet[curveIndex]; curveIndex++)
                    {
                    PK_INTERVAL_t   range;

                    tempSPCCnt = 0;
                    pTempSPCTags = NULL;

                    if (NULL != trimPP && NULL != trimPP[boundIndex])
                        {
                        range.value[0] = *(trimPP[boundIndex] + 2* curveIndex);
                        range.value[1] = *(trimPP[boundIndex] + 2* curveIndex + 1);
                        }
                    else
                        {
                        PK_CURVE_ask_interval (pSpaceCurveSet[curveIndex], &range);
                        }

                    if ((PK_ERROR_no_errors != (status = makeSPCurvesFromSpaceCurve (&pTempSPCTags, &tempSPCCnt, pSpaceCurveSet[curveIndex], range, surfaceTag, tolerance, adaptive))) ||
                        (PK_ERROR_no_errors != (status = updateTrimData (&trimData, &spCurveCnt, &spcArraySize, tempSPCCnt, pTempSPCTags, boundIndex + 1))))
                        {
                        failedFlag = 1;
                        break;
                        }
                    }

                if (NULL != pTempSPCTags)
                    PK_MEMORY_free (pTempSPCTags);
                }

            if (NULL != pUVCurveSet && ((preferSpaceCurvesFlag && 1 == tryIndex) || (!preferSpaceCurvesFlag && 0 == tryIndex)))
                {
                failedFlag = 0;

                for (int curveIndex = 0; 0 != pUVCurveSet[curveIndex]; curveIndex++)
                    {
                    if ((PK_ERROR_no_errors != (status = PK_CURVE_embed_in_surf (pUVCurveSet[curveIndex], surfaceTag, &tempSPCCnt, &pTempSPCTags))) ||
                        (PK_ERROR_no_errors != (status = updateTrimData (&trimData, &spCurveCnt, &spcArraySize, tempSPCCnt, pTempSPCTags, boundIndex + 1))))
                        {
                        failedFlag = 1;
                        break;
                        }
                    }
                }
            }

        // If after 2 tries, failedFlag is 1, then go to cleanup
        if (failedFlag)
            goto clean_up;
        }

    trimData.n_spcurves = spCurveCnt;
    defaultTrimDataRange (&trimData);

    status = trimSurfaceBySPCurves (bodyTagP, &trimData, surfaceTag, tolerance, adaptive);

clean_up:

    if (PK_ERROR_no_errors != status)
        {
        PK_MARK_goto (rollback); // failed somewhere, rollback
        }
    else
        {
        int                 nFaults;
        PK_check_fault_t*   faultsP = NULL;
        PK_BODY_check_o_t   checkOptions;

        PK_BODY_check_o_m (checkOptions);

        checkOptions.max_faults = 0;

        if (PK_ERROR_no_errors != (status = PK_BODY_check (*bodyTagP, &checkOptions, &nFaults, &faultsP)))
            {
            PK_ENTITY_delete (1, bodyTagP);
            }
        else
            {
            int     nGeoms = 0;

            status = PK_BODY_simplify_geom (*bodyTagP, PK_LOGICAL_false, &nGeoms, NULL);
            }
        }

    PK_SESSION_set_check_continuity (continuityCheckFlag); // restore old continuity check settings

    freeTrimData (&trimData);

    PK_MARK_delete (rollback);

    return (BentleyStatus) status;
    }

/*---------------------------------------------------------------------------------**//**
* function angleBetweenVectors
* Returns the angle between two vectors. This angle is between 0 and pi. Rotating the first vector by this angle around the cross product
* between the vectors aligns it with the second vector.
* @bsimethod                                                    Earlin.Lutz     07/96
+---------------+---------------+---------------+---------------+---------------+------*/
static double   angleBetweenVectors
(
DPoint3dP       pVector1,       // => first vector
DPoint3dP       pVector2        // => second vector
)
    {
    DPoint3d    crossProduct;
    double      cross, dot;

    crossProduct.CrossProduct (*pVector1, *pVector2);

    cross = crossProduct.Magnitude ();
    dot   = pVector1->DotProduct (*pVector2);

    if (cross == 0.0 && dot == 0.0)
        return 0.0;

    return (atan2 (cross, dot));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          03/98
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     pointIsCorner
(
DPoint2d        *pPt,       // => a point in a bspline surface trim boundar
DPoint3d        *pNext,     // => vector
DPoint3d        *pPrev,     // => vector
int             edgeCodeN,
int             edgeCode,
int             edgeCodeP
)
    {
    double      lengthP, lengthN;

    if (edgeCode)
        {
        // Lie on the same edge, consider it is smooth point
        if (edgeCodeP & edgeCode & edgeCodeN)
            return false;
        else if (edgeCodeP || edgeCodeN)
            return true;
        }

    // Check the adacent segments length
    lengthN = pNext->Magnitude ();
    lengthP = pPrev->Magnitude ();

    if (lengthN > TOLERANCE_UVSpaceLength || lengthP > TOLERANCE_UVSpaceLength)
        return true;

    return (angleBetweenVectors (pNext, pPrev) > TOLERANCE_UVSpaceAngle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          03/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void     findAllCorners
(
int             *pCorners,
int             *pNumCorners,
DPoint2d        *pPts,
int             num,
double          angle
)
    {
    int         edgeCodeN, edgeCodeP, edgeCode, numCorners, i;
    DPoint3d    deltaN, deltaP;

    deltaN.z = deltaP.z = 0.0;
    deltaP.x = pPts[0].x - pPts[num-2].x;
    deltaP.y = pPts[0].y - pPts[num-2].y;
    edgeCode = bsputil_edgeCode (&pPts[0], 0.0);
    edgeCodeP = bsputil_edgeCode (&pPts[num-2], 0.0);

    for (i = numCorners = 0; i < num - 1; i++)
        {
        deltaN.x = pPts[i+1].x - pPts[i].x;
        deltaN.y = pPts[i+1].y - pPts[i].y;
        edgeCodeN = bsputil_edgeCode (&pPts[i+1], 0.0);

        if (true == pointIsCorner (&pPts[i], &deltaN, &deltaP, edgeCodeN, edgeCode, edgeCodeP))
            pCorners[numCorners++] = i;

        deltaP = deltaN;
        edgeCodeP = edgeCode;
        edgeCode = edgeCodeN;
        }

    if (pNumCorners)
        *pNumCorners = numCorners;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          03/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int      createFittingCurve
(
MSBsplineCurveP pCurve,     // <=
DPoint3dP       pPts,       // => 
int             num,        // =>
double          tolerance,  // => least square tolerance
bool            smoothEnd   // => periodic ends
)
    {
    bool        onLine;

    /* Step 1: See whether points lie on a line */
    if (num <= 2)
        onLine = true;
    else
        bsiGeom_pointArrayColinearTest (&onLine, pPts, num, TOLERANCE_UVSpaceLinear);

    if (onLine)
        {
        DPoint3d    points[2];

        points[0] = pPts[0];
        points[1] = pPts[num-1];

        return pCurve->InitFromPoints (points, 2);
        }

    /* Step 2: there are <= 200 points, use interpolation and data reduction */
    if (num <= 200)
        {
        if (SUCCESS == bspcurv_c2CubicInterpolateCurve (pCurve, pPts, NULL, num, false, 1.0e-8, NULL, smoothEnd))
            return pCurve->RemoveKnotsBounded (tolerance, false, false);
        }

    DVec3d      sTan, eTan;

    /* Step 3: there are > 200 points, use least square approximation  */
    if (smoothEnd)
        {
        sTan.DifferenceOf (pPts[1], pPts[num-2]);
        eTan = sTan;
        sTan.Normalize ();
        eTan.Normalize ();
        }

    int         status;

    if (SUCCESS == (status = pCurve->InitFromLeastSquaresFit (pPts, num, true, smoothEnd ? &sTan : NULL, smoothEnd ? &eTan : NULL, false, 2, 3, true, tolerance)))
        {
        if (pCurve->params.numPoles > 90)
            {
            tolerance *= 10.0;

            status = pCurve->RemoveKnotsBounded (tolerance, false, false);
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          03/98
+---------------+---------------+---------------+---------------+---------------+------*/
PUSH_MSVC_IGNORE(6385 6386) // static analysis thinks we exceed the bounds of pCorners... I don't see how.
// It also thinks pPoints is exceeded... that's harder to disprove, but adding gratuitous checks feels wrong...
static int      fitBoundaryToG1Curves
(
MSBsplineCurve  **ppCrvs,   /* <= */
int             *pNumCrvs,  /* <= */
DPoint2d        *pPts,      /* => bspline surface boundary, 1st point is same as last point */
int             num,        /* => num points in the boundary */
double          angle,      /* => angle to determine whether a point is a sharp corner */
double          tolerance   /* => tolerance used in the least square fitting */
)
    {
    int         status = SUCCESS;
    bool        smoothEnd = false;
    int         i, j, tmp, numPts, *pCorners = NULL, numCorners;
    int         numCurvesOut;
    DPoint3d    *pPoints = NULL;

    /* pCurners used to query which points are corners so to break into seperate curves */
    if (ppCrvs == NULL || pNumCrvs == NULL || num < 2 ||
        NULL == (pCorners = (int *) malloc (num * sizeof (int))) ||
        NULL == (pPoints = (DPoint3dP) malloc (num * sizeof (DPoint3d))))
        return  ERROR;

    /* Step 1: Find all sharp corner points */
    findAllCorners (pCorners, &numCorners, pPts, num, angle);

    /* Step 2: Fit a curve bewteen corners if any */
    if (numCorners == 0)
        {
        numCorners  = 1;
        pCorners[0] = 0;
        smoothEnd   = true;
        }

    *ppCrvs = (MSBsplineCurve*) malloc (numCorners * sizeof (MSBsplineCurve));
    numCurvesOut = 0;

    for (i = 0; i < numCorners; i++)
        {
        numPts = (i == numCorners - 1) ? num - pCorners[numCorners-1] + pCorners[0] :
        pCorners[i+1] - pCorners[i] + 1;

        if (i != numCorners - 1)
            {
            for (j = 0; j < numPts; j++)
                {
                pPoints[j].x = pPts[pCorners[i]+j].x;
                pPoints[j].y = pPts[pCorners[i]+j].y;
                pPoints[j].z = 0.0;
                }
            }
        else
            {
            tmp = num - pCorners[i];

            for (j = 0; j < tmp; j++)
                {
                pPoints[j].x = pPts[pCorners[i]+j].x;
                pPoints[j].y = pPts[pCorners[i]+j].y;
                pPoints[j].z = 0.0;
                }

            for (j = 0; j < pCorners[0]; j++)
                {
                pPoints[j+tmp].x = pPts[j+1].x;
                pPoints[j+tmp].y = pPts[j+1].y;
                pPoints[j+tmp].z = 0.0;
                }
            }

        if (SUCCESS == createFittingCurve (&((*ppCrvs)[numCurvesOut]), pPoints, numPts, tolerance, smoothEnd))
            numCurvesOut++;
        }

    *pNumCrvs = numCurvesOut;

    if (pCorners)
        free (pCorners);

    if (pPoints)
        free (pPoints);

    return status;
    }
POP_MSVC_IGNORE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          10/92
+---------------+---------------+---------------+---------------+---------------+------*/
PUSH_MSVC_IGNORE(6386) // static analysis thinks we exceed the bounds of uvCurvesPP[index] in the last else in this function... I don't see how.
static int      createUVCurves
(
PK_ENTITY_t**   uvCurvesPP,
int             index,
BsurfBoundary*  pBoundary,
bool            fit,
double          angle,
double          tolerance
)
    {
    StatusInt   status = SUCCESS;
    int         numCurves;

    if (NULL != pBoundary->pFirst)
        {
        int         i;
        TrimCurve*  pTrim;

        for (numCurves = 0, pTrim = pBoundary->pFirst; NULL != pTrim; pTrim = pTrim->pNext)
            numCurves++;

        uvCurvesPP[index] = (PK_ENTITY_t *) malloc ((numCurves + 1) * sizeof (**uvCurvesPP));

        for (i=0, pTrim = pBoundary->pFirst; NULL != pTrim; pTrim = pTrim->pNext, i++)
            PSolidGeom::CreateCurveFromMSBsplineCurve2d (&uvCurvesPP[index][i], pTrim->curve);

        uvCurvesPP[index][numCurves] = 0;
        }
   else
        {
        int         num  = pBoundary->numPoints;
        DPoint2dP   pPts = pBoundary->points;

        if (num <= 1)
            {
            uvCurvesPP[index] = NULL;

            status = ERROR;
            }
        else if (fit)
            {
            MSBsplineCurveP curvesP = NULL;

            if (SUCCESS == (status = fitBoundaryToG1Curves (&curvesP, &numCurves, pPts, num, angle, tolerance)))
                {
                uvCurvesPP[index] = (PK_ENTITY_t *) malloc ((numCurves + 1) * sizeof (**uvCurvesPP));

                for (int i=0; i < numCurves; i++)
                    {
                    PSolidGeom::CreateCurveFromMSBsplineCurve2d (&uvCurvesPP[index][i], curvesP[i]);

                    curvesP[i].ReleaseMem ();
                    }

                uvCurvesPP[index][numCurves] = 0;

                if (curvesP)
                    free (curvesP);
                }
            else
                {
                uvCurvesPP[index] = NULL;
                }
            }
        else
            {
            int     i;

            uvCurvesPP[index] = (PK_ENTITY_t *) malloc (num * sizeof (**uvCurvesPP));

            for (i=0; i < num - 1; i++)
                {
                DPoint3d        points[2];
                MSBsplineCurve  curve;

                points[0].Init (pPts[i].x, pPts[i].y, 0.0);
                points[1].Init (pPts[i+1].x, pPts[i+1].y, 0.0);
                curve.InitFromPoints (points, 2);

                PSolidGeom::CreateCurveFromMSBsplineCurve2d (&uvCurvesPP[index][i], curve);

                curve.ReleaseMem ();
                }

            uvCurvesPP[index][i] = 0;
            }
        }

    return status;
    }
POP_MSVC_IGNORE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          03/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void     createTrimmedSheetBodyList
(
TAG_LIST&       tagList,
BsurfBoundary*  boundsP,        // => bspline surface boundaries from VU graph
int*            parentIdP,      // => parent ids
int             num,            // => bounds array size
PK_BSURF_t      surfaceTag,     // => bspline surface tag
bool            fit,            // => whether to fit with smooth curves
double          angleTol,       // => used to determine whether a UV point is a corner
double          fitTol          // => used to fit in UV space
)
    {
    int         startId = (NULL == parentIdP) ? 0 : parentIdP[0];
    int baseBoundaryIndexForCurrentBody = 0;

    for (int i = 0, numLoopsInCurrentBody = 0; i < num; i++)
        {
        int     nextId = (i < num - 1) ? (NULL == parentIdP ? 0 : parentIdP[i+1]) : -1;

        numLoopsInCurrentBody++;

        if (startId == nextId)
            continue;

        PK_BSURF_t      surfTag;

        PK_ENTITY_copy (surfaceTag, &surfTag);

        PK_ENTITY_t**   uvCurvesPP = (PK_ENTITY_t **) malloc (numLoopsInCurrentBody * sizeof (*uvCurvesPP));

        for (int j = 0; j < numLoopsInCurrentBody; j++)
            createUVCurves (uvCurvesPP, j, boundsP + baseBoundaryIndexForCurrentBody + j, fit, angleTol, fitTol);

        PK_BODY_t       bodyTag = PK_ENTITY_null;

        if (SUCCESS == PSolidGeom::CreateSheetBodyFromTrimmedSurface (&bodyTag, NULL, uvCurvesPP, 0, NULL, numLoopsInCurrentBody, surfTag, 1.0, true))
            tagList.push_back (bodyTag);
        else
            PK_ENTITY_delete (1, &surfTag);

        if (uvCurvesPP)
            {
            for (int j = 0; j < numLoopsInCurrentBody; j++)
                {
                PK_ENTITY_t* curveSet = uvCurvesPP[j];

                if (!curveSet)
                    continue;

                while (0 != (*curveSet))
                    {
                    PK_ENTITY_delete (1, curveSet);

                    ++curveSet;
                    }

                free (uvCurvesPP[j]);
                }

            free (uvCurvesPP);
            }
        baseBoundaryIndexForCurrentBody += numLoopsInCurrentBody;
        numLoopsInCurrentBody = 0;
        startId = nextId;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Peter.Yu        05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static void     createTrimmedSheetBodyListUseVuSet
(
TAG_LIST&           tagList,
MSBsplineSurfaceCR  surface,
int                 surfaceTag,     // => bspline surface tag
bool                fit,            // => whether to fit with smooth curves
double              angleTol,       // => used to determine whether a UV point is a corner
double              fitTol          // => used to fit in UV space
)
    {
    int             newNum = 0, *parentIdP = NULL;
    VuSetP          graphP;
    BsurfBoundary   *boundsP = NULL;

    // Use VU graph to create topologically correct loop array
    graphP = vu_newVuSet (0);

    if (surface.holeOrigin == false)
        vu_addRange (graphP, 0.0, 0.0, 1.0, 1.0);

    vu_addBsurfBoundaries (graphP, surface.boundaries, surface.numBounds);
    vu_markConnectedParity (graphP);
    vu_breakCompoundVertices (graphP, VU_EXTERIOR_EDGE);
    vu_collectPolygons (graphP, &boundsP, &parentIdP, &newNum, true, 2);

    vu_freeVuSet (graphP);

    // Fit trim boundaries to smooth curves while keeping sharp corners
    if (newNum > 0)
        createTrimmedSheetBodyList (tagList, boundsP, parentIdP, newNum, surfaceTag, true, TOLERANCE_UVSpaceAngle, TOLERANCE_UVSpaceFitting);

    if (NULL != parentIdP)
        vumemfuncs_free (parentIdP);

    if (NULL != boundsP)
        {
        BsurfBoundary   *bndP, *endP;

        for (bndP = endP = boundsP, endP += newNum; bndP < endP; bndP++)
            vumemfuncs_free (bndP->points);

        vumemfuncs_free (boundsP);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Josh.Schifter   12/96
+---------------+---------------+---------------+---------------+---------------+------*/
static int      pki_wire_body_from_curve
(
int             *pBodyTagOut,               /* <= body created */
int             curveTagIn,                 /* => input surface */
DPoint2dCR      paramRange                 /* => parameter range (required) */
)
    {
    PK_INTERVAL_t   range;

    range.value[0] = paramRange.x;
    range.value[1] = paramRange.y;

    return PK_CURVE_make_wire_body (curveTagIn, range, pBodyTagOut);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Josh.Schifter   12/96
+---------------+---------------+---------------+---------------+---------------+------*/
static int      pki_sheet_body_from_surface
(
int             *pBodyTagOut,               /* <= body created */
int             surfTagIn,                  /* => input surface */
DRange2dCR      paramRange                  /* => uv parameter range (required)  */
)
    {
    PK_UVBOX_t  range;

    range.param[0] = paramRange.low.x;
    range.param[1] = paramRange.low.y;
    range.param[2] = paramRange.high.x;
    range.param[3] = paramRange.high.y;

    return PK_SURF_make_sheet_body (surfTagIn, range, pBodyTagOut);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          07/96
+---------------+---------------+---------------+---------------+---------------+------*/
static int      pki_create_body_from_geometry
(
int             *pBodyTagOut,
int             *pGeomTagIn
)
    {
    int         status;
    PK_CLASS_t  entClass;

    PK_ENTITY_ask_class (*pGeomTagIn, &entClass);

    if (entClass == PK_CLASS_surf || entClass == PK_CLASS_bsurf)
        {
        DRange2d        paramRange;
        PK_UVBOX_t      range;

        PK_SURF_ask_uvbox (*pGeomTagIn, &range);

        paramRange.low.x = range.param[0];
        paramRange.low.y = range.param[1];
        paramRange.high.x = range.param[2];
        paramRange.high.y = range.param[3];

        status = pki_sheet_body_from_surface (pBodyTagOut, *pGeomTagIn, paramRange);
        }
    else if (entClass == PK_CLASS_curve)
        {
        DPoint2d        paramRange;
        PK_INTERVAL_t   range;

        PK_CURVE_ask_interval (*pGeomTagIn, &range);

        paramRange.x = range.value[0];
        paramRange.y = range.value[1];

        status = pki_wire_body_from_curve (pBodyTagOut, *pGeomTagIn, paramRange);
        }
    else
        {
        status = ERROR;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/01
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidGeom::BodyFromMSBsplineSurface (PK_BODY_t& bodyTag, MSBsplineSurfaceCR surface)
    {
    PK_BSURF_t          surfaceTag;
    MSBsplineSurfacePtr tmpSurface = MSBsplineSurface::CreatePtr ();

    tmpSurface->CopyFrom (surface);

    // Create a surface tag first
    if (SUCCESS != createSurfaceFromMSBsplineSurface (&surfaceTag, *tmpSurface))
        return ERROR;

    TAG_LIST    tagList;

    // Add trim boundaries if any
    if (tmpSurface->numBounds > 0 && NULL != tmpSurface->boundaries)
        {
        if (NULL != tmpSurface->boundaries[0].pFirst && tmpSurface->holeOrigin)
            createTrimmedSheetBodyList (tagList, tmpSurface->boundaries, NULL, tmpSurface->numBounds, surfaceTag, true, TOLERANCE_UVSpaceAngle, TOLERANCE_UVSpaceFitting);

        if (0 == tagList.size ())
            createTrimmedSheetBodyListUseVuSet (tagList, *tmpSurface, surfaceTag, true, TOLERANCE_UVSpaceAngle, TOLERANCE_UVSpaceFitting);
        }

    if (0 != tagList.size ())
        {
        bodyTag = tagList.front ();

        if (tagList.size () > 1)
            {
            int         numTools = (int) tagList.size () - 1;
            PK_BODY_t*  toolBodies = &tagList[1];

            PSolidUtil::Boolean (NULL, PK_boolean_unite, false, bodyTag, toolBodies, numTools, PKI_BOOLEAN_OPTION_AllowDisjoint);
            }
        }
    else
        {
        if (SUCCESS != pki_create_body_from_geometry (&bodyTag, &surfaceTag))
            {
            PK_ENTITY_delete (1, &surfaceTag);

            return ERROR;
            }
        }

    int                 nFaults;
    PK_check_fault_t*   faultsP = NULL;
    PK_BODY_check_o_t   options;

    PK_BODY_check_o_m (options);

    // Check geometry only...
    options.max_faults  = 0;
    options.top_geo     = PK_check_top_geo_no_c;
    options.fa_X        = PK_check_fa_X_no_c;
    options.loops       = PK_check_loops_no_c;
    options.fa_fa       = PK_check_fa_fa_no_c;
    options.sh          = PK_check_sh_no_c;
    options.corrupt     = PK_check_corrupt_no_c;
    options.size_box    = PK_check_size_box_no_c;

    if (SUCCESS != PK_BODY_check (bodyTag, &options, &nFaults, &faultsP))
        {
        PK_ENTITY_delete (1, &bodyTag);

        return ERROR;
        }

    int     nGeoms;

    PK_BODY_simplify_geom (bodyTag, PK_LOGICAL_true, &nGeoms, NULL);
    PK_TOPOL_delete_redundant (bodyTag);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley     10/2015
+---------------+---------------+---------------+---------------+--------------+------*/
StatusInt PSolidGeom::FaceToBSplineSurface(MSBsplineSurfacePtr& bSurface, CurveVectorPtr& uvBoundaries, PK_FACE_t faceTag)
    {
    PK_LOGICAL_t    isBox;
    PK_UVBOX_t      uvBox;
    PK_SURF_t       surfaceTag;
    PK_CLASS_t      surfaceClass = 0;
    PK_LOGICAL_t    orientation;
    double          tolerance = 1.0E-6;     // Needs work.

    if (SUCCESS != PK_FACE_ask_oriented_surf (faceTag, &surfaceTag, &orientation) ||
        SUCCESS != PK_FACE_is_uvbox (faceTag, &isBox, &uvBox) ||
       (!isBox && (SUCCESS != PK_FACE_find_uvbox (faceTag, &uvBox) || ! (uvBoundaries = PSolidGeom::FaceToUVCurveVector (faceTag, NULL, true)).IsValid())))
        {
//      BeAssert (false && "Unable to extract boundaries for surface");
        return ERROR;
        }

    PK_FACE_output_surf_trimmed_o_t options;

    PK_FACE_output_surf_trimmed_o_m (options);

    options.want_geoms  = PK_LOGICAL_true;
    options.want_topols = PK_LOGICAL_false;
    options.curve_tolerance = tolerance;
    options.surf_tolerance = 0.1 * tolerance;

    if (PK_CLASS_bsurf == surfaceClass) // no approximation needed if the surface is already a Bspline surface - Lu
        options.trim_surf = PK_FACE_trim_surf_own_c;
    else
        options.trim_surf = PK_FACE_trim_surf_bsurf_c;

    PK_SURF_t           bSurfaceTag;
    PK_LOGICAL_t        sense;
    PK_SURF_trim_data_t trimData;
    PK_GEOM_t*          geometryP = NULL;
    PK_INTERVAL_t*      intervalP = NULL;
    PK_TOPOL_t*         topologyP = NULL;
    StatusInt           status;

    if (SUCCESS != (status = PK_FACE_output_surf_trimmed (faceTag, &options, &bSurfaceTag, &sense, &trimData, &geometryP, &intervalP, &topologyP)))
        return ERROR;

    bSurface = MSBsplineSurface::CreatePtr();

    status = PSolidGeom::CreateMSBsplineSurfaceFromSurface (*bSurface, bSurfaceTag, NULL, geometryP, intervalP, 0, 0, tolerance, false);

    // The normalization step should not be necessary - but our (stupid) evaluate function doesn't handle max != 1.0 and I'm afraid to fix it right now...
    DPoint2d        min, max;

    bsputil_extractParameterRange (&min.x, &max.x, bSurface->uKnots, &bSurface->uParams);
    bsputil_extractParameterRange (&min.y, &max.y, bSurface->vKnots, &bSurface->vParams);

    if (min.x != 0.0 || min.y != 0.0 || max.x != 1.0 || max.y != 1.0)
        {
        if (uvBoundaries.IsValid())
            {
            double sx = 1.0 / (max.x - min.x);
            double sy = 1.0 / (max.y - min.y);

            uvBoundaries->TransformInPlace (Transform::FromRowValues (sx, 0.0, 0.0, - min.x * sx, 0.0,  sy, 0.0, - min.y * sy, 0.0, 0.0, 1.0, 0.0));
            }
        bSurface->NormalizeKnots();
        }

    PK_ENTITY_delete (1, &bSurfaceTag);

    if (trimData.n_spcurves)
        {
        PK_ENTITY_delete (trimData.n_spcurves, trimData.spcurves);

        PK_MEMORY_free (trimData.spcurves);
        PK_MEMORY_free (trimData.intervals);
        PK_MEMORY_free (trimData.trim_loop);
        PK_MEMORY_free (trimData.trim_set);
        }

    if (geometryP)
        {
        PK_ENTITY_delete (trimData.n_spcurves, geometryP);
        PK_MEMORY_free (geometryP);
        }

    if (intervalP)
        PK_MEMORY_free (intervalP);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidGeom::BodyFromBSurface (IBRepEntityPtr& entityOut, MSBsplineSurfaceCR surface, uint32_t nodeId)
    {
    PSolidKernelManager::StartSession (); // Make sure frustrum is initialized...

    DPoint3d    firstPole;

    if (surface.rational)
        bsputil_unWeightPoles (&firstPole, surface.poles, surface.weights, 1);
    else
        firstPole = surface.poles[0];

    Transform   solidToDgn, dgnToSolid;

    PSolidUtil::GetTransforms (solidToDgn, dgnToSolid, &firstPole);

    MSBsplineSurface    tmpSurface;

    if (SUCCESS != tmpSurface.CopyFrom (surface))
        return ERROR;

    PK_BODY_t   bodyTag;

    if (SUCCESS != tmpSurface.TransformSurface (&dgnToSolid) ||
        SUCCESS != PSolidGeom::BodyFromMSBsplineSurface (bodyTag, tmpSurface))
        {
        tmpSurface.ReleaseMem ();

        return ERROR;
        }

    tmpSurface.ReleaseMem ();

    if (nodeId)
        PSolidTopoId::AddNodeIdAttributes (bodyTag, nodeId, true);

    entityOut = PSolidUtil::CreateNewEntity (bodyTag, solidToDgn);

    return SUCCESS;
    }
