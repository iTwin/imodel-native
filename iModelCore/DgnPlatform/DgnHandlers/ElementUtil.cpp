/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/ElementUtil.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#define PARTIAL_TOLERANCE       1.0E-3

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/10
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus partialCurveToElement
(
EditElementHandleR  eeh,
ElementHandleCR     eh,
CurveVectorPtr&     partialCurve,
bool                simplify
)
    {
    if (!partialCurve.IsValid ())
        return ERROR;

    if (simplify)
        partialCurve->ConsolidateAdjacentPrimitives ();

    ICurvePathEdit* pathEdit;

    if (NULL != (pathEdit = dynamic_cast <ICurvePathEdit*> (&eh.GetHandler ())))
        {
        eeh.Duplicate (eh);

        if (SUCCESS == pathEdit->SetCurveVector (eeh, *partialCurve.get ()))
            return SUCCESS;

        eeh.Invalidate ();
        }

    return DraftingElementSchema::ToElement (eeh, *partialCurve, &eh, eh.GetElementCP ()->Is3d(), *eh.GetDgnModelP ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/10
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus partialDeleteElement
(
EditElementHandleR  outEeh1,
EditElementHandleR  outEeh2,
ElementHandleCR     eh,
DPoint3dCP          pointF,
DPoint3dCP          pointL,
DPoint3dCP          pointD,
DVec3dP             dirVec,             // <=> Output when computeDirection
bool                computeDirection,
ViewportP           vp
)
    {
    CurveVectorPtr  pathCurve = ICurvePathQuery::ElementToCurveVector (eh);

    if (!pathCurve.IsValid ())
        return ERROR;

    bool        isOpenPath   = pathCurve->IsOpenPath ();
    bool        isClosedPath = pathCurve->IsClosedPath ();

    if (!isOpenPath && !isClosedPath)
        return ERROR;

    DPoint3d            startPt, endPt;
    CurveLocationDetail startLocation, endLocation;

    if (!pathCurve->GetStartEnd (startPt, endPt) ||
        !pathCurve->ClosestPointBounded (startPt, startLocation) ||
        !pathCurve->ClosestPointBounded (endPt, endLocation))
        return ERROR;


    DMap4d      elemToRoot, rootToView;

    elemToRoot.InitIdentity();

    if (vp)
        rootToView = *vp->GetWorldToViewMap ();
    else
        rootToView.InitIdentity ();

    DPoint3d    firstPt, finalPt;

    if (pointF)
        {
        firstPt = *pointF;
        elemToRoot.M1.MultiplyAndRenormalize (&firstPt, &firstPt, 1);
        }
    else
        {
        firstPt = startPt;
        }

    if (pointL)
        {
        finalPt = *pointL;
        elemToRoot.M1.MultiplyAndRenormalize (&finalPt, &finalPt, 1);
        }
    else
        {
        finalPt = endPt;
        }

    DMap4d      viewToElem;

    viewToElem.InitProduct (rootToView, elemToRoot);

    CurveLocationDetail firstLocation, finalLocation;

    if (!pathCurve->ClosestPointBoundedXY (firstPt, &viewToElem.M0, firstLocation) ||
        !pathCurve->ClosestPointBoundedXY (finalPt, &viewToElem.M0, finalLocation))
        return ERROR;

    bool    reversed, simplify = (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Invalid != pathCurve->HasSingleCurvePrimitive ());

    if (reversed = (1 == pathCurve->CurveLocationDetailCompare (firstLocation, finalLocation)))
        {
        CurveLocationDetail tmpLocation = firstLocation;

        firstLocation = finalLocation;
        finalLocation = tmpLocation;
        }

    if (isClosedPath && (pointD || dirVec || computeDirection))
        {
        bool        invalidDirection = false;
        DPoint3d    dirPt;

        if (pointD)
            {
            dirPt = *pointD;
            elemToRoot.M1.MultiplyAndRenormalize (&dirPt, &dirPt, 1);
            }
        else if (computeDirection)
            {
            DVec3d  tmpDVec;
            double  len = tmpDVec.NormalizedDifference (firstLocation.point, finalLocation.point);

            if (!reversed)
                tmpDVec.Negate ();

            dirPt.SumOf (firstPt, tmpDVec, PARTIAL_TOLERANCE);

            if (dirVec)
                *dirVec = tmpDVec;

            invalidDirection = (len < 1.0e-5);
            }
        else if (dirVec)
            {
            dirPt.SumOf (firstPt, *dirVec, PARTIAL_TOLERANCE);
            }

        CurveLocationDetail dirLocation;

        if (!pathCurve->ClosestPointBoundedXY (dirPt, &viewToElem.M0, dirLocation))
            return ERROR;

        int     wrapCount = (invalidDirection || (-1 == pathCurve->CurveLocationDetailCompare (firstLocation, dirLocation) && -1 == pathCurve->CurveLocationDetailCompare (dirLocation, finalLocation)) ? (int) pathCurve->size () : 0);

        CurveVectorPtr  partialCurve = pathCurve->CloneBetweenCyclicIndexedFractions ((int) pathCurve->CurveLocationDetailIndex (firstLocation)+wrapCount, firstLocation.fraction, (int) pathCurve->CurveLocationDetailIndex (finalLocation), finalLocation.fraction);

        partialCurveToElement (outEeh1, eh, partialCurve, simplify);
        }
    else
        {
        CurveVectorPtr  partialCurve1 = pathCurve->CloneBetweenDirectedFractions ((int) pathCurve->CurveLocationDetailIndex (startLocation), startLocation.fraction, (int) pathCurve->CurveLocationDetailIndex (firstLocation), firstLocation.fraction, false);

        partialCurveToElement (outEeh1, eh, partialCurve1, simplify);

        CurveVectorPtr  partialCurve2 = pathCurve->CloneBetweenDirectedFractions ((int) pathCurve->CurveLocationDetailIndex (finalLocation), finalLocation.fraction, (int) pathCurve->CurveLocationDetailIndex (endLocation), endLocation.fraction, false);

        partialCurveToElement (outEeh2, eh, partialCurve2, simplify);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CurveVectorUtil::PartialDeleteElement
(
EditElementHandleR  outEeh1,
EditElementHandleR  outEeh2,
ElementHandleCR     eh,
DPoint3dCP          pointF,
DPoint3dCP          pointL,
DPoint3dCP          pointD,
ViewportP           vp
)
    {
    return partialDeleteElement (outEeh1, outEeh2, eh, pointF, pointL, pointD, NULL, false, vp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CurveVectorUtil::PartialDeleteElement
(
EditElementHandleR  outEeh1,
EditElementHandleR  outEeh2,
ElementHandleCR     eh,
DPoint3dCP          pointF,
DPoint3dCP          pointL,
DVec3dP             dirVec,             // <=> Output when computeDirection
bool                computeDirection,
ViewportP           vp
)
    {
    return partialDeleteElement (outEeh1, outEeh2, eh, pointF, pointL, NULL, dirVec, computeDirection, vp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             07/86
+---------------+---------------+---------------+---------------+---------------+------*/
bool BentleyApi::in_span (double theta, double start, double sweep)
    {
    theta -= start;

    if (fabs (theta) > 62.8) return (0);

    while (theta < 0.0) theta += msGeomConst_2pi;
    while (theta > msGeomConst_2pi) theta -= msGeomConst_2pi;
    if (sweep > 0.0)
        {
        if (sweep >= theta)
            return (true);
        return (false);
        }
    else
        {
        if (theta >= (msGeomConst_2pi + sweep))
            return (true);
        return (false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/09
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementUtil::InitScanRangeForUnion (DRange3dR range, bool is3d)
    {
    range.Init();
    if (!is3d)
        {
        range.low.z = range.high.z = 0.0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementUtil::StripAttributes (DgnElementR elem)
    {
    int attributeLength = elem.GetSizeWords() - elem.GetAttributeOffset();
    elem.SetSizeWords(elem.GetSizeWords() - attributeLength);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ElementUtil::AppendAttributes (DgnElementR elem, UInt16 *bufferP, int attribWords)
    {
    if (attribWords <= 0)
        return ERROR;
    
    // append the attribute data to the END of any existing attribute data 
    short *endP = (short *) &elem + elem.GetSizeWords();
    memcpy (endP, bufferP, attribWords * sizeof (short));
    elem.SetSizeWords(elem.GetSizeWords() + attribWords); // now fix up the words to follow
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
int             ElementUtil::ExtractAttributes (UInt16 *bufferP, int maxAttriutebSize, DgnElementCR srcElm)
    {
    int attribWords = srcElm.GetSizeWords() - srcElm.GetAttributeOffset();
    if (attribWords <= 0) // do nothing if attribute property bit not set
        return 0;
    
    if (attribWords > maxAttriutebSize)
        attribWords = maxAttriutebSize; // this is bad data?!?

    // now copy the attribute data to the user's buffer
    short const *startP = (short const *) &srcElm + srcElm.GetAttributeOffset();
    memcpy (bufferP, startP, attribWords * sizeof (short));
    return attribWords;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementUtil::CopyAttributes (DgnElementP dstElmP, DgnElementCP srcElmP)
    {
    if (!srcElmP)
        return;

    int         attribWords;

    if (0 >= (attribWords = (srcElmP->GetSizeWords() - srcElmP->GetAttributeOffset())))
        return;

    memcpy (((short *) dstElmP) + dstElmP->GetSizeWords(), ((short *) srcElmP) + srcElmP->GetAttributeOffset(), attribWords * 2);
    dstElmP->SetSizeWords(dstElmP->GetSizeWords() + attribWords);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/00
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementUtil::SetRequiredFields (DgnElementR u, LevelId level, bool is3d)
    {
    u.SetLevel(level.GetValueUnchecked());
    u.InvalidateElementId(); // clear id if template was supplied...
    u.SetIs3d(is3d);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/00
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementUtil::SetRequiredFields (DgnElementR out, ElementHandleCP templateEh, int defaultSize, bool copyAttributes, bool initScanRange, bool is3d, DgnModelP modelRef)
    {
    DgnElementCP in = (templateEh ? templateEh->GetElementCP () : NULL);
    if (in)
        {
        in->CopyTo (out);
        ElementUtil::SetRequiredFields (out, in->GetLevel(), is3d);
        }
    else
        {
        memset (&out, 0, defaultSize);
        ElementUtil::SetRequiredFields (out, LEVEL_DEFAULT_LEVEL_ID, is3d);
        }

    out.InvalidateElementId();
    out.SetSizeWordsNoAttributes(defaultSize/2);

    if (copyAttributes)
        ElementUtil::CopyAttributes (&out, in);
    if (initScanRange)
        ElementUtil::InitScanRangeForUnion (out.GetRangeR(), is3d);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     08/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void     addIntersectionPoints
(
bvector<DPoint3d>*isPnt1,
bvector<DPoint3d>*isPnt2,
DPoint3dCP      point1,
DPoint3dCP      point2,
int             numAdd
)
    {
    for (int i = 0; i < numAdd; i++)
        {
        if (isPnt1)
            isPnt1->push_back (point1[i]);

        if (isPnt2)
            isPnt2->push_back (point2[i]);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     08/09
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     isValidFraction (double f, bool extend)
    {
    static double fractionTol = 1.0e-10;

    return extend || (f > -fractionTol && f < 1.0 + fractionTol);
    }

/*---------------------------------------------------------------------------------**//**
* From local params in arc space, assign order (01 or 10) to match intersection point order as computed in V8.11 (and much older) ...
* @bsimethod                                                    Earlin.Lutz     08/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void     legacyArcLineIntersectionOrder (DVec3dP localParams, int count, int order[])
    {
    for (int i = 0; i < count; i++)
        order[i] = i;

    if (2 != count)
        return;

    double  dx = localParams[1].x - localParams[0].x;
    double  dy = localParams[1].y - localParams[0].y;
    bool    flip = false;

    if (fabs (dy) < bsiTrig_smallAngle ())
        flip = dx > 0.0;
    else
        flip = dy > 0.0;

    if (flip)
        {
        order[0] = 1;
        order[1] = 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Arc-line intersection with points sorted to match (we fervantly hope) order produced by l_intsec.c.
* @bsimethod                                                    Earlin.Lutz     08/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void     legacyArcLineIntersect
(
bvector<DPoint3d>*isPnt1,
bvector<DPoint3d>*isPnt2,
DEllipse3dR     ellipse,
DSegment3dR     segment,
bool            extend
)
    {
    int         order[2];
    double      fraction[2];
    DVec3d      ellipseParams[2];
    DPoint3d    linePoints[2];
    int         n1 = ellipse.intersectSweptDSegment3d (linePoints, ellipseParams, fraction, &segment);

    legacyArcLineIntersectionOrder (ellipseParams, n1, order);

    for (int k = 0; k < n1; k++)
        {
        int     i = order[k];

        if (isValidFraction (fraction[i], extend) && (extend || ellipse.isAngleInSweep (bsiTrig_atan2 (ellipseParams[i].y, ellipseParams[i].x))))
            {
            DPoint3d    ellipsePoint;

            ellipse.evaluate (&ellipsePoint, ellipseParams[i].x, ellipseParams[i].y);
            addIntersectionPoints (isPnt1, isPnt2, &ellipsePoint, &linePoints[i], 1);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* curve-curve intersections with special cases mimicing legacy code in l_intsec.c
* Segment*Arc -- mimic sort logic (both circular and elliptic arcs)
* Circle*Circle -- mimic sort logic 
* (Segment or Circle) * Curve - exact, in order along curve.
* All others -- exact, order along first gpa
* @bsimethod                                                    Earlin.Lutz     08/09
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus getIntersectionsWithSpecialCases
(
bvector<DPoint3d>*isPnt1,
bvector<DPoint3d>*isPnt2,
GPArrayP        gpa1P,
GPArrayP        gpa2P,
bool            extend
)
    {
    DSegment3d  segment1, segment2;
    DEllipse3d  ellipse1, ellipse2;
    DPoint3d    point1[4];
    DPoint3d    point2[4];
    double      fraction1[4];
    double      fraction2[4];
    int         selector = 1;   // Identifies the gpa that must be first in what used to be the "stroke" case.
                                // (We do the intersections exactly, but this selection is still needed to legacy order.)
    if (gpa1P->IsSingleDSegment3d (&segment1))
        {
        if (gpa2P->IsSingleDSegment3d (&segment2))
            {
            if (DSegment3d::closestApproach (fraction1, fraction2, point1, point2, &segment1, &segment2))
                {
                if (isValidFraction (fraction1[0], extend) && isValidFraction (fraction2[0], extend))
                    addIntersectionPoints (isPnt1, isPnt2, point1, point2, 1);
                }

            return SUCCESS;
            }
        else if (gpa2P->IsSingleDEllipse3d (&ellipse2))
            {
            legacyArcLineIntersect (isPnt2, isPnt1, ellipse2, segment1, extend);

            return SUCCESS;
            }
        else
            {
            selector = 2;
            }
        }
    else if (gpa1P->IsSingleDEllipse3d (&ellipse1))
        {
        if (gpa2P->IsSingleDSegment3d (&segment2))
            {
            legacyArcLineIntersect (isPnt1, isPnt2, ellipse1, segment2, extend);

            return SUCCESS;
            }
        else if (gpa2P->IsSingleDEllipse3d (&ellipse2) && bsiDEllipse3d_isParallelPlaneCircleCircleIntersect (isPnt1, isPnt2, NULL, NULL, &ellipse1, &ellipse2, 0.0))
            {
            return SUCCESS;
            }
        else
            {
            selector = 2;
            }
        }

    // If no special cases, do xyz closest approach within tolerance.
    static double s_absTol = 1.0e-12;
    static double s_relTol = 1.0e-10;

    double          tolerance = GPArray::GetTolerance (gpa1P, gpa2P, s_absTol, s_relTol);
    GPArraySmartP   collector1, collector2;

    if (1 == selector)
        gpa1P->GetClosestApproachPoints (*collector1, *collector2, *gpa2P, tolerance, false, false, tolerance);
    else
        gpa2P->GetClosestApproachPoints (*collector2, *collector1, *gpa1P, tolerance, false, false, tolerance);

    DPoint3d    xyz1, xyz2;

    for (size_t i0 = 0; (*collector1).GetNormalized (xyz1, (int) i0) && (*collector2).GetNormalized (xyz2, (int) i0); i0++)
        addIntersectionPoints (isPnt1, isPnt2, &xyz1, &xyz2, 1);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ElementUtil::GetIntersections
(
bvector<DPoint3d>*  isPnt1,
bvector<DPoint3d>*  isPnt2,
ElementHandleCR            eh1,
ElementHandleCR            eh2,
TransformCP             pathTrans1,
TransformCP             pathTrans2,
bool                    extendSegment
)
    {
    // See comment on {assoc.c}assoc_intersectionGeometryByIndex for why we use the "obsolete" function. \
    // Do not change this to use the new intersect function!
    GPArraySmartP   gpa1, gpa2;

    if (SUCCESS != gpa1->Add (eh1) || SUCCESS != gpa2->Add (eh2))
        return ERROR;

    if (pathTrans1)
        gpa1->Transform (pathTrans1);

    if (pathTrans2)
        gpa2->Transform (pathTrans2);

    return getIntersectionsWithSpecialCases (isPnt1, isPnt2, gpa1, gpa2, extendSegment);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ElementUtil::GetIntersectionPointByIndex
(
DPoint3dR       isPnt,
ElementHandleCR    eh1,
ElementHandleCR    eh2,
TransformCP     pathTrans1,
TransformCP     pathTrans2,
int             index
)
    {
    if (index < 0)
        {
        BeAssert (false);

        return ERROR;
        }

    bvector<DPoint3d> isPnts1, isPnts2;

    if (SUCCESS != ElementUtil::GetIntersections (&isPnts1, &isPnts2, eh1, eh2, pathTrans1, pathTrans2, true))
        return ERROR;

    int     nIntersect = (int) isPnts1.size ();

    if (index > nIntersect-1)
        return ERROR;

    // Make sure this is a real intersection (not apparent one)
    if (!LegacyMath::RpntEqual (&isPnts1[index], &isPnts2[index]))
        return ERROR;

    isPnt = isPnts1[index];

    return SUCCESS;
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       convertElementToSurface (EditElementHandleR surfElemHandle, ElementHandleCR eh)
    {
    ISolidPrimitivePtr  solidPrimitive = ISolidPrimitiveQuery::ElementToSolidPrimitive (eh, false);

    if (!solidPrimitive.IsValid ())
        return ERROR;

    bvector<MSBsplineSurfacePtr> surfaces;

    if (!MSBsplineSurface::CreateTrimmedSurfaces (surfaces, *solidPrimitive))
        return ERROR;

    if (1 == surfaces.size ())
        return BSplineSurfaceHandler::CreateBSplineSurfaceElement (surfElemHandle, &eh, *surfaces.front (), *eh.GetDgnModelP ());

    PhysicalModelP physModel = dynamic_cast<PhysicalModelP>(eh.GetDgnModelP ());
    if (NULL == physModel)
        return ERROR;

    PhysicalGraphicsPtr graphics = physModel->CreatePhysicalGraphics();
    for (auto& surface : surfaces)
        graphics->AddMSBsplineSurface(*surface);
    
    return graphics->SaveEntries(surfElemHandle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ElementUtil::NonUniformScaleAsBsplineSurf
(
bool&           wasHandled,
EditElementHandleR elemHandle,
TransformInfoCR trans,
RotMatrixP      pElementRMatrix // Allow nonuniform scaling if only in Z of this matrix.
)
    {
    StatusInt   status = SUCCESS;
    double      ratio = 1.0;
    bool        ortho;
    RotMatrix   rMatrix;

    rMatrix.initFrom (trans.GetTransform ());

    // If an element matrix is provided, allow nonuniform along the Z axis only.
    if (NULL != pElementRMatrix)
        {
        DVec3d      columns[2];
        DVec3d      axisScales;

        pElementRMatrix->getColumn (&columns[0], 0);
        pElementRMatrix->getColumn (&columns[1], 1);

        rMatrix.multiply (columns, columns, 2);

        axisScales.x = columns[0].normalize();
        axisScales.y = columns[1].normalize();
        if (true == (ortho = columns[0].isPerpendicularTo (&columns[1])))
            ratio = (axisScales.x > axisScales.y) ? (axisScales.x / axisScales.y) : (axisScales.y / axisScales.x);
        }
    else
        {
        ortho = rMatrix.isOrthonormal (&rMatrix, NULL, &ratio);
        }

    // Non-uniform scale not allowed on cone/type 18/19 revolution...mirror is ok...
    if (true == (wasHandled = !(ortho && fabs (ratio - 1.0) < 1.0e-8)))
        {
        EditElementHandle  surfElemHandle;

        if (SUCCESS != convertElementToSurface (surfElemHandle, elemHandle))
            return ERROR;

        if (SUCCESS == (status = surfElemHandle.GetHandler().ApplyTransform (surfElemHandle, trans)))
            elemHandle.ReplaceElementDescr (surfElemHandle.ExtractElementDescr().get());
        }

    return status;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ElementUtil::GetIgnoreScaleDisplayTransforms
(
TransformP      newTopTransP,
TransformP      newScTransP,
ViewContextR    context
)
    {
    Transform   fwdTopLocalToFrustum, invTopLocalToFrustum, fwdRefLocalToFrustum, invRefLocalToFrustum;

    context.GetCurrLocalToFrustumTrans (fwdTopLocalToFrustum);
    context.GetLocalToFrustumTrans (fwdRefLocalToFrustum, context.GetRefTransClipDepth ());
    bsiTransform_invertTransform (&invTopLocalToFrustum, &fwdTopLocalToFrustum);
    bsiTransform_invertTransform (&invRefLocalToFrustum, &fwdRefLocalToFrustum);

    bsiTransform_multiplyTransformTransform (newTopTransP, &fwdRefLocalToFrustum, &invTopLocalToFrustum);
    bsiTransform_multiplyTransformTransform (newScTransP, &fwdTopLocalToFrustum, &invRefLocalToFrustum);

    return SUCCESS;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
GLOBAL_TYPEDEF (TextStyleLinkage, TextStyleLinkage)
struct TextStyleLinkage
    {
    LinkageHeader   linkHeader;

    // Anything below here added to the element incrementally as needed.
    UInt32          linkageKey;
    UInt32          modifiers;
    UInt32          extModifiers;

    // Reserve enough space for maximum number of parameters.
    byte            modData[sizeof (LegacyTextStyle)];
    
    }; // Textstylelinkage

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum TEXTSTYLE_PROP
    {
    TEXTSTYLE_PROP_FontNo               = 0,
    TEXTSTYLE_PROP_ShxBigFont           = 1,
    TEXTSTYLE_PROP_Width                = 2,
    TEXTSTYLE_PROP_Height               = 3,
    TEXTSTYLE_PROP_Slant                = 4,
    TEXTSTYLE_PROP_LineSpacing          = 5,
    TEXTSTYLE_PROP_InterCharSpacing     = 6,
    TEXTSTYLE_PROP_UnderlineOffset      = 7,
    TEXTSTYLE_PROP_OverlineOffset       = 8,
    TEXTSTYLE_PROP_WidthFactor          = 9,
    TEXTSTYLE_PROP_LineOffset           = 10,
    TEXTSTYLE_PROP_Just                 = 11,
    TEXTSTYLE_PROP_NodeJust             = 12,
    TEXTSTYLE_PROP_LineLength           = 13,
    TEXTSTYLE_PROP_TextDirection        = 14,
    TEXTSTYLE_PROP_BackgroundStyle      = 15,
    TEXTSTYLE_PROP_BackgroundWeight     = 16,
    TEXTSTYLE_PROP_BackgroundColor      = 17,
    TEXTSTYLE_PROP_BackgroundFillColor  = 18,
    TEXTSTYLE_PROP_BackgroundBorder     = 19,
    TEXTSTYLE_PROP_UnderlineStyle       = 20,
    TEXTSTYLE_PROP_UnderlineWeight      = 21,
    TEXTSTYLE_PROP_UnderlineColor       = 22,
    TEXTSTYLE_PROP_OverlineStyle        = 23,
    TEXTSTYLE_PROP_OverlineWeight       = 24,
    TEXTSTYLE_PROP_OverlineColor        = 25,
    TEXTSTYLE_PROP_ParentId             = 26,
    TEXTSTYLE_PROP_Flags                = 27,
    TEXTSTYLE_PROP_OverrideFlags        = 28,
    TEXTSTYLE_PROP_Color                = 29
    
    }; // TEXTSTYLE_PROP

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/02
+---------------+---------------+---------------+---------------+---------------+------*/
static byte* appendTextStyleLinkageData (byte& buffer, LegacyTextStyleCR textStyle)
    {
    byte* pBuffer = &buffer;
    
    LegacyTextStyleFlags emptyFlags;
    memset (&emptyFlags, 0, sizeof(emptyFlags));
    
    LegacyTextStyleOverrideFlags emptyOverrides;
    memset (&emptyOverrides, 0, sizeof(emptyOverrides));

    // Skip over modifiers - we will write them out last.
    UInt32  modifiers           = 0;
    UInt32  extModifiers        = 0;
    byte*   originalBufferHead  = pBuffer;
    
    pBuffer += sizeof (modifiers);
    pBuffer += sizeof (extModifiers);

    if (0 != textStyle.fontNo)                  {   ByteStreamHelper::AppendUInt32  (pBuffer, textStyle.fontNo);                    modifiers |= (0x0001 << TEXTSTYLE_PROP_FontNo);                 }
    if (0 != textStyle.shxBigFont)              {   ByteStreamHelper::AppendUInt32  (pBuffer, textStyle.shxBigFont);                modifiers |= (0x0001 << TEXTSTYLE_PROP_ShxBigFont);             }
    if (0.0 != textStyle.width)                 {   ByteStreamHelper::AppendDouble  (pBuffer, textStyle.width);                     modifiers |= (0x0001 << TEXTSTYLE_PROP_Width);                  }
    if (0.0 != textStyle.height)                {   ByteStreamHelper::AppendDouble  (pBuffer, textStyle.height);                    modifiers |= (0x0001 << TEXTSTYLE_PROP_Height);                 }
    if (0.0 != textStyle.slant)                 {   ByteStreamHelper::AppendDouble  (pBuffer, textStyle.slant);                     modifiers |= (0x0001 << TEXTSTYLE_PROP_Slant);                  }
    if (0.0 != textStyle.lineSpacing)           {   ByteStreamHelper::AppendDouble  (pBuffer, textStyle.lineSpacing);               modifiers |= (0x0001 << TEXTSTYLE_PROP_LineSpacing);            }
    if (0.0 != textStyle.interCharSpacing)      {   ByteStreamHelper::AppendDouble  (pBuffer, textStyle.interCharSpacing);          modifiers |= (0x0001 << TEXTSTYLE_PROP_InterCharSpacing);       }
    if (0.0 != textStyle.underlineOffset)       {   ByteStreamHelper::AppendDouble  (pBuffer, textStyle.underlineOffset);           modifiers |= (0x0001 << TEXTSTYLE_PROP_UnderlineOffset);        }
    if (0.0 != textStyle.overlineOffset)        {   ByteStreamHelper::AppendDouble  (pBuffer, textStyle.overlineOffset);            modifiers |= (0x0001 << TEXTSTYLE_PROP_OverlineOffset);         }
    if (0.0 != textStyle.widthFactor)           {   ByteStreamHelper::AppendDouble  (pBuffer, textStyle.widthFactor);               modifiers |= (0x0001 << TEXTSTYLE_PROP_WidthFactor);            }
    if (0.0 != textStyle.lineOffset.x ||
        0.0 != textStyle.lineOffset.y)          {   ByteStreamHelper::AppendDouble  (pBuffer, textStyle.lineOffset.x);
                                                    ByteStreamHelper::AppendDouble  (pBuffer, textStyle.lineOffset.y);              modifiers |= (0x0001 << TEXTSTYLE_PROP_LineOffset);             }
    if (0 != textStyle.just)                    {   ByteStreamHelper::AppendUInt16  (pBuffer, textStyle.just);                      modifiers |= (0x0001 << TEXTSTYLE_PROP_Just);                   }
    if (0 != textStyle.nodeJust)                {   ByteStreamHelper::AppendUInt16  (pBuffer, textStyle.nodeJust);                  modifiers |= (0x0001 << TEXTSTYLE_PROP_NodeJust);               }
    if (0 != textStyle.lineLength)              {   ByteStreamHelper::AppendUInt16  (pBuffer, textStyle.lineLength);                modifiers |= (0x0001 << TEXTSTYLE_PROP_LineLength);             }
    if (0 != textStyle.textDirection)           {   ByteStreamHelper::AppendUInt16  (pBuffer, textStyle.textDirection);             modifiers |= (0x0001 << TEXTSTYLE_PROP_TextDirection);          }
    if (0 != textStyle.backgroundStyle.style)   {   ByteStreamHelper::AppendInt32   (pBuffer, textStyle.backgroundStyle.style);     modifiers |= (0x0001 << TEXTSTYLE_PROP_BackgroundStyle);        }
    if (0 != textStyle.backgroundStyle.weight)  {   ByteStreamHelper::AppendUInt32  (pBuffer, textStyle.backgroundStyle.weight);    modifiers |= (0x0001 << TEXTSTYLE_PROP_BackgroundWeight);       }
    if (0 != textStyle.backgroundStyle.color)   {   ByteStreamHelper::AppendUInt32  (pBuffer, textStyle.backgroundStyle.color);     modifiers |= (0x0001 << TEXTSTYLE_PROP_BackgroundColor);        }
    if (0 != textStyle.backgroundFillColor)     {   ByteStreamHelper::AppendUInt32  (pBuffer, textStyle.backgroundFillColor);       modifiers |= (0x0001 << TEXTSTYLE_PROP_BackgroundFillColor);    }
    if (0.0 != textStyle.backgroundBorder.x ||
        0.0 != textStyle.backgroundBorder.y)    {   ByteStreamHelper::AppendDouble  (pBuffer, textStyle.backgroundBorder.x);
                                                    ByteStreamHelper::AppendDouble  (pBuffer, textStyle.backgroundBorder.y);        modifiers |= (0x0001 << TEXTSTYLE_PROP_BackgroundBorder);       }
    if (0 != textStyle.underlineStyle.style)    {   ByteStreamHelper::AppendInt32   (pBuffer, textStyle.underlineStyle.style);      modifiers |= (0x0001 << TEXTSTYLE_PROP_UnderlineStyle);         }
    if (0 != textStyle.underlineStyle.weight)   {   ByteStreamHelper::AppendUInt32  (pBuffer, textStyle.underlineStyle.weight);     modifiers |= (0x0001 << TEXTSTYLE_PROP_UnderlineWeight);        }
    if (0 != textStyle.underlineStyle.color)    {   ByteStreamHelper::AppendUInt32  (pBuffer, textStyle.underlineStyle.color);      modifiers |= (0x0001 << TEXTSTYLE_PROP_UnderlineColor);         }
    if (0 != textStyle.overlineStyle.style)     {   ByteStreamHelper::AppendInt32   (pBuffer, textStyle.overlineStyle.style);       modifiers |= (0x0001 << TEXTSTYLE_PROP_OverlineStyle);          }
    if (0 != textStyle.overlineStyle.weight)    {   ByteStreamHelper::AppendUInt32  (pBuffer, textStyle.overlineStyle.weight);      modifiers |= (0x0001 << TEXTSTYLE_PROP_OverlineWeight);         }
    if (0 != textStyle.overlineStyle.color)     {   ByteStreamHelper::AppendUInt32  (pBuffer, textStyle.overlineStyle.color);       modifiers |= (0x0001 << TEXTSTYLE_PROP_OverlineColor);          }
    
    if (0 != memcmp (&textStyle.flags, &emptyFlags, sizeof (textStyle.flags)))
        {
        UInt32* pLong = (UInt32*)&textStyle.flags;

        ByteStreamHelper::AppendUInt32 (pBuffer, *(pLong));
        modifiers |= (0x0001 << TEXTSTYLE_PROP_Flags);
        }

    if (0 != memcmp (&textStyle.overrideFlags, &emptyOverrides, sizeof (textStyle.overrideFlags)))
        {
        UInt32* value = (UInt32*)&textStyle.overrideFlags;

        ByteStreamHelper::AppendUInt32 (pBuffer, *(value + 0));
        ByteStreamHelper::AppendUInt32 (pBuffer, *(value + 1));
        modifiers |= (0x0001 << TEXTSTYLE_PROP_OverrideFlags);
        }

    if (textStyle.color)
        {
        ByteStreamHelper::AppendUInt32 (pBuffer, textStyle.color);
        modifiers |= (0x0001 << TEXTSTYLE_PROP_Color);
        }

    ByteStreamHelper::AppendUInt32 (originalBufferHead, modifiers);
    ByteStreamHelper::AppendUInt32 (originalBufferHead, extModifiers);

    return pBuffer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/02
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ElementUtil::AppendTextStyleAsLinkage (EditElementHandleR eeh, LegacyTextStyleCR textStyle, UInt32 linkageKey)
    {
    DeleteTextStyleLinkage (eeh, linkageKey);

    TextStyleLinkage linkage;
    memset (&linkage, 0, sizeof (linkage));

    linkage.linkHeader.user       = 1;
    linkage.linkHeader.primaryID  = LINKAGEID_TEXTSTYLE;
    linkage.linkageKey            = linkageKey;

    byte* pEnd = appendTextStyleLinkageData (*(byte*)&linkage.modifiers, textStyle);

    // Do not add an empty linkage
    if (0 == linkage.modifiers && 0 == linkage.extModifiers)
        return SUCCESS;

    LinkageUtil::SetWords (&linkage.linkHeader, (((pEnd - ((byte*)&linkage)) + 7) & ~7) / 2);
    return (SUCCESS == eeh.AppendElementLinkage (NULL, linkage.linkHeader, (byte*)&linkage + sizeof(linkage.linkHeader))) ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/02
+---------------+---------------+---------------+---------------+---------------+------*/
static void extractTextStyleLinkageData(LegacyTextStyleR textStyle, byte const & buffer)
    {
    memset (&textStyle, 0, sizeof (textStyle));
    
    byte const *    pBuffer         = &buffer;
    UInt32          modifiers       = 0;
    UInt32          extModifiers    = 0;

    ByteStreamHelper::ExtractUInt32 (modifiers, pBuffer);
    ByteStreamHelper::ExtractUInt32 (extModifiers, pBuffer);

    if (modifiers & (0x0001 << TEXTSTYLE_PROP_FontNo))              {   ByteStreamHelper::ExtractUInt32 (textStyle.fontNo, pBuffer);                    }
    if (modifiers & (0x0001 << TEXTSTYLE_PROP_ShxBigFont))          {   ByteStreamHelper::ExtractUInt32 (textStyle.shxBigFont, pBuffer);                }
    if (modifiers & (0x0001 << TEXTSTYLE_PROP_Width))               {   ByteStreamHelper::ExtractDouble (textStyle.width, pBuffer);                     }
    if (modifiers & (0x0001 << TEXTSTYLE_PROP_Height))              {   ByteStreamHelper::ExtractDouble (textStyle.height, pBuffer);                    }
    if (modifiers & (0x0001 << TEXTSTYLE_PROP_Slant))               {   ByteStreamHelper::ExtractDouble (textStyle.slant, pBuffer);                     }
    if (modifiers & (0x0001 << TEXTSTYLE_PROP_LineSpacing))         {   ByteStreamHelper::ExtractDouble (textStyle.lineSpacing, pBuffer);               }
    if (modifiers & (0x0001 << TEXTSTYLE_PROP_InterCharSpacing))    {   ByteStreamHelper::ExtractDouble (textStyle.interCharSpacing, pBuffer);          }
    if (modifiers & (0x0001 << TEXTSTYLE_PROP_UnderlineOffset))     {   ByteStreamHelper::ExtractDouble (textStyle.underlineOffset, pBuffer);           }
    if (modifiers & (0x0001 << TEXTSTYLE_PROP_OverlineOffset))      {   ByteStreamHelper::ExtractDouble (textStyle.overlineOffset, pBuffer);            }
    if (modifiers & (0x0001 << TEXTSTYLE_PROP_WidthFactor))         {   ByteStreamHelper::ExtractDouble (textStyle.widthFactor, pBuffer);               }
    if (modifiers & (0x0001 << TEXTSTYLE_PROP_LineOffset))          {   ByteStreamHelper::ExtractDouble (textStyle.lineOffset.x, pBuffer);
                                                                        ByteStreamHelper::ExtractDouble (textStyle.lineOffset.y, pBuffer);              }
    if (modifiers & (0x0001 << TEXTSTYLE_PROP_Just))                {   ByteStreamHelper::ExtractUInt16 (textStyle.just, pBuffer);                      }
    if (modifiers & (0x0001 << TEXTSTYLE_PROP_NodeJust))            {   ByteStreamHelper::ExtractUInt16 (textStyle.nodeJust, pBuffer);                  }
    if (modifiers & (0x0001 << TEXTSTYLE_PROP_LineLength))          {   ByteStreamHelper::ExtractUInt16 (textStyle.lineLength, pBuffer);                }
    if (modifiers & (0x0001 << TEXTSTYLE_PROP_TextDirection))       {   ByteStreamHelper::ExtractUInt16 (textStyle.textDirection, pBuffer);             }
    if (modifiers & (0x0001 << TEXTSTYLE_PROP_BackgroundStyle))     {   ByteStreamHelper::ExtractInt32 (textStyle.backgroundStyle.style, pBuffer);      }
    if (modifiers & (0x0001 << TEXTSTYLE_PROP_BackgroundWeight))    {   ByteStreamHelper::ExtractUInt32 (textStyle.backgroundStyle.weight, pBuffer);    }
    if (modifiers & (0x0001 << TEXTSTYLE_PROP_BackgroundColor))     {   ByteStreamHelper::ExtractUInt32 (textStyle.backgroundStyle.color, pBuffer);     }
    if (modifiers & (0x0001 << TEXTSTYLE_PROP_BackgroundFillColor)) {   ByteStreamHelper::ExtractUInt32 (textStyle.backgroundFillColor, pBuffer);       }
    if (modifiers & (0x0001 << TEXTSTYLE_PROP_BackgroundBorder))    {   ByteStreamHelper::ExtractDouble (textStyle.backgroundBorder.x, pBuffer);
                                                                        ByteStreamHelper::ExtractDouble (textStyle.backgroundBorder.y, pBuffer);        }
    if (modifiers & (0x0001 << TEXTSTYLE_PROP_UnderlineStyle))      {   ByteStreamHelper::ExtractInt32 (textStyle.underlineStyle.style, pBuffer);       }
    if (modifiers & (0x0001 << TEXTSTYLE_PROP_UnderlineWeight))     {   ByteStreamHelper::ExtractUInt32 (textStyle.underlineStyle.weight, pBuffer);     }
    if (modifiers & (0x0001 << TEXTSTYLE_PROP_UnderlineColor))      {   ByteStreamHelper::ExtractUInt32 (textStyle.underlineStyle.color, pBuffer);      }
    if (modifiers & (0x0001 << TEXTSTYLE_PROP_OverlineStyle))       {   ByteStreamHelper::ExtractInt32 (textStyle.overlineStyle.style, pBuffer);        }
    if (modifiers & (0x0001 << TEXTSTYLE_PROP_OverlineWeight))      {   ByteStreamHelper::ExtractUInt32 (textStyle.overlineStyle.weight, pBuffer);      }
    if (modifiers & (0x0001 << TEXTSTYLE_PROP_OverlineColor))       {   ByteStreamHelper::ExtractUInt32 (textStyle.overlineStyle.color, pBuffer);       }
    if (modifiers & (0x0001 << TEXTSTYLE_PROP_ParentId))            {   ByteStreamHelper::ExtractUInt32 (textStyle.parentId, pBuffer);                  }

    if (modifiers & (0x0001 << TEXTSTYLE_PROP_Flags))
        {
        UInt32 value;
        ByteStreamHelper::ExtractUInt32 (value, pBuffer);
        *(UInt32*)&textStyle.flags = value;
        }

    if (modifiers & (0x0001 << TEXTSTYLE_PROP_OverrideFlags))
        {
        UInt32* value = (UInt32*)&textStyle.overrideFlags;
        
        ByteStreamHelper::ExtractUInt32 (*(value + 0), pBuffer);
        ByteStreamHelper::ExtractUInt32 (*(value + 1), pBuffer);
        }

    if (modifiers & (0x0001 << TEXTSTYLE_PROP_Color))
        ByteStreamHelper::ExtractUInt32 (textStyle.color, pBuffer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/02
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ElementUtil::ExtractTextStyleFromLinkage (LegacyTextStyleR textStyle, ElementHandleCR eh, UInt32 linkageKey)
    {
    for (ConstElementLinkageIterator iter = eh.BeginElementLinkages(); iter.IsValid(); iter.ToNext())
        {
        if (LINKAGEID_TEXTSTYLE != iter.GetLinkage()->primaryID)
            continue;

        TextStyleLinkage*   pLinkage = (TextStyleLinkage *) iter.GetLinkage();

        if (linkageKey != pLinkage->linkageKey)
            continue;

        extractTextStyleLinkageData (textStyle, *(byte*)&pLinkage->modifiers);
        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/02
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ElementUtil::DeleteTextStyleLinkage (EditElementHandleR eeh, UInt32 linkageKey)
    {
    for (ElementLinkageIterator iter = eeh.BeginElementLinkages(); iter.IsValid(); iter.ToNext())
        {
        if (LINKAGEID_TEXTSTYLE != iter.GetLinkage()->primaryID)
            continue;

        TextStyleLinkage*   pLinkage = (TextStyleLinkage *) iter.GetLinkage();

        if (linkageKey != pLinkage->linkageKey)
            continue;

        eeh.RemoveElementLinkage (iter);
        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ElementUtil::ExtractTextShape (ElementHandleCR eh, DPoint3dP pts, DPoint3dR userOrigin)
    {
#if defined (NEEDS_WORK_DGNITEM)
    switch (eh.GetLegacyType())
        {
        case TEXT_ELM:
            {
            TextString  textString;

            if (SUCCESS != ((TextElemHandler&) eh.GetHandler ()).InitTextString (eh, textString))
                return ERROR;

            textString.GenerateBoundingShape (pts);
            textString.ComputeUserOrigin (userOrigin);

            return SUCCESS;
            }

        case TEXT_NODE_ELM:
            {
            RotMatrix   rMatrix;
            Transform   fwdTrans, invTrans;

            TextNodeHandler::GetUserOrigin (eh, userOrigin);
            TextNodeHandler::GetOrientation (eh, rMatrix);

            fwdTrans.initFrom (&rMatrix, &userOrigin);
            invTrans.inverseOf (&fwdTrans);

            DRange3d    range;

            range.init ();

            for (ChildElemIter textEh (eh, ExposeChildrenReason::Count); textEh.IsValid (); textEh = textEh.ToNext ())
                {
                TextString  textString;

                if (SUCCESS != ((TextElemHandler&) textEh.GetHandler ()).InitTextString (textEh, textString))
                    return ERROR;

                textString.GenerateBoundingShape (pts);
                invTrans.multiply (pts, pts, 5);

                range.extend (pts, 5);
                }

            pts[0] = range.low;
            pts[2] = range.high;

            pts[1].x = pts[2].x;
            pts[1].y = pts[0].y;
            pts[1].z = pts[0].z;
            pts[3].x = pts[0].x;
            pts[3].y = pts[2].y;
            pts[3].z = pts[2].z;
            pts[4]   = pts[0];

            fwdTrans.multiply (pts, pts, 5);

            return SUCCESS;
            }

        default:
            return ERROR;
        }
#endif
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/09
+---------------+---------------+---------------+---------------+---------------+------*/
typedef struct thicknessLinkage
    {
    LinkageHeader   hdr;
    UInt32          cap:1;
    UInt32          alwaysUseDirection:1;
    UInt32          reserved:30;
    double          thickness[3];   // thickness value or scaled direction bvector
    } ThicknessLinkage;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/09
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     getDefaultDirectionForElement (DVec3dR direction, ElementHandleCR eh)
    {
    DisplayHandlerP dHandler = eh.GetDisplayHandler();

    if (!dHandler)
        return false;

    return dHandler->IsPlanar (eh, &direction, NULL, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void     applyTransformToThicknessLinkage (EditElementHandleR eeh, TransformInfoCR trans)
    {
    DgnElementCP elmCP = eeh.GetElementCP ();

    if (elmCP->GetSizeWords() == elmCP->GetAttributeOffset() || !elmCP->Is3d())
        return;

    UShort*             linkP = NULL;
    ThicknessLinkage    thicknessLinkage;

    memset (&thicknessLinkage, 0, sizeof (ThicknessLinkage));

    if (NULL == (linkP = (UShort*) elemUtil_extractLinkage (&thicknessLinkage, NULL, elmCP, LINKAGEID_Thickness)))
        return;

    int         linkBytes = ((sizeof (LinkageHeader) + sizeof (double) + 1) + 7) & ~7;
    DVec3d      direction;
    RotMatrix   rMatrix;

    rMatrix.InitFrom(*( trans.GetTransform ()));

    if (LinkageUtil::GetWords (&thicknessLinkage.hdr) > linkBytes/2)
        {
        memcpy (&direction, &thicknessLinkage.thickness[0], sizeof (direction));

        rMatrix.Multiply(direction);

        memcpy (linkP+sizeof (LinkageHeader), &direction, sizeof (direction));
        }
    else
        {
        double      thickness;

        memcpy (&thickness, &thicknessLinkage.thickness[0], sizeof (thickness));

        if (getDefaultDirectionForElement (direction, eeh))
            {
            bool        isNegative = (thickness < 0.0);

            direction.Scale (direction,  thickness);
            rMatrix.Multiply(direction);

            thickness = direction.Magnitude () * (isNegative ? -1.0 : 1.0);
            }

        memcpy (linkP+sizeof (LinkageHeader), &thickness, sizeof (thickness));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ElementUtil::AddThickness (EditElementHandleR eeh, double thickness, DVec3dCP direction, bool isCapped, bool alwaysUseDirection)
    {
    elemUtil_deleteLinkage (eeh.GetElementP (), LINKAGEID_Thickness); // Remove existing linkage...

    if (0.0 == thickness)
        return SUCCESS;

    int                 linkBytes = 0;
    ThicknessLinkage    thicknessLinkage;

    memset (&thicknessLinkage, 0, sizeof (ThicknessLinkage));

    thicknessLinkage.hdr.user       = 1;
    thicknessLinkage.hdr.primaryID  = LINKAGEID_Thickness;

    thicknessLinkage.cap                = isCapped;
    thicknessLinkage.alwaysUseDirection = alwaysUseDirection;

    DgnV8ElementBlank   tmpElm;

    eeh.GetElementCP ()->CopyTo (tmpElm);

    if (direction)
        {
        DVec3d    tmpDir;

        linkBytes = ((sizeof (LinkageHeader) + sizeof (DPoint3d) + 1) + 7) & ~7;
        LinkageUtil::SetWords (&thicknessLinkage.hdr, linkBytes/sizeof (short));

        tmpDir.Scale (*( (DVec3d *) direction),  thickness);
        memcpy (&thicknessLinkage.thickness[0], &tmpDir, sizeof (DPoint3d));
        }
    else
        {
        linkBytes = ((sizeof (LinkageHeader) + sizeof (double) + 1) + 7) & ~7;
        LinkageUtil::SetWords (&thicknessLinkage.hdr, linkBytes/sizeof (short));

        thicknessLinkage.thickness[0] = thickness;
        }

    if (SUCCESS != elemUtil_appendLinkage (&tmpElm, &thicknessLinkage.hdr))
        return ERROR;

    eeh.ReplaceElement (&tmpElm);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ElementUtil::ExtractThickness (double& thickness, DVec3dR direction, bool& isCapped, bool& alwaysUseDirection, ElementHandleCR eh)
    {
    DgnElementCP elmCP = eh.GetElementCP ();

    if (elmCP->GetSizeWords() == elmCP->GetAttributeOffset() || !elmCP->Is3d())
        return ERROR;

    if (!mdlElement_attributePresent (elmCP, LINKAGEID_Thickness, NULL))
        return ERROR;

    ThicknessLinkage     thicknessLinkage;

    memset (&thicknessLinkage, 0, sizeof (ThicknessLinkage));

    if (NULL == elemUtil_extractLinkage (&thicknessLinkage, NULL, elmCP, LINKAGEID_Thickness))
        return ERROR;

    int         linkBytes = ((sizeof (LinkageHeader) + sizeof (double) + 1) + 7) & ~7;

    isCapped = thicknessLinkage.cap;

    alwaysUseDirection = thicknessLinkage.alwaysUseDirection;

    if (LinkageUtil::GetWords (&thicknessLinkage.hdr) > linkBytes/2)
        {
        memcpy (&direction, &thicknessLinkage.thickness[0], 3*sizeof (double));
        thickness = direction.normalize ();
        }
    else
        {
        direction.init (0.0, 0.0, 1.0);
        thickness = thicknessLinkage.thickness[0];
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static void     transformClippingDepths (EditElementHandleR eeh, TransformCR transform)
    {
    // Look for saved view clip depth linkage...only for planar elements!
    for (ElementLinkageIterator li = eeh.BeginElementLinkages(); li != eeh.EndElementLinkages(); ++li)
        {
        UInt16      linkageKey;
        UInt32      numEntries;
        double*     doubleData = (double*) ElementLinkageUtil::GetDoubleArrayDataCP (li, linkageKey, numEntries);

        if (NULL == doubleData || DOUBLEARRAY_LINKAGE_KEY_ClippingDepth != linkageKey)
            continue;

        if (3 != numEntries)
            break;

        double* zFront = doubleData;
        double* zBack = (doubleData+1);

#if defined (NEEDS_WORK)
        // *** This is incomplete (only planar elements) and ambiguous (we must assume origin has been transformed)
        // *** Ideally, the handler would create a box, transform it, and then measure the results.
        DVec3d      normal, defaultNormal; defaultNormal.init (0,0,1.0);
        DPoint3d    origin;

        if (eh.GetDisplayHandler()->IsPlanar (eh, &normal, &origin, &defaultNormal))
            {
            DPoint3d fbpoints[2];
            bsiDPoint3d_addScaledDVec3d (&fbpoints[0], &origin, &normal, f);
            bsiDPoint3d_addScaledDVec3d (&fbpoints[1], &origin, &normal, b);
            //bsiTransform_multiplyDPoint3dArrayInPlace (&transform, &origin, 1);   At this point, origin should already have been transformed
            bsiTransform_multiplyDPoint3dArrayInPlace (&transform, fbpoints, 2);
            f = bsiDPoint3d_distance (&origin, &fbpoints[0]);
            b = bsiDPoint3d_distance (&origin, &fbpoints[1]);
            }
        else
#endif      
            {
            double  scale = 1.0;

            if (transform.isUniformScaleAndRotateAroundLine (NULL, NULL, NULL, &scale))
                {
                *zFront *= scale;
                *zBack  *= scale;
                }
            }

        break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementUtil::ApplyTransformToLinkages (EditElementHandleR eeh, TransformInfoCR trans)
    {
    // Transform known graphics-specific linkages
    DgnElementCP    el = eeh.GetElementP ();
    DgnModelP       model = eeh.GetDgnModelP ();

#ifdef DGNV10FORMAT_CHANGES_WIP
    BeAssert (el->hdr.IsGraphic());
#endif

    if (el->GetSizeWords() > el->GetAttributeOffset())
        {
#if defined (NEEDS_WORK_DGNITEM)
        PatternLinkageUtil::OnElementTransform (eeh, *trans.GetTransform(), true);
#endif
        applyTransformToThicknessLinkage (eeh, trans);
        transformClippingDepths (eeh, *trans.GetTransform());
        }

    if (LineStyleUtil::ElementHasLineStyle (eeh.GetElementP (), model))
        LineStyleUtil::TransformParams (eeh, trans.GetTransform(), model, model, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/02
+---------------+---------------+---------------+---------------+---------------+------*/
static bool hasXData (ElementHandleCR eh)
    {
    DgnElementCP el = eh.GetElementCP ();

    if (el->GetSizeWords() <= el->GetAttributeOffset())
        return false;

    if (!mdlElement_attributePresent (el, LINKAGEID_XData, NULL))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/02
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt getDwgTransform (TransformR dwgToDgnTransform, TransformR dgnToDwgTransform, DgnModelR modelRef)
    {
#ifdef DGNV10FORMAT_CHANGES_WIP
    DgnElement   linkageHolder;

    if (SUCCESS != dgnModel_getLinkageHolderElement (modelRef.GetDgnModelP (), &linkageHolder))
        return ERROR;

    ElementHandle   eh (&linkageHolder, &modelRef);

    for (ConstElementLinkageIterator li = eh.BeginElementLinkages(); li != eh.EndElementLinkages(); ++li)
        {
        UInt16          linkageKey;
        UInt32          numEntries;
        double const*   doubleData = ElementLinkageUtil::GetDoubleArrayDataCP (li, linkageKey, numEntries);

        if (NULL == doubleData || DOUBLEARRAY_LINKAGE_KEY_DwgTransform != linkageKey)
            continue;

        if ((sizeof (Transform) / sizeof (double)) != numEntries)
            return ERROR;

        memcpy (&dgnToDwgTransform, doubleData, sizeof (dgnToDwgTransform));
        dwgToDgnTransform.InverseOf (dgnToDwgTransform);

        return SUCCESS;
        }
#endif

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementUtil::ApplyTransformToXDataLinkage (EditElementHandleR eeh, TransformInfoCR trans)
    {
    Transform   dwgToDgn, dgnToDwg;

    if (!hasXData (eeh) || SUCCESS != getDwgTransform (dwgToDgn, dgnToDwg, *eeh.GetDgnModelP ()))
        return;

    Transform   composite;

    composite.InitProduct (*trans.GetTransform (), dwgToDgn);
    composite.InitProduct (dgnToDwg, composite);

    for (UInt8 const* xDataHdr : XDataHelper::Collection (eeh))
        {
        switch (XDataHelper::Iterator::GetGroupCode (xDataHdr))
            {
            case XDataHelper::DWGXDATA_Space_Point:
                {
                composite.Multiply(*((DPoint3dP) XDataHelper::Iterator::GetDataCP (xDataHdr)));
                break;
                }

            case XDataHelper::DWGXDATA_Disp_Point:
                {
                composite.MultiplyMatrixOnly (*((DVec3dP) XDataHelper::Iterator::GetDataCP (xDataHdr)));
                break;
                }

            case XDataHelper::DWGXDATA_Dir_Point:
                {
                composite.MultiplyMatrixOnly (*((DVec3dP) XDataHelper::Iterator::GetDataCP (xDataHdr)));
                ((DVec3dP) XDataHelper::Iterator::GetDataCP (xDataHdr))->Normalize ();
                break;
                }

            case XDataHelper::DWGXDATA_Dist:
            case XDataHelper::DWGXDATA_Scale:
                {
                DPoint3d    scale;

                LegacyMath::TMatrix::FromNormalizedRowsOfTMatrix (NULL, &scale, &composite);
                *((double *) XDataHelper::Iterator::GetDataCP (xDataHdr)) *= scale.x;
                break;
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementUtil::FenceStretchXDataLinkage (EditElementHandleR eeh, TransformInfoCR trans, FenceParamsP fp)
    {
    Transform   dwgToDgn, dgnToDwg;

    if (!hasXData (eeh) || SUCCESS != getDwgTransform (dwgToDgn, dgnToDwg, *eeh.GetDgnModelP ()))
        return;

    for (UInt8 const* xDataHdr : XDataHelper::Collection (eeh))
        {
        switch (XDataHelper::Iterator::GetGroupCode (xDataHdr))
            {
            case XDataHelper::DWGXDATA_Space_Point:
                {
                void*       entryData = (void*) XDataHelper::Iterator::GetDataCP (xDataHdr);
                DPoint3d    dgnPoint;

                memcpy (&dgnPoint, entryData, sizeof (dgnPoint));
                dwgToDgn.Multiply (dgnPoint);

                if (!fp->PointInside (dgnPoint))
                    break;

                trans.GetTransform ()->Multiply (dgnPoint);
                dgnToDwg.Multiply (dgnPoint);

                memcpy (entryData, &dgnPoint, sizeof (dgnPoint));
                break;
                }
            }
        }
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void updatePatternFromTemplate (EditElementHandleR eeh, ElementHandleCR templateEh, IAreaFillPropertiesEdit* areaObj, IAreaFillPropertiesQuery* areaTemplateObj)
    {
    DPoint3d                 origin;
    PatternParamsPtr         params;
    bvector<DwgHatchDefLine> hatchLines;

    if (!areaTemplateObj->GetPattern (templateEh, params, &hatchLines, &origin, 0))
        return;

    // NOTE: Need to compute pattern offset for new boundary... 
    DVec3d      patNormal, elmNormal;
    DPoint3d    patOrigin, elmOrigin;

    PatternLinkageUtil::GetHatchOrigin (elmOrigin, *eeh.GetElementCP ());
    params->rMatrix.GetRow (patNormal, 2);

    if (!eeh.GetDisplayHandler ()->IsPlanar (eeh, &elmNormal, NULL, NULL))
        elmNormal = patNormal;

    LegacyMath::Vec::LinePlaneIntersect (&patOrigin, &origin, &patNormal, &elmOrigin, &elmNormal, false);

    if (!elmOrigin.IsEqual (patOrigin, 1.0e-8))
        {
        params->offset.DifferenceOf (patOrigin, elmOrigin);
        params->modifiers = params->modifiers | PatternParamsModifierFlags::Offset;
        }

    areaObj->AddPattern (eeh, *params, &hatchLines.front ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void applyPatternFromTemplate (EditElementHandleR eeh, ElementHandleCR templateEh)
    {
    IAreaFillPropertiesEdit* areaObj = dynamic_cast <IAreaFillPropertiesEdit*> (&eeh.GetHandler ());

    if (areaObj)
        {
        areaObj->RemovePattern (eeh, -1);

        IAreaFillPropertiesQuery* areaTemplateObj = dynamic_cast <IAreaFillPropertiesQuery*> (&templateEh.GetHandler ());

        if (areaTemplateObj)
            updatePatternFromTemplate (eeh, templateEh, areaObj, areaTemplateObj);

        return;
        }

    // Apply template to public children...
    ChildEditElemIter childElm (eeh, ExposeChildrenReason::Edit);

    if (!childElm.IsValid ())
        return;

    for (; childElm.IsValid (); childElm = childElm.ToNext ())
        applyPatternFromTemplate (childElm, templateEh);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void setAreaFillFromDisplayParams (EditElementHandleR eeh, ElemDisplayParamsCR params)
    {
    IAreaFillPropertiesEdit* areaObj = dynamic_cast <IAreaFillPropertiesEdit*> (&eeh.GetHandler ());

    if (areaObj)
        {
        areaObj->RemoveAreaFill (eeh);

        if (NULL != params.GetGradient ())
            {
            areaObj->AddGradientFill (eeh, *params.GetGradient ());
            }
        else if (FillDisplay::Never != params.GetFillDisplay ())
            {
            bool    alwaysFill = (FillDisplay::Always == params.GetFillDisplay ());
            UInt32  fillColor = params.GetFillColor ();

            // Try to find an existing extended color id for TBGR color...
            if (INVALID_COLOR == fillColor)
                {
                DgnProjectP  dgnFile = eeh.GetDgnProject ();

                if (dgnFile)
                    fillColor = dgnFile->Colors().FindElementColor (IntColorDef (params.GetFillColorTBGR ()));
                }

            areaObj->AddSolidFill (eeh, INVALID_COLOR != fillColor ? &fillColor : NULL, &alwaysFill);
            }

        return;
        }

#if defined (NEEDS_WORK_DGNITEM)
    // Apply to public children...
    ChildEditElemIter childElm (eeh, ExposeChildrenReason::Edit);

    if (!childElm.IsValid ())
        return;

    for (; childElm.IsValid (); childElm = childElm.ToNext ())
        setAreaFillFromDisplayParams (childElm, params);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void setMaterialFromDisplayParams (EditElementHandleR eeh, ElemDisplayParamsCR params)
    {
#ifdef WIP_VANCOUVER_MERGE // material
    IMaterialPropertiesExtension* materialExt = IMaterialPropertiesExtension::Cast (eeh.GetHandler ());

    if (materialExt)
        {
        materialExt->DeleteMaterialAttachment (eeh);

        if (params.IsAttachedMaterial () && params.GetMaterial ())
            {
            MaterialCP  material = params.GetMaterial ();
            MaterialId  materialId (material->GetElementId (), material->GetName ().c_str ());

            materialExt->AddMaterialAttachment (eeh, materialId);
            }

        return;
        }

    // Apply to public children...
    ChildEditElemIter childElm (eeh, ExposeChildrenReason::Edit);

    if (!childElm.IsValid ())
        return;

    for (; childElm.IsValid (); childElm = childElm.ToNext ())
        setMaterialFromDisplayParams (childElm, params);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementPropertiesSetter::ApplyElemDisplayParamsRestricted (EditElementHandleR eeh, ElemDisplayParamsCR params, TemplateIgnores options)
    {
    ElementPropertiesSetter remapper;

    if (0 == (options & TEMPLATE_IGNORE_Level))
        remapper.SetLevel (params.GetSubLevelId().GetLevel());

    if (0 == (options & TEMPLATE_IGNORE_Color))
        {
        UInt32  color = params.GetLineColor ();

        // Try to find an existing extended color id for TBGR color...
        if (INVALID_COLOR == color)
            {
            DgnProjectP  dgnFile = eeh.GetDgnProject ();

            if (dgnFile)
                color = dgnFile->Colors().FindElementColor (IntColorDef (params.GetLineColorTBGR ()));
            }
        
        if (INVALID_COLOR != color)
            remapper.SetColor (color);
        }

    if (0 == (options & TEMPLATE_IGNORE_Style))
        {
        Int32               style = params.GetLineStyle ();
        LineStyleParamsCP   styleParams = params.GetLineStyleParams ();

        remapper.SetLinestyle (style, IS_LINECODE (style) || 0 != (options & TEMPLATE_IGNORE_StyleModifiers) ? NULL : styleParams);
        }

    if (0 == (options & TEMPLATE_IGNORE_Weight))
        remapper.SetWeight (params.GetWeight ());

    if (0 == (options & TEMPLATE_IGNORE_ElemClass))
        remapper.SetElementClass (params.GetElementClass ());

    if (0 == (options & TEMPLATE_IGNORE_Transparency))
        remapper.SetTransparency (params.GetTransparency ());

    if (0 == (options & TEMPLATE_IGNORE_Priority))
        remapper.SetDisplayPriority (params.GetElementDisplayPriority ());

    remapper.Apply (eeh);

    if (0 == (options & TEMPLATE_IGNORE_Fill))
        setAreaFillFromDisplayParams (eeh, params);

    if (0 == (options & TEMPLATE_IGNORE_Material))
        setMaterialFromDisplayParams (eeh, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementPropertiesSetter::ApplyElemDisplayParams (EditElementHandleR eeh, ElemDisplayParamsCR params)
    {
    ApplyElemDisplayParamsRestricted (eeh, params, TEMPLATE_IGNORE_None);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/07
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementPropertiesSetter::ApplyTemplateRestricted (EditElementHandleR eeh, ElementHandleCR templateEh, TemplateIgnores options)
    {
    DisplayHandlerP dHandler = templateEh.GetDisplayHandler ();

    if (!dHandler)
        return;

    ElemDisplayParams params;

    dHandler->GetElemDisplayParams (templateEh, params, true); // Get material for apply...

    ApplyElemDisplayParamsRestricted (eeh, params, options);

#if defined (NEEDS_WORK_DGNITEM)
    if (0 == (options & TEMPLATE_IGNORE_Pattern))
        applyPatternFromTemplate (eeh, templateEh); // Pattern isn't part of ElemDisplayParams...
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/07
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementPropertiesSetter::ApplyTemplate (EditElementHandleR eeh, ElementHandleCR templateEh)
    {
    ApplyTemplateRestricted (eeh, templateEh, TEMPLATE_IGNORE_None);
    }
