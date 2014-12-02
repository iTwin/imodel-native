/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/BSplineCurveHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BSplineStatus   BSplineCurveHandler::IsValidCurve (MSBsplineCurveCR curve)
    {
    if (!curve.HasValidPoleAllocation ())
        return BSPLINE_STATUS_NoPoles;

    if (!curve.HasValidWeightAllocation ())
        return BSPLINE_STATUS_NoWeights;

    if (!curve.HasValidKnotAllocation ())
        return BSPLINE_STATUS_NoKnots;

    if (!curve.HasValidOrder ())
        return BSPLINE_STATUS_BadOrder;

    if (!curve.HasValidPoleCounts ())
        return BSPLINE_STATUS_TooFewPoles;

    return BSPLINE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BSplineStatus   BSplineCurveHandler::IsValidInterpolationCurve (MSInterpolationCurveCR curve)
    {
    // Make sure none of the required components are missing
    if (!curve.fitPoints)
        return BSPLINE_STATUS_NoPoles;

    if (!curve.knots)
        return BSPLINE_STATUS_NoKnots;

    if (curve.params.numPoints > MAX_POLES-2)
        return BSPLINE_STATUS_TooManyPoles; // Chain of interpolation curves not supported...
    
    if (curve.params.numKnots > MAX_KNOTS)
        return BSPLINE_STATUS_TooManyKnots; // Chain of interpolation curves not supported...

    return BSPLINE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     createCurveKnots
(
EditElementHandleR  eeh,
MSBsplineCurveCR    curve
)
    {
    DgnElementCP hdr     = eeh.GetElementCP ();
    // Note: Subtract sizeof (double) at the end [rather than (hdr->bspoine_curve.num_knots-1) * sizeof(double)] to avoid any 'negative' unsigned numbers.
    int         elmSize = (int) (sizeof (Bspline_knot) + (hdr->ToBspline_curve().num_knots * sizeof (double)) - sizeof (double));

    double*     knots = curve.knots;

    if (!hdr->ToBspline_curve().flags.storeFullKnots)
        {
        if (!curve.params.numKnots)
            return true;

        knots = curve.knots + curve.params.order;
        }

    DgnV8ElementBlank   out;

    hdr->CopyTo (out);
    ElementUtil::SetRequiredFields (out, BSPLINE_KNOT_ELM, LevelId(hdr->GetLevel()), false, ElementUtil::ELEMDIM_NA);

    for (UInt32 i=0; i < hdr->ToBspline_curve().num_knots; i++)
        out.ToBspline_knotR().knots[i] = knots[i];

    out.SetSizeWordsNoAttributes(elmSize/2);
    eeh.GetElementDescrP()->AddComponent(*new MSElementDescr(out, *eeh.GetDgnModelP()));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     createCurveWeights
(
EditElementHandleR  eeh,
MSBsplineCurveCR    curve
)
    {
    DgnElementCP hdr = eeh.GetElementCP ();
    int         elmSize = sizeof (Bspline_weight) + (curve.params.numPoles - 1) * sizeof (double);

    if (elmSize/2 > MAX_V8_ELEMENT_SIZE)
        return false;

    int         i;
    double      maxWeight = 0.0;

    for (i=0; i < curve.params.numPoles; i++)
        {
        double  absWeight = fabs (curve.weights[i]);

        if (absWeight > maxWeight)
            maxWeight = absWeight;
        }

    DgnV8ElementBlank   out;

    hdr->CopyTo (out);
    ElementUtil::SetRequiredFields (out, BSPLINE_WEIGHT_ELM, LevelId(hdr->GetLevel()), false, ElementUtil::ELEMDIM_NA);

    double      scale = (maxWeight >= 1.0 ? maxWeight : 1.0);

    for (i=0; i < curve.params.numPoles; i++)
        out.ToBspline_weightR().weights[i] = curve.weights[i] / scale;

    out.SetSizeWordsNoAttributes(elmSize/2);
    eeh.GetElementDescrP()->AddComponent(*new MSElementDescr(out, *eeh.GetDgnModelP()));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     createCurvePoles
(
EditElementHandleR  eeh,
MSBsplineCurveCR    curve
)
    {
    if (curve.params.numPoles > MAX_POLES)
        return false;

    DgnElementCP hdr = eeh.GetElementCP ();
    int         elmSize;

    if (hdr->Is3d())
        elmSize = sizeof (Line_String_3d) + (curve.params.numPoles-1) * sizeof (DPoint3d);
    else
        elmSize = sizeof (Line_String_2d) + (curve.params.numPoles-1) * sizeof (DPoint2d);

    DgnV8ElementBlank   out;

    hdr->CopyTo (out);
    ElementUtil::SetRequiredFields (out, BSPLINE_POLE_ELM, LevelId(hdr->GetLevel()), false, (ElementUtil::ElemDim) hdr->Is3d());

    out.ToLine_String_2dR().numverts = curve.params.numPoles;
    out.ToLine_String_2dR().reserved = 0;

    if (curve.rational)
        bsputil_unWeightPoles (curve.poles, curve.poles, curve.weights, curve.params.numPoles);

    if (hdr->Is3d())
        ElementUtil::PackLineWords3d (out.ToLine_String_2dR().vertice, curve.poles, curve.params.numPoles);
    else
        ElementUtil::PackLineWords2d (out.ToLine_String_2dR().vertice, curve.poles, curve.params.numPoles);

    if (curve.rational)
        bsputil_weightPoles (curve.poles, curve.poles, curve.weights, curve.params.numPoles);

    out.SetSizeWordsNoAttributes(elmSize/2);

    if (SUCCESS != DisplayHandler::ValidateElementRange (&out, eeh.GetDgnModelP()))
        return false;

    eeh.GetElementDescrP()->AddComponent(*MSElementDescr::Allocate (out, *eeh.GetDgnModelP()).get());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static void     createCurveHeader
(
EditElementHandleR  eeh,
DgnElementCP         in,                     //  => template element (or NULL)
MSBsplineCurveCR    curve,
bool                is3d,
DgnModelR           model
)
    {
    DgnV8ElementBlank   out;

    if (in)
        {
        in->CopyTo (out);
        ElementUtil::SetRequiredFields (out, BSPLINE_CURVE_ELM, LevelId(in->GetLevel()), false, (ElementUtil::ElemDim) is3d);
        }
    else
        {
        memset (&out, 0, sizeof (Bspline_curve));
        ElementUtil::SetRequiredFields (out, BSPLINE_CURVE_ELM, LEVEL_DEFAULT_LEVEL_ID, false, (ElementUtil::ElemDim) is3d);
        }

    int         elmSize = sizeof (Bspline_curve);

    out.SetSizeWordsNoAttributes(elmSize/2);
    ElementUtil::CopyAttributes (&out, in);

    memset (&out.ToBspline_curveR().flags, 0, sizeof (out.ToBspline_curve().flags));

    out.ToBspline_curveR().flags.order               = curve.params.order - 2;
    out.ToBspline_curveR().flags.curve_display       = curve.display.curveDisplay   != 0;
    out.ToBspline_curveR().flags.poly_display        = curve.display.polygonDisplay != 0;
    out.ToBspline_curveR().flags.rational            = curve.rational != 0;
    out.ToBspline_curveR().flags.closed              = curve.params.closed != 0;
    out.ToBspline_curveR().flags.construct_type      = BSPLINE_CONSTRUCTION_POLE;

    if (curve.type >= 0 && curve.type <= 7)
        out.ToBspline_curveR().flags.curve_type = curve.type;

    out.ToBspline_curveR().num_poles = curve.params.numPoles;
    out.ToBspline_curveR().num_knots = curve.params.numKnots;

    int         numFullKnots, numInteriorKnots;

    // correct header to indicate #knots stored in the knot element
    out.ToBspline_curveR().flags.storeFullKnots = curve.params.numKnots ? TO_BOOL (mdlBspline_curveStoreFullKnots (&curve)) : false;

    numFullKnots = curve.NumberAllocatedKnots ();
    numInteriorKnots = numFullKnots - 2 * curve.params.order;

    if (out.ToBspline_curve().flags.storeFullKnots)
        out.ToBspline_curveR().num_knots = numFullKnots;
    else if (curve.params.numKnots)
        out.ToBspline_curveR().num_knots = numInteriorKnots;

    eeh.SetElementDescr(new MSElementDescr(out, model), false);
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static BSplineStatus   createCurveElement 
(
EditElementHandleR  eeh,
DgnElementCP        templateEl,
MSBsplineCurveCR    curve,
bool                is3d,
DgnModelR           model
)
    {
    createCurveHeader (eeh, templateEl, curve, is3d, model);

    if (!createCurveKnots (eeh, curve))
        return BSPLINE_STATUS_BadKnots;

    if (!createCurvePoles (eeh, curve))
        return BSPLINE_STATUS_BadPoles;

    if (curve.rational && !createCurveWeights (eeh, curve))
        return BSPLINE_STATUS_BadWeights;

    return (SUCCESS == eeh.GetDisplayHandler()->ValidateElementRange (eeh)) ? BSPLINE_STATUS_Success : BSPLINE_STATUS_BadBspElement;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static BSplineStatus   createChainElement
(
EditElementHandleR  eeh,
ElementHandleCP     templateEh,
MSBsplineCurveCR    curve,
bool                is3d,
DgnModelR           model
)
    {
    MSBsplineCurve  tmpCurve;

    if (SUCCESS != tmpCurve.CopyFrom (curve))
        return BSPLINE_STATUS_BadBspElement;

    ChainHeaderHandler::CreateChainHeaderElement (eeh, templateEh, TO_BOOL (curve.params.closed), is3d, model);

    BSplineStatus   status = BSPLINE_STATUS_BadBspElement;
    EditElementHandle  curveEeh;

    do
        {
        double          breakPoint = tmpCurve.knots[MAX_POLES];
        MSBsplineCurve  segment;

        memset (&segment, 0, sizeof (segment));

        if (SUCCESS != segment.CopySegment (tmpCurve, 0.0, breakPoint) ||
            SUCCESS != tmpCurve.CopySegment (tmpCurve, breakPoint, 1.0) ||
            BSPLINE_STATUS_Success != (status = createCurveElement (curveEeh, templateEh ? templateEh->GetElementCP () : NULL, segment, is3d, model)))
            {
            segment.ReleaseMem ();
            tmpCurve.ReleaseMem ();

            return status;
            }

        eeh.GetElementDescrP()->AddComponent(*curveEeh.ExtractElementDescr().get());

        } while (tmpCurve.params.numPoles > MAX_POLES);

    status = createCurveElement (curveEeh, templateEh ? templateEh->GetElementCP () : NULL, tmpCurve, is3d, model);

    tmpCurve.ReleaseMem ();

    if (BSPLINE_STATUS_Success != status)
        return status;

    eeh.GetElementDescrP()->AddComponent(*curveEeh.ExtractElementDescr().get());

    return (SUCCESS == eeh.GetDisplayHandler()->ValidateElementRange (eeh)) ? BSPLINE_STATUS_Success : BSPLINE_STATUS_BadBspElement;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BSplineStatus   BSplineCurveHandler::CreateBSplineCurveElement
(
EditElementHandleR  eeh,
ElementHandleCP     templateEh,
MSBsplineCurveCR    curve,
bool                is3d,
DgnModelR           model
)
    {
    BSplineStatus   status;

    if (BSPLINE_STATUS_Success != (status = BSplineCurveHandler::IsValidCurve (curve)))
        return status;

    // CR #123496: guard against programmer filling in knots field incorrectly, or not at all
    if (!mdlBspline_areKnotsValid (curve.knots, &curve.params, false))
        bspknot_computeKnotVector (curve.knots, (BsplineParam*) &curve.params, NULL);

    if (curve.params.numPoles <= MAX_POLES)
        {
        if (BSPLINE_STATUS_Success != (status = createCurveElement (eeh, templateEh ? templateEh->GetElementCP () : NULL, curve, is3d, model)))
            return status;
        }
    else
        {
        if (BSPLINE_STATUS_Success != (status = createChainElement (eeh, templateEh, curve, is3d, model)))
            return status;
        }

    return (eeh.IsValid () ? BSPLINE_STATUS_Success : BSPLINE_STATUS_BadBspElement);
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static void     computeInterpolationCurveTangentPoints
(
DPoint3dP               startTangentPt,
DPoint3dP               endTangentPt,
MSInterpolationCurveCR  curve
)
    {
    if (startTangentPt)
        startTangentPt->sumOf (&curve.fitPoints[0], &curve.startTangent, curve.fitPoints[0].distance (&curve.fitPoints[1]) * 0.5);

    if (endTangentPt)
        endTangentPt->sumOf (&curve.fitPoints[curve.params.numPoints-1], &curve.endTangent, curve.fitPoints[curve.params.numPoints-1].distance (&curve.fitPoints[curve.params.numPoints-2]) * 0.5);
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     createInterpolationCurveKnots
(
EditElementHandleR      eeh,
MSInterpolationCurveCR  curve
)
    {
    DgnElementCP hdr     = eeh.GetElementCP ();
    // Note: We need signed arithmetic for this size calculation because num_knots can be zero.
    int         elmSize = (int) sizeof (Bspline_knot) + ((int) hdr->ToBspline_curve().num_knots - 1) * (int) sizeof (double);

    if (elmSize/2 > MAX_V8_ELEMENT_SIZE)
        return false;

    if (!curve.params.numKnots)
        return true;

    double*     knots = curve.knots;
    DgnV8ElementBlank   out;

    hdr->CopyTo (out);
    ElementUtil::SetRequiredFields (out, BSPLINE_KNOT_ELM, LevelId(hdr->GetLevel()), false, ElementUtil::ELEMDIM_NA);

    for (UInt32 i=0; i < hdr->ToBspline_curve().num_knots; i++)
        out.ToBspline_knotR().knots[i] = knots[i];

    out.SetSizeWordsNoAttributes(elmSize/2);

    eeh.GetElementDescrP()->AddComponent(*new MSElementDescr(out, *eeh.GetDgnModelP()));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     createInterpolationCurvePoles
(
EditElementHandleR      eeh,
MSInterpolationCurveCR  curve
)
    {
    int         numPoles = curve.params.numPoints + 2;

    if (numPoles > MAX_POLES)
        return false;

    DgnElementCP hdr = eeh.GetElementCP ();
    int         elmSize;

    if (hdr->Is3d())
        elmSize = sizeof (Line_String_3d) + (numPoles-1) * sizeof (DPoint3d);
    else
        elmSize = sizeof (Line_String_2d) + (numPoles-1) * sizeof (DPoint2d);

    DgnV8ElementBlank   out;

    hdr->CopyTo (out);
    ElementUtil::SetRequiredFields (out, BSPLINE_POLE_ELM, LevelId(hdr->GetLevel()), false, (ElementUtil::ElemDim) hdr->Is3d());

    out.ToLine_String_2dR().numverts = numPoles;
    out.ToLine_String_2dR().reserved = 0;

    DPoint3d    tangentPoints[2];

    // Compute interpolation curve tangent points...
    computeInterpolationCurveTangentPoints (&tangentPoints[0], &tangentPoints[1], curve);

    if (hdr->Is3d())
        {
        ElementUtil::PackLineWords3d (out.ToLine_String_2dR().vertice, curve.fitPoints, curve.params.numPoints);
        ElementUtil::PackLineWords3d ((DPoint2dP) (out.ToLine_String_3d().vertice+curve.params.numPoints), tangentPoints, 2);
        }
    else
        {
        ElementUtil::PackLineWords2d (out.ToLine_String_2dR().vertice, curve.fitPoints, curve.params.numPoints);
        ElementUtil::PackLineWords2d (out.ToLine_String_2dR().vertice+curve.params.numPoints, tangentPoints, 2);
        }

    out.SetSizeWordsNoAttributes(elmSize/2);

    if (SUCCESS != DisplayHandler::ValidateElementRange (&out, eeh.GetDgnModelP()))
        return false;

    eeh.GetElementDescrP()->AddComponent(*new MSElementDescr(out, *eeh.GetDgnModelP()));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static void     createInterpolationCurveHeader
(
EditElementHandleR      eeh,
DgnElementCP             in,                     //  => template element (or NULL)
MSInterpolationCurveCR  curve,
bool                    is3d,
DgnModelR               model
)
    {
    DgnV8ElementBlank   out;

    if (in)
        {
        in->CopyTo (out);
        ElementUtil::SetRequiredFields (out, BSPLINE_CURVE_ELM, LevelId(in->GetLevel()), false, (ElementUtil::ElemDim) is3d);
        }
    else
        {
        memset (&out, 0, sizeof (Bspline_curve));
        ElementUtil::SetRequiredFields (out, BSPLINE_CURVE_ELM, LEVEL_DEFAULT_LEVEL_ID, false, (ElementUtil::ElemDim) is3d);
        }

    int         elmSize = sizeof (Bspline_curve);

    out.SetSizeWordsNoAttributes(elmSize/2);
    ElementUtil::CopyAttributes (&out, in);

    memset (&out.ToBspline_curveR().flags, 0, sizeof (out.ToBspline_curve().flags));

    out.ToBspline_curveR().flags.order               = curve.params.order - 2;
    out.ToBspline_curveR().flags.curve_display       = curve.display.curveDisplay   != 0;
    out.ToBspline_curveR().flags.poly_display        = curve.display.polygonDisplay != 0;
    out.ToBspline_curveR().flags.rational            = false;
    out.ToBspline_curveR().flags.closed              = curve.params.isPeriodic;
    out.ToBspline_curveR().flags.curve_type          = 0;
    out.ToBspline_curveR().flags.construct_type      = BSPLINE_CONSTRUCTION_INTERPOLATION;
    out.ToBspline_curveR().flags.isNaturalTangents   = curve.params.isNaturalTangents;
    out.ToBspline_curveR().flags.isChordLenTangents  = curve.params.isChordLenTangents;
    out.ToBspline_curveR().flags.isColinearTangents  = curve.params.isColinearTangents;
    out.ToBspline_curveR().flags.isChordLenKnots     = curve.params.isChordLenKnots;

    out.ToBspline_curveR().num_poles = curve.params.numPoints + 2;
    out.ToBspline_curveR().num_knots = curve.params.numKnots;

    if (!curve.params.isPeriodic)
        {
        if (fabs (curve.startTangent.x) <= mgds_fc_nearZero && fabs (curve.startTangent.y) <= mgds_fc_nearZero && fabs (curve.startTangent.z) <= mgds_fc_nearZero)
            out.ToBspline_curveR().flags.isZeroStartTangent = true;

        if (fabs (curve.endTangent.x) <= mgds_fc_nearZero && fabs (curve.endTangent.y) <= mgds_fc_nearZero && fabs (curve.endTangent.z) <= mgds_fc_nearZero)
            out.ToBspline_curveR().flags.isZeroEndTangent = true;
        }

    eeh.SetElementDescr (new MSElementDescr(out, model), false);
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static BSplineStatus   createInterpolationCurveElement 
(
EditElementHandleR      eeh,
DgnElementCP            templateEl,
MSInterpolationCurveCR  curve,
bool                    is3d,
DgnModelR               model
)
    {
    createInterpolationCurveHeader (eeh, templateEl, curve, is3d, model);

    if (!createInterpolationCurveKnots (eeh, curve))
        return BSPLINE_STATUS_BadKnots;

    if (!createInterpolationCurvePoles (eeh, curve))
        return BSPLINE_STATUS_BadPoles;

    return (SUCCESS == eeh.GetDisplayHandler()->ValidateElementRange (eeh)) ? BSPLINE_STATUS_Success : BSPLINE_STATUS_BadBspElement;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BSplineStatus   BSplineCurveHandler::CreateBSplineCurveElement
(
EditElementHandleR      eeh,
ElementHandleCP         templateEh,
MSInterpolationCurveCR  curve,
bool                    is3d,
DgnModelR               model
)
    {
    BSplineStatus   status;

    if (BSPLINE_STATUS_Success != (status = BSplineCurveHandler::IsValidInterpolationCurve (curve)))
        return status;

    if (BSPLINE_STATUS_Success != (status = createInterpolationCurveElement (eeh, templateEh ? templateEh->GetElementCP () : NULL, curve, is3d, model)))
        return status;

    return (eeh.IsValid () ? BSPLINE_STATUS_Success : BSPLINE_STATUS_BadBspElement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/04
+---------------+---------------+---------------+---------------+---------------+------*/
static void     extractPoleDataTo3d
(
DPoint3d        *outPolesP,
const DPoint2d  *inPolesP,
int             numPoles,
bool            is3d
)
    {
    if (is3d)
        {
        memcpy (outPolesP, inPolesP, numPoles * sizeof (DPoint3d));
        }
    else
        {
        for (int i=0; i < numPoles; i++, inPolesP++, outPolesP++)
            outPolesP->init (inPolesP);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void     checkClosedForRepeatedPole
(
BsplineParam    *tmpParamsP,
DgnElementCP     headerP,        /* => bcurve elm */
ChildElemIter*  elIter          /* => iter pointing to firstElem (pole) */
)
    {
    if (tmpParamsP->numKnots)
        {
        tmpParamsP->numPoles = tmpParamsP->numKnots + 1;
        }
    else if (tmpParamsP->order != tmpParamsP->numPoles)
        {
        // closed non-Bezier curves with uniform knots have highest order continuity at the closure, but a redundant end pole ruins this
        int             offset;
        DPoint3d        p3d0, p3d1;
        DgnElementCP     poleElmP = elIter->GetElementCP();

        offset = tmpParamsP->numPoles - 1;

        if (headerP->Is3d())
            {
            p3d0 = poleElmP->ToBspline_pole_3d().poles[0];
            p3d1 = poleElmP->ToBspline_pole_3d().poles[offset];
            }
        else
            {
            memcpy (&p3d0, poleElmP->ToBspline_pole_2d().poles, sizeof (DPoint2d));
            memcpy (&p3d1, poleElmP->ToBspline_pole_2d().poles + offset, sizeof (DPoint2d));
            p3d0.z = p3d1.z = 0;
            }

        if (headerP->ToBspline_curve().flags.rational)
            {
            ChildElemIter    tmpIter (*elIter, ExposeChildrenReason::Count);

            if (tmpIter.IsValid())
                {
                DgnElementCP     weightElmP = tmpIter.GetElementCP();

                if (bsiDPoint3d_pointEqual (&p3d0, &p3d1) && (weightElmP->ToBspline_weight().weights[0] == weightElmP->ToBspline_weight().weights[offset]))
                    tmpParamsP->numPoles--;
                }
            }
        else if (bsiDPoint3d_pointEqual (&p3d0, &p3d1))
            {
            tmpParamsP->numPoles--;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/03
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   processPoleCurveComponent
(
BsplineParam    *tmpParamsP,
DgnElementCP     headerP,        /* => bcurve elm */
DPoint3d        **polesPP,
double          **knotsPP,
double          **weightsPP,
bool            *polesFoundP,
bool            *knotsFoundP,
bool            *weightsFoundP,
bool            storeFullKnots,
ElementHandleCP elHandle        /* => pointing to current component */
)
    {
    DgnElementCP elP = elHandle->GetElementCP();

    if (!elP)
        return DGNMODEL_STATUS_OutOfMemory;

    switch (elP->GetLegacyType())
        {
        case BSPLINE_POLE_ELM:
            {
            *polesFoundP = true;

            if (polesPP && *polesPP)
                extractPoleDataTo3d (*polesPP, elP->ToBspline_pole_2d().poles, tmpParamsP->numPoles, elP->Is3d());
            break;
            }

        case BSPLINE_KNOT_ELM:
            {
            *knotsFoundP = true;

            if (knotsPP && *knotsPP)
                {
                double      tmpKnots[MAX_KNOTS];

                if (!storeFullKnots)
                    {
                    if (tmpParamsP->numKnots > MAX_KNOTS)
                        return BSPLINE_STATUS_BadBspElement;

                    bsputil_extractScaledValues (tmpKnots, (double *) elP->ToBspline_knot().knots, tmpParamsP->numKnots);

                    bspknot_computeKnotVector (*knotsPP, tmpParamsP, tmpKnots);
                    }
                else
                    {
                    int     nFullKnots = bspknot_numberKnots (tmpParamsP->numPoles, tmpParamsP->order, tmpParamsP->closed);

                    if (nFullKnots > MAX_KNOTS)
                        return BSPLINE_STATUS_BadBspElement;

                    /* Non-normalized knots: extract full knot bvector */
                    bsputil_extractScaledValues (tmpKnots, (double *) elP->ToBspline_knot().knots, nFullKnots);

                    bspknot_computeKnotVectorNotNormalized (*knotsPP, tmpParamsP, tmpKnots);
                    }
                }
            break;
            }

        case BSPLINE_WEIGHT_ELM:
            {
            *weightsFoundP = true;

            if (weightsPP && *weightsPP)
                bspconv_extractWeightValues (*weightsPP, (double *) elP->ToBspline_weight().weights, tmpParamsP->numPoles);
            break;
            }

        default:
            return BSPLINE_STATUS_BadBspElement;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/03
+---------------+---------------+---------------+---------------+---------------+------*/
static int      extractCurveDefinedByPoles
(
DgnElement       *header,                /* <= Header element */
int             *type,                  /* <= curve type */
int             *rational,              /* <= rational (weights included) */
BsplineDisplay  *display,               /* <= display parameters */
BsplineParam    *params,                /* <= number of poles etc. */
DPoint3d        **poles,                /* <= pole coordinates */
double          **knots,                /* <= knot bvector */
double          **weights,              /* <= weights (if (Rational) */
ElementHandleCP elHandle                /* => Input element descriptor */
)
    {
    StatusInt       status = SUCCESS;
    int             allocSize;
    bool            storeFullKnots, polesFound, knotsFound, weightsFound;
    BsplineParam    tmpParams;
    DgnElementCP     headerP = elHandle->GetElementCP();

    polesFound = knotsFound = weightsFound = false;

    if (BSPLINE_CURVE_ELM != headerP->GetLegacyType())
        return BSPLINE_STATUS_NoBspHeader;

    ChildElemIter   childIter (*elHandle, ExposeChildrenReason::Count);

    if (!childIter.IsValid ())
        return BSPLINE_STATUS_NoPoles;

    if (header)
        {
        allocSize = headerP->GetSizeWords() * 2;
        memcpy (header, headerP, allocSize);
        }

    if (type)
        *type = headerP->ToBspline_curve().flags.curve_type;

    if (rational)
        *rational = headerP->ToBspline_curve().flags.rational;

    if (display)
        {
        display->curveDisplay   = headerP->ToBspline_curve().flags.curve_display;
        display->polygonDisplay = headerP->ToBspline_curve().flags.poly_display;
        }

    tmpParams.order    = headerP->ToBspline_curve().flags.order + 2;
    tmpParams.closed   = headerP->ToBspline_curve().flags.closed;
    tmpParams.numPoles = headerP->ToBspline_curve().num_poles;
    tmpParams.numKnots = headerP->ToBspline_curve().num_knots;
    storeFullKnots     = headerP->ToBspline_curve().flags.storeFullKnots;

    /* Check for repeated last pole on closed curves */
    if (tmpParams.closed)
        checkClosedForRepeatedPole (&tmpParams, headerP, &childIter);

    if (params)
        *params = tmpParams;

    if (tmpParams.order < 2 || tmpParams.order > MAX_ORDER)
        return BSPLINE_STATUS_BadOrder;

    if (tmpParams.numPoles < tmpParams.order)
        return BSPLINE_STATUS_TooFewPoles;

    /* Allocate pole buffer */
    if (poles)
        {
        allocSize = tmpParams.numPoles * sizeof (DPoint3d);

//        if (NULL == (*poles = (DPoint3d *) dlmSystem_mdlMallocWithDescr (allocSize, (mdlDesc *) pHeapDescr)))
        if (NULL == (*poles = (DPoint3d *) BSIBaseGeom::Malloc (allocSize)))
            return DGNMODEL_STATUS_OutOfMemory;
        memset (*poles, 0, allocSize);
        }

    if (knots)
        {
        allocSize = bspknot_numberKnots (tmpParams.numPoles, tmpParams.order, tmpParams.closed) * sizeof (double);

//        if (NULL == (*knots = (double *) dlmSystem_mdlMallocWithDescr (allocSize, (mdlDesc *) pHeapDescr)))
        if (NULL == (*knots = (double *) BSIBaseGeom::Malloc (allocSize)))
            {
            status = DGNMODEL_STATUS_OutOfMemory;

            goto wrapup;
            }
        }

    if (weights)
        {
        if (headerP->ToBspline_curve().flags.rational)
            {
            allocSize = tmpParams.numPoles * sizeof (double);

//            if (NULL == (*weights = (double *) dlmSystem_mdlMallocWithDescr (allocSize, (mdlDesc *) pHeapDescr)))
            if (NULL == (*weights = (double *) BSIBaseGeom::Malloc (allocSize)))
                {
                status = DGNMODEL_STATUS_OutOfMemory;

                goto wrapup;
                }
            }
        else
            {
            *weights = NULL;
            }
        }

    /* If no component data is required, return now */
    if (!poles && !knots && !weights)
        return SUCCESS;

    // step through all of the children of the shape (ignoring their symbology)
    do
        {
        if (SUCCESS != (status = processPoleCurveComponent (&tmpParams, headerP,
                                                            poles, knots, weights, &polesFound, &knotsFound, &weightsFound,
                                                            storeFullKnots, &childIter)))
            goto wrapup;

        childIter = childIter.ToNext();

        } while (childIter.IsValid());

    if (!polesFound)
        status = BSPLINE_STATUS_NoPoles;

    if (tmpParams.numKnots && !knotsFound)
        status = BSPLINE_STATUS_NoKnots;

    if (headerP->ToBspline_curve().flags.rational && !weightsFound)
        status = BSPLINE_STATUS_NoWeights;

    // If we didn't find a knot element, still need to set output...
    if (!knotsFound)
        {
        if (knots && *knots)
            bspknot_computeKnotVector (*knots, &tmpParams, NULL);
        }

wrapup:
    if (status)
        {
        if (*poles)
//            dlmSystem_mdlFreeWithDescr (*poles, (mdlDesc *) pHeapDescr);
            BSIBaseGeom::Free (*poles);

        if (*knots)
//            dlmSystem_mdlFreeWithDescr (*knots, (mdlDesc *) pHeapDescr);
            BSIBaseGeom::Free (*knots);

        if (*weights)
//            dlmSystem_mdlFreeWithDescr (*weights, (mdlDesc *) pHeapDescr);
            BSIBaseGeom::Free (*weights);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/03
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   processInterpolationCurveComponent
(
InterpolationParam  *tmpParamsP,
DgnElementCP         headerP,        /* => bcurve elm */
DPoint3d            **polesPP,
double              **knotsPP,
bool                *polesFoundP,
bool                *knotsFoundP,
DPoint3d            *startTangentP,
DPoint3d            *endTangentP,
ElementHandleP      elHandle        /* => pointing to current component */
)
    {
    DgnElementCP     elP = elHandle->GetElementCP();

    switch (elP->GetLegacyType())
        {
        case BSPLINE_POLE_ELM:
            {
            *polesFoundP = true;

            if (polesPP && *polesPP)
                {
                DPoint3d    points[MAX_POLES];

                extractPoleDataTo3d (points, elP->ToBspline_pole_2d().poles, tmpParamsP->numPoints + 2, elP->Is3d());
                memcpy (*polesPP, points, tmpParamsP->numPoints * sizeof (DPoint3d));

                /* no start/end tangents for periodic interpolants */
                if (!tmpParamsP->isPeriodic)
                    {
                    /*
                    The last two points in the poles list are the heads of
                    the start/end tangents, but we ignore them if we're
                    going to autocompute them.
                    */
                    if (startTangentP && !elP->ToBspline_curve().flags.isZeroStartTangent)
                        mdlBspline_setInterpolationTangents (startTangentP, &points[0], &points[tmpParamsP->numPoints], NULL, NULL, NULL);

                    if (endTangentP && !elP->ToBspline_curve().flags.isZeroEndTangent)
                        mdlBspline_setInterpolationTangents (NULL, NULL, NULL, endTangentP, &points[tmpParamsP->numPoints-1], &points[tmpParamsP->numPoints+1]);
                    }
                }
            break;
            }

        case BSPLINE_KNOT_ELM:
            {
            *knotsFoundP = true;

            if (knotsPP && *knotsPP)
                bsputil_extractScaledValues (*knotsPP, (double *) elP->ToBspline_knot().knots, tmpParamsP->numKnots);
            break;
            }

        default:
            return BSPLINE_STATUS_BadBspElement;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/03
+---------------+---------------+---------------+---------------+---------------+------*/
static int      extractInterpolationCurveInfo
(
DgnElement           *header,            /* <= Header element */
InterpolationParam  *params,            /* <= interpolation params*/
BsplineDisplay      *display,           /* <= display parameters */
DPoint3d            **fitPts,           /* <= fit point coordinates */
DPoint3d            *startTangent,      /* <= start tangent */
DPoint3d            *endTangent,        /* <= end tangent */
double              **knots,            /* <= knot bvector */
ElementHandleCP     elHandle            /* => Input element descriptor */
)
    {
    StatusInt           status = SUCCESS;
    int                 allocSize;
    bool                polesFound, knotsFound;
    InterpolationParam  tmpParams;
    DgnElementCP         headerP = elHandle->GetElementCP();

    polesFound = knotsFound = false;

    if (BSPLINE_CURVE_ELM != headerP->GetLegacyType())
        return BSPLINE_STATUS_NoBspHeader;

    ChildElemIter    childIter (*elHandle, ExposeChildrenReason::Count);

    if (!childIter.IsValid())
        return BSPLINE_STATUS_NoPoles;

    if (header)
        {
        allocSize = headerP->GetSizeWords() * 2;
        memcpy (header, headerP, allocSize);
        }

    if (display)
        {
        display->curveDisplay   = headerP->ToBspline_curve().flags.curve_display;
        display->polygonDisplay = headerP->ToBspline_curve().flags.poly_display;
        }

    tmpParams.order              = headerP->ToBspline_curve().flags.order + 2;
    tmpParams.isPeriodic         = headerP->ToBspline_curve().flags.closed;
    tmpParams.numPoints          = headerP->ToBspline_curve().num_poles - 2;
    tmpParams.numKnots           = headerP->ToBspline_curve().num_knots;
    tmpParams.isChordLenKnots    = headerP->ToBspline_curve().flags.isChordLenKnots;
    tmpParams.isChordLenTangents = headerP->ToBspline_curve().flags.isChordLenTangents;
    tmpParams.isColinearTangents = headerP->ToBspline_curve().flags.isColinearTangents;
    tmpParams.isNaturalTangents  = headerP->ToBspline_curve().flags.isNaturalTangents;

    if (params)
        *params = tmpParams;

    if (tmpParams.numPoints < 2)
        return BSPLINE_STATUS_TooFewPoles;

    /* Allocate pole buffer */
    if (fitPts)
        {
        allocSize = tmpParams.numPoints * sizeof (DPoint3d);

//        if (NULL == (*fitPts = (DPoint3d *) dlmSystem_mdlMallocWithDescr (allocSize, (mdlDesc *) pHeapDescr)))
        if (NULL == (*fitPts = (DPoint3d *) BSIBaseGeom::Malloc (allocSize)))
            return DGNMODEL_STATUS_OutOfMemory;
        }

    if (knots)
        {
        allocSize = tmpParams.numKnots * sizeof (double);

//        if (NULL == (*knots = (double *) dlmSystem_mdlMallocWithDescr (allocSize, (mdlDesc *) pHeapDescr)))
        if (NULL == (*knots = (double *) BSIBaseGeom::Malloc (allocSize)))
            {
            status = DGNMODEL_STATUS_OutOfMemory;

            goto wrapup;
            }
        }

    /* default tangent is zero => autocompute it acc to isNaturalTangents flag */
    if (startTangent)
        {
        startTangent->x = startTangent->y = startTangent->z = 0.0;
        }

    if (endTangent)
        {
        endTangent->x = endTangent->y = endTangent->z = 0.0;
        }

    /* If no component data is required, return now */
    if (!fitPts && !knots && !startTangent && !endTangent)
        return SUCCESS;

    // step through all of the children of the shape (ignoring their symbology)
    do
        {
        if (SUCCESS != (status = processInterpolationCurveComponent (&tmpParams, headerP,
                                                                     fitPts, knots, &polesFound, &knotsFound,
                                                                     startTangent, endTangent, &childIter)))
            goto wrapup;

        childIter = childIter.ToNext();
        } while (childIter.IsValid());

    if (!polesFound)
        status = BSPLINE_STATUS_NoPoles;

    if (tmpParams.numKnots && !knotsFound)
        status = BSPLINE_STATUS_NoKnots;

wrapup:
    if (status)
        {
        if (*fitPts)
//            dlmSystem_mdlFreeWithDescr (*fitPts, (mdlDesc *) pHeapDescr);
            BSIBaseGeom::Free (*fitPts);

        if (*knots)
//            dlmSystem_mdlFreeWithDescr (*knots, (mdlDesc *) pHeapDescr);
            BSIBaseGeom::Free (*knots);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/03
+---------------+---------------+---------------+---------------+---------------+------*/
static int      extractCurveDefinedByInterpolation
(
DgnElement       *header,                /* <= Header element */
int             *type,                  /* <= curve type */
int             *rational,              /* <= rational (weights included) */
BsplineDisplay  *display,               /* <= display parameters */
BsplineParam    *params,                /* <= number of poles etc. */
DPoint3d        **poles,                /* <= pole coordinates */
double          **knots,                /* <= knot bvector */
double          **weights,              /* <= weights (if (Rational) */
ElementHandleCP elHandle                /* => Input element descriptor */
)
    {
    StatusInt           status;
    DPoint3d            *fitPoints = NULL, tangents[2];
    InterpolationParam  tmpParams;

    if (rational)
        *rational = false;

    /* ignore knots from elemIter; we will autocompute them from fitpts */
    if (SUCCESS == (status = extractInterpolationCurveInfo (header, &tmpParams, display, &fitPoints,
                                    &tangents[0], &tangents[1], NULL, elHandle)))
        {
        MSBsplineCurve  curve;

        memset (&curve, 0, sizeof (MSBsplineCurve));

        if (SUCCESS == (status = bspcurv_c2CubicInterpolateCurveExt (&curve, fitPoints, NULL, tmpParams.numPoints, true, mgds_fc_epsilon, tangents,
                                        TO_BOOL (tmpParams.isPeriodic), TO_BOOL (tmpParams.isChordLenKnots), TO_BOOL (tmpParams.isColinearTangents),
                                        TO_BOOL (tmpParams.isChordLenTangents), TO_BOOL (tmpParams.isNaturalTangents))))
            {
            if (params)
                *params = curve.params;

            if (poles)
                *poles = curve.poles;

            if (knots)
                *knots = curve.knots;

            if (weights)
                *weights = NULL;
            }
        }

//    dlmSystem_mdlFreeWithDescr (fitPoints, (mdlDesc *) pHeapDescr);
    BSIBaseGeom::Free (fitPoints);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz     10/03
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     readToCurve
(
ElementHandleCR source,
MSBsplineCurveR curve
)
    {
    if (!source.IsValid () || BSPLINE_CURVE_ELM != source.GetLegacyType())
        return false;

    memset (&curve, 0, sizeof (curve));

    if (BSPLINE_CONSTRUCTION_POLE == source.GetElementCP ()->ToBspline_curve().flags.construct_type)
        {
        if (SUCCESS != extractCurveDefinedByPoles (NULL, &curve.type, &curve.rational,
                                                   &curve.display, &curve.params,
                                                   &curve.poles, &curve.knots, &curve.weights,
                                                   &source))
            return false;
        }
    else if (BSPLINE_CONSTRUCTION_INTERPOLATION == source.GetElementCP ()->ToBspline_curve().flags.construct_type)
        {
        if (SUCCESS != extractCurveDefinedByInterpolation (NULL, &curve.type, &curve.rational,
                                                           &curve.display, &curve.params,
                                                           &curve.poles, &curve.knots, &curve.weights,
                                                           &source))
            return false;
        }
    else
        {
        return false;
        }

    if (curve.rational)
        bsputil_weightPoles (curve.poles, curve.poles, curve.weights, curve.params.numPoles);

    mdlBspline_validateCurveKnots (curve.knots, curve.poles, curve.weights, &curve.params);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  02/10
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     isClosedCurve (ElementHandleCR eh)
    {
    // NOTE: Checking physically closed is expensive...if people want fill/pattern/rendered bcurves, they can create closed ones!
    return eh.GetElementCP()->ToBspline_curve().flags.closed;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   BSplineCurveHandler::CurveFromElement (MSBsplineCurveR curve, ElementHandleCR eh)
    {
    CurveVectorPtr curveVector = ICurvePathQuery::ElementToCurveVector (eh);

    if (curveVector.IsNull ())
        return ERROR;

    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Invalid == curveVector->HasSingleCurvePrimitive ())
        return ERROR;

    if (!curveVector->front ()->GetMSBsplineCurve (curve))
        return ERROR;

    if (curveVector->IsClosedPath ())
        curve.MakeClosed ();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   BSplineCurveHandler::InterpolationCurveFromElement (MSInterpolationCurveR fitCurve, ElementHandleCR eh)
    {
    if (BSPLINE_CURVE_ELM != eh.GetLegacyType() || BSPLINE_CONSTRUCTION_INTERPOLATION != eh.GetElementCP()->ToBspline_curve().flags.construct_type)
        return ERROR;

    memset (&fitCurve, 0, sizeof (fitCurve));

    return (SUCCESS == extractInterpolationCurveInfo (NULL, &fitCurve.params, &fitCurve.display, &fitCurve.fitPoints, &fitCurve.startTangent, &fitCurve.endTangent, &fitCurve.knots, &eh) ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BSplineCurveHandler::_IsSupportedOperation (ElementHandleCP eh, SupportOperation stype)
    {
    switch (stype)
        {
        case SupportOperation::LineStyle:
            return true;

        default:
            return T_Super::_IsSupportedOperation (eh, stype);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            BSplineCurveHandler::_QueryProperties (ElementHandleCR eh, PropertyContextR context)
    {
    // NOTE: For match ignore pole element which is a displayable and just report on hdr...
    if (QueryPropertyPurpose::Match == context.GetIQueryPropertiesP ()->_GetQueryPropertiesPurpose ())
        T_Super::_QueryHeaderProperties (eh, context);
    else
        T_Super::_QueryProperties (eh, context);

    DisplayHandler::QueryThicknessProperty (eh, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            BSplineCurveHandler::_EditProperties (EditElementHandleR eeh, PropertyContextR context)
    {
    T_Super::_EditProperties (eeh, context);

    DisplayHandler::EditThicknessProperty (eeh, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/06
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   BSplineCurveHandler::_ValidateElementRange (EditElementHandleR elHandle)
    {
    // For interpolation curves cannot use range of pole element, compute range by drawing...
    if (BSPLINE_CONSTRUCTION_INTERPOLATION == elHandle.GetElementCP ()->ToBspline_curve().flags.construct_type)
        return DisplayHandler::_ValidateElementRange (elHandle);

    // Need to draw to get correct range for extrude thickness linkage...
    if (mdlElement_attributePresent (elHandle.GetElementCP (), LINKAGEID_Thickness, NULL))
        return DisplayHandler::_ValidateElementRange (elHandle);

    return T_Super::_ValidateElementRange (elHandle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    10/06
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus BSplineCurveHandler::_OnGeoCoordinateReprojection (EditElementHandleR source, IGeoCoordinateReprojectionHelper& reprojectionHelper, bool inChain)
    {
    IGeoCoordinateReprojectionSettingsP settings = reprojectionHelper.GetSettings();
    if (reprojectionHelper.ShouldStroke (source, settings->StrokeCurves()))
        return GeoCoordinateReprojectionStrokeElement (source, reprojectionHelper, inChain);
    else
        {
        ReprojectStatus status = REPROJECT_Success;

        for (ChildEditElemIter childIter (source, ExposeChildrenReason::Count); childIter.IsValid(); )
            {
            ChildEditElemIter nextChild = childIter.ToNext();
            DisplayHandlerP  childHandler;
            if (NULL != (childHandler = childIter.GetDisplayHandler()))
                {
                ReprojectStatus childStatus;
                if (REPROJECT_Success != (childStatus = childHandler->GeoCoordinateReprojection (childIter, reprojectionHelper, true)))
                    status = childStatus;
                childHandler->ValidateElementRange (childIter);
                }
            childIter = nextChild;
            }

        return status;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            BSplineCurveHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_BSPLINE_CURVE_ELM));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawBCurvePolygon (ElementHandleCR thisElm, ViewContextR context)
    {
    switch (context.GetDrawPurpose ())
        {
        case DrawPurpose::Pick:
        case DrawPurpose::FenceAccept:
            break; // Want for locate...

        case DrawPurpose::FitView:
            {
            if (!thisElm.GetElementCP ()->ToBspline_curve().flags.curve_display)
                break; // Want if only poly is displayed...

            return;
            }

        default:
            {
            if (context.GetIViewDraw().IsOutputQuickVision ())
                break; // Want for display/plot...

            return;
            }
        }

    // host may override polygon display a la ACAD's "splframe" global
    switch (T_HOST.GetGraphicsAdmin()._GetControlPolyDisplay ())
        {
        case DgnPlatformLib::Host::GraphicsAdmin::CONTROLPOLY_DISPLAY_Always:
            break;

        case DgnPlatformLib::Host::GraphicsAdmin::CONTROLPOLY_DISPLAY_Never:
            return;

        default: // CONTROLPOLY_DISPLAY_ByElement
            {
            if (!thisElm.GetElementCP()->ToBspline_curve().flags.poly_display)
                return;
            }
        }

    IDrawGeomR      output = context.GetIDrawGeom();
    IPickGeom*      pick   = context.GetIPickGeom();
    GeomDetailP     detail = pick ? &pick->GetGeomDetail() : NULL;

    ElemDisplayParamsP currDisplayParams = context.GetCurrentDisplayParams();

    // For clarity's sake, never display the pole polygon for an interpolation curve.
    // Just display fat interpolation points & end tangent segments, if nontrivial.
    if (BSPLINE_CONSTRUCTION_INTERPOLATION == thisElm.GetElementCP()->ToBspline_curve().flags.construct_type)
        {
        MSInterpolationCurve    fitCurve;

        if (SUCCESS != BSplineCurveHandler::InterpolationCurveFromElement (fitCurve, thisElm))
            return;

        if (NULL != detail)
            detail->SetElemArg (BCURVE_ELEMARG_FitCurvePolygon);

        // Display fat dots for interpolation curve points...
        currDisplayParams->SetWeight (5);
        currDisplayParams->SetLineStyle (0);
        context.CookDisplayParams();

        output.DrawPointString3d (fitCurve.params.numPoints, fitCurve.fitPoints, NULL);

        if (!fitCurve.params.isPeriodic)
            {
            DPoint3d    tangentPoints[4];

            // Compute interpolation curve tangent points...
            computeInterpolationCurveTangentPoints (&tangentPoints[0], &tangentPoints[2], fitCurve);

            // Display fat dots for start/end tangent points...
            output.DrawPointString3d (1, &tangentPoints[0], NULL);
            output.DrawPointString3d (1, &tangentPoints[2], NULL);

            // Display dotted style start/end tangent lines...
            currDisplayParams->SetWeight (0);
            currDisplayParams->SetLineStyle (2);
            context.CookDisplayParams();

            tangentPoints[1] = fitCurve.fitPoints[0];
            tangentPoints[3] = fitCurve.fitPoints[fitCurve.params.numPoints-1];

            if (NULL != detail)
                detail->SetElemArg (BCURVE_ELEMARG_StartTangent);

            output.DrawLineString3d (2, &tangentPoints[0], NULL);

            if (NULL != detail)
                detail->SetElemArg (BCURVE_ELEMARG_EndTangent);

            output.DrawLineString3d (2, &tangentPoints[2], NULL);
            }

        fitCurve.ReleaseMem ();
        }
    else
        {
        MSBsplineCurve  curve;

        if (!readToCurve (thisElm, curve))
            return;

        if (NULL != detail)
            detail->SetElemArg (BCURVE_ELEMARG_ControlPolygon);

        if (curve.rational)
            bsputil_unWeightPoles (curve.poles, curve.poles, curve.weights, curve.params.numPoles);

        // Display dotted style control polygon...
        currDisplayParams->SetWeight (0);
        currDisplayParams->SetLineStyle (2);
        context.CookDisplayParams();

        if (curve.params.closed)
            {
            // Last point IS duplicated...
            if (mdlBspline_curveShouldBeOpened (&curve))
                {
                output.DrawShape3d (curve.params.numPoles, curve.poles, false, NULL);
                }
            else
                {
                DPoint3d    *ptsP = (DPoint3d *) alloca ((curve.params.numPoles+1) * sizeof (DPoint3d));

                memcpy (ptsP, curve.poles, curve.params.numPoles * sizeof (DPoint3d));
                ptsP[curve.params.numPoles] = ptsP[0];

                output.DrawShape3d (curve.params.numPoles+1, ptsP, false, NULL);
                }
            }
        else
            {
            output.DrawLineString3d (curve.params.numPoles, curve.poles, NULL);
            }

        // Display fat dots for poles...
        currDisplayParams->SetWeight (5);
        currDisplayParams->SetLineStyle (0);
        context.CookDisplayParams();

        output.DrawPointString3d (curve.params.numPoles, curve.poles, NULL);

        curve.ReleaseMem ();
        }

    if (NULL != detail)
        detail->SetElemArg (0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            BSplineCurveHandler::_Draw (ElementHandleCR thisElm, ViewContextR context)
    {
    UInt32      info = context.GetDisplayInfo (IsRenderable (thisElm));

    if (DISPLAY_INFO_None == info)
        return;

    if (0 != (info & DISPLAY_INFO_Edge) && !thisElm.GetElementCP ()->ToBspline_curve().flags.curve_display)
        info &= ~DISPLAY_INFO_Edge; 

    context.DrawCurveVector (thisElm, *this, (GeomRepresentations) info, true);

    // NOTE: Changes current matsymb...do not include in cached representations!
    if (!context.CheckICachedDraw ())
        drawBCurvePolygon (thisElm, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  02/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BSplineCurveHandler::_IsRenderable (ElementHandleCR eh)
    {
    return isClosedCurve (eh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   BSplineCurveHandler::_GetCurveVector (ElementHandleCR eh, CurveVectorPtr& curves)
    {
    if (BSPLINE_CURVE_ELM != eh.GetLegacyType())
        return ERROR;

    if (BSPLINE_CONSTRUCTION_INTERPOLATION == eh.GetElementCP ()->ToBspline_curve().flags.construct_type)
        {
        MSInterpolationCurve fitCurve;

        if (SUCCESS != BSplineCurveHandler::InterpolationCurveFromElement (fitCurve, eh))
            return ERROR;

        curves = CurveVector::Create (isClosedCurve (eh) ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open);
        curves->push_back (ICurvePrimitive::CreateInterpolationCurve (fitCurve));
        fitCurve.ReleaseMem ();

        return SUCCESS;
        }

    MSBsplineCurve  bcurve;

    if (!readToCurve (eh, bcurve))
        return ERROR;

    curves = CurveVector::Create (isClosedCurve (eh) ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open);
    curves->push_back (ICurvePrimitive::CreateBsplineCurve (bcurve));
    bcurve.ReleaseMem ();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   BSplineCurveHandler::_SetCurveVector (EditElementHandleR eeh, CurveVectorCR path)
    {
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve == path.HasSingleCurvePrimitive ())
        {
        MSInterpolationCurveCP  fitCurve = path.front ()->GetInterpolationCurveCP ();
        EditElementHandle       newEeh;

        // NOTE: In case eeh is component use ReplaceElementDescr, Create methods uses SetElementDescr...
        if (BSPLINE_STATUS_Success != BSplineCurveHandler::CreateBSplineCurveElement (newEeh, &eeh, *fitCurve, eeh.GetElementCP ()->Is3d(), *eeh.GetDgnModelP ()))
            return ERROR;

        return (BentleyStatus) eeh.ReplaceElementDescr (newEeh.ExtractElementDescr().get());
        }

    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve != path.HasSingleCurvePrimitive ())
        return ERROR;

    MSBsplineCurveCP    curve = path.front ()->GetBsplineCurveCP ();
    EditElementHandle   newEeh;

    // NOTE: In case eeh is component use ReplaceElementDescr, Create methods uses SetElementDescr...
    if (BSPLINE_STATUS_Success != BSplineCurveHandler::CreateBSplineCurveElement (newEeh, &eeh, *curve, eeh.GetElementCP ()->Is3d(), *eeh.GetDgnModelP ()))
        return ERROR;

    return (BentleyStatus) eeh.ReplaceElementDescr (newEeh.ExtractElementDescr().get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BSplineCurveHandler::_GetAreaType (ElementHandleCR eh, bool* isHoleP) const
    {
    if (!isClosedCurve (eh))
        return false;

    return IAreaFillPropertiesQuery::_GetAreaType (eh, isHoleP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BSplineCurveHandler::_GetSolidFill (ElementHandleCR eh, UInt32* fillColorP, bool* alwaysFilledP) const
    {
    if (!isClosedCurve (eh))
        return false;

    return IAreaFillPropertiesEdit::_GetSolidFill (eh, fillColorP, alwaysFilledP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BSplineCurveHandler::_GetGradientFill (ElementHandleCR eh, GradientSymbPtr& symb) const
    {
    if (!isClosedCurve (eh))
        return false;

    return IAreaFillPropertiesEdit::_GetGradientFill (eh, symb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BSplineCurveHandler::_GetPattern (ElementHandleCR eh, PatternParamsPtr& params, bvector<DwgHatchDefLine>* hatchDefLinesP, DPoint3dP originP, int index) const
    {
    if (!isClosedCurve (eh))
        return false;

    return IAreaFillPropertiesEdit::_GetPattern (eh, params, hatchDefLinesP, originP, index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BSplineCurveHandler::_SetAreaType (EditElementHandleR eeh, bool isHole)
    {
    if (!isClosedCurve (eeh))
        return false;

    return IAreaFillPropertiesEdit::_SetAreaType (eeh, isHole);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BSplineCurveHandler::_RemoveAreaFill (EditElementHandleR eeh)
    {
    if (!isClosedCurve (eeh))
        return false;

    return IAreaFillPropertiesEdit::_RemoveAreaFill (eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BSplineCurveHandler::_RemovePattern (EditElementHandleR eeh, int index)
    {
    if (!isClosedCurve (eeh))
        return false;

    return IAreaFillPropertiesEdit::_RemovePattern (eeh, index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BSplineCurveHandler::_AddSolidFill (EditElementHandleR eeh, UInt32* fillColorP, bool* alwaysFilledP)
    {
    if (!isClosedCurve (eeh))
        return false;

    return IAreaFillPropertiesEdit::_AddSolidFill (eeh, fillColorP, alwaysFilledP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BSplineCurveHandler::_AddGradientFill (EditElementHandleR eeh, GradientSymbCR symb)
    {
    if (!isClosedCurve (eeh))
        return false;

    return IAreaFillPropertiesEdit::_AddGradientFill (eeh, symb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BSplineCurveHandler::_AddPattern (EditElementHandleR eeh, PatternParams& params, DwgHatchDefLine* hatchDefLinesP, int index)
    {
    if (!isClosedCurve (eeh))
        return false;

    return IAreaFillPropertiesEdit::_AddPattern (eeh, params, hatchDefLinesP, index);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct          BPolePropertiesFilter : public ProcessPropertiesFilter
{
public:

BPolePropertiesFilter (IQueryProperties* queryObj) : ProcessPropertiesFilter (queryObj) {}
BPolePropertiesFilter (IEditProperties*  editObj)  : ProcessPropertiesFilter (editObj)  {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            _EachLevelCallback (EachLevelArg& arg) override
    {
    arg.SetPropertyFlags ((PropsCallbackFlags) (PROPSCALLBACK_FLAGS_UndisplayedID | arg.GetPropertyFlags ()));
    m_callbackObj->_EachLevelCallback (arg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            _EachColorCallback (EachColorArg& arg) override
    {
    arg.SetPropertyFlags ((PropsCallbackFlags) (PROPSCALLBACK_FLAGS_UndisplayedID | arg.GetPropertyFlags ()));
    m_callbackObj->_EachColorCallback (arg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            _EachLineStyleCallback (EachLineStyleArg& arg) override
    {
    arg.SetPropertyFlags ((PropsCallbackFlags) (PROPSCALLBACK_FLAGS_UndisplayedID | arg.GetPropertyFlags ()));
    m_callbackObj->_EachLineStyleCallback (arg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            _EachWeightCallback (EachWeightArg& arg) override
    {
    arg.SetPropertyFlags ((PropsCallbackFlags) (PROPSCALLBACK_FLAGS_UndisplayedID | arg.GetPropertyFlags ()));
    m_callbackObj->_EachWeightCallback (arg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            _EachElementClassCallback (EachElementClassArg& arg) override
    {
    arg.SetPropertyFlags ((PropsCallbackFlags) (PROPSCALLBACK_FLAGS_UndisplayedID | arg.GetPropertyFlags ()));
    m_callbackObj->_EachElementClassCallback (arg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            _EachTransparencyCallback (EachTransparencyArg& arg) override
    {
    arg.SetPropertyFlags ((PropsCallbackFlags) (PROPSCALLBACK_FLAGS_UndisplayedID | arg.GetPropertyFlags ()));
    m_callbackObj->_EachTransparencyCallback (arg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            _EachDisplayPriorityCallback (EachDisplayPriorityArg& arg) override
    {
    arg.SetPropertyFlags ((PropsCallbackFlags) (PROPSCALLBACK_FLAGS_UndisplayedID | arg.GetPropertyFlags ()));
    m_callbackObj->_EachDisplayPriorityCallback (arg);
    }

}; // BPolePropertiesFilter

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Evan.Williams                  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            BSplinePoleHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_BSPLINE_POLE_ELM));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BSplinePoleHandler::_IsSupportedOperation (ElementHandleCP eh, SupportOperation stype)
    {
    // NOTE: Pole element should really be treated as a non-displayable.
    return Handler::_IsSupportedOperation (eh, stype);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            BSplinePoleHandler::_QueryProperties (ElementHandleCR eh, PropertyContextR context)
    {
    // NOTE: Need to set "undisplayed" flags....redundant pole symbology gets out of synch...should not be displayable element!
    IQueryProperties*       queryObj = context.GetIQueryPropertiesP ();
    BPolePropertiesFilter   filterObj (queryObj);

    context.SetIQueryPropertiesP (&filterObj);
    T_Super::_QueryProperties (eh, context);
    context.SetIQueryPropertiesP (queryObj);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            BSplinePoleHandler::_EditProperties (EditElementHandleR eeh, PropertyContextR context)
    {
    // NOTE: Need to set "undisplayed" flags....redundant pole symbology gets out of synch...should not be displayable element!
    IEditProperties*        editObj = context.GetIEditPropertiesP ();
    BPolePropertiesFilter   filterObj (editObj);

    context.SetIEditPropertiesP (&filterObj);
    T_Super::_EditProperties (eeh, context);
    context.SetIEditPropertiesP (editObj);
    }

/*---------------------------------------------------------------------------------**//**
* This method is NOT used during normal display. Instead, bspline poles are drawn by their containing
* curve/surface (if they're displayable.) This method exists solely for the case where we ask to calculate the
* range of a bspline pole.
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            BSplinePoleHandler::_Draw (ElementHandleCR source, ViewContextR context)
    {
    DgnElementCP el = source.GetElementCP();

    if (DisplayHandler::Is3dElem (el))
        {
        context.DrawStyledLineString3d (el->ToLine_String_3d().numverts, el->ToLine_String_3d().vertice, &el->GetRange().low, false);
        }
    else
        {
        DPoint2d    range[2];
        DisplayHandler::GetDPRange(range, &el->GetRange());
        context.DrawStyledLineString2d (el->ToLine_String_2d().numverts, el->ToLine_String_2d().vertice, context.GetDisplayPriority(), range, false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BSplinePoleHandler::_OnTransform
(
EditElementHandleR eeh,
TransformInfoCR trans
)
    {
    if (SUCCESS != T_Super::_OnTransform (eeh, trans))
        return ERROR;

    BeAssert (BSPLINE_POLE_ELM == eeh.GetLegacyType());

    return LineStringBaseHandler::TransformLineString (eeh, trans, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    Handler                                 Brien.Bastings  09/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BSplineCurveHandler::_OnFenceClip
(
ElementAgendaP      inside,
ElementAgendaP      outside,
ElementHandleCR     elemHandle,
FenceParamsP        fp,
FenceClipFlags      options
)
    {
    fp->ParseAcceptedElement (inside, outside, elemHandle);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            BSplinePoleHandler::_OnConvertTo3d (EditElementHandleR eeh, double elevation)
    {
    DgnV8ElementBlank   elm;

    eeh.GetElementCP ()->CopyTo (elm);

    int         numpoles = elm.ToBspline_pole_2d().numpoles;

    elm.SetSizeWordsNoAttributes((offsetof (Bspline_pole_3d, poles) + numpoles * sizeof (DPoint3d)) / 2);
    elm.ToBspline_pole_3dR().numpoles = numpoles;
    DataConvert::Points2dTo3d (elm.ToBspline_pole_3dR().poles, eeh.GetElementCP ()->ToBspline_pole_2d().poles, numpoles, elevation);
    ElementUtil::CopyAttributes (&elm, eeh.GetElementCP ());

    eeh.ReplaceElement (&elm);

    T_Super::_OnConvertTo3d (eeh, elevation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            BSplinePoleHandler::_OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    // Pre-transform to "flatten" element and known linkages...
    TransformInfo   tInfo (flattenTrans);

    eeh.GetHandler().ApplyTransform (eeh, tInfo);

    DgnV8ElementBlank   elm;

    eeh.GetElementCP ()->CopyTo (elm);

    int         numpoles = elm.ToBspline_pole_3d().numpoles;

    elm.SetSizeWords((offsetof (Bspline_pole_2d, poles) + numpoles * sizeof (DPoint2d)) / 2);
    elm.ToBspline_pole_2dR().numpoles = numpoles;
    DataConvert::Points3dTo2d (elm.ToBspline_pole_2dR().poles, eeh.GetElementCP ()->ToBspline_pole_3d().poles, numpoles);
    ElementUtil::CopyAttributes (&elm, eeh.GetElementCP ());

    eeh.ReplaceElement (&elm);

    T_Super::_OnConvertTo2d (eeh, flattenTrans, flattenDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    10/06
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus BSplinePoleHandler::_OnGeoCoordinateReprojection (EditElementHandleR source, IGeoCoordinateReprojectionHelper& reprojectionHelper, bool inChain)
    {
    ReprojectStatus status  = REPROJECT_Success;
    DgnElementP      el      = source.GetElementP();
    bool            is3d    = Is3dElem (el);

    if (is3d)
        status = reprojectionHelper.ReprojectPoints (el->ToLine_String_3dR().vertice, NULL, NULL, el->ToLine_String_3d().vertice, el->ToLine_String_3d().numverts);
    else
        status = reprojectionHelper.ReprojectPoints2D (el->ToLine_String_2dR().vertice, NULL, NULL, el->ToLine_String_2d().vertice, el->ToLine_String_2d().numverts);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BSplinePoleHandler::_OnFenceStretch
(
EditElementHandleR  elemHandle,
TransformInfoCR     transform,
FenceParamsP        fp,
FenceStretchFlags   options
)
    {
    BeAssert (BSPLINE_POLE_ELM == elemHandle.GetLegacyType());

    return LineStringUtil::FenceStretch (elemHandle, transform, fp, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       HitPath::GetBSplineParameters
(
double          *uParamP,           // <=
double          *vParamP            // <=
)   const
    {
    if (uParamP)
        *uParamP = 0.0;

    if (vParamP)
        *vParamP = 0.0;

    ElementRefP         elemRef = GetCursorElem ();
    GeomDetail const&   detail = GetGeomDetail();

    if (BSPLINE_SURFACE_ELM == elemRef->GetLegacyType() && (uParamP || vParamP))
        {
        ElementHandle       elHandle (elemRef);
        MSBsplineSurface    bSurface;

        if (SUCCESS == BSplineSurfaceHandler::SurfaceFromElement (bSurface, elHandle))
            {
            DPoint2d    param;

            DPoint3d    closePt;
            
            m_geomDetail.GetClosestPoint (closePt);
            DPoint3d xyz;
            bSurface.ClosestPoint (xyz, param, closePt);
            if (uParamP)
                *uParamP = param.x;

            if (vParamP)
                *vParamP = param.y;

            bSurface.ReleaseMem ();
            }
        }
    else if (uParamP)
        {
        *uParamP = detail.GetCloseParam ();
        }

    // If hit generated from the control polygon should get linear params
    switch (elemRef->GetLegacyType())
        {
        case BSPLINE_CURVE_ELM:
            {
            switch (detail.GetElemArg ())
                {
                case BCURVE_ELEMARG_ControlPolygon:
                case BCURVE_ELEMARG_FitCurvePolygon:
                case BCURVE_ELEMARG_StartTangent:
                case BCURVE_ELEMARG_EndTangent:
                    return ERROR;
                }
            break;
            }

        case BSPLINE_SURFACE_ELM:
            {
            switch (detail.GetElemArg ())
                {
                case BSURF_ELEMARG_ControlPolygon:
                    return ERROR;
                }
            break;
            }
        }

    return SUCCESS;
    }

