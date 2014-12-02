/*----------------------------------------------------------------------+
|
|   $Source: DgnHandlers/Dimension/DimRadial.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/86
+---------------+---------------+---------------+---------------+---------------+------*/
static void getArcDimPoint
(
DPoint3d   *arcPoint,       /* <= Starting point for radius dimension  */
DPoint3d   *dPoint,         /* => current cursor location              */
DimArcInfo *dimArc,
RotMatrix  *rMatrix,
int        *crossState,
int        useCrossState
)
    {
    DVec3d    rvec, radDir;
    double    angle, curDist;

    bsiDVec3d_subtractDPoint3dDPoint3d (&rvec, dPoint, &dimArc->center);
    rMatrix->MultiplyTranspose(rvec);

    angle = Angle::Atan2 (rvec.y, rvec.x);

    if (!useCrossState)
        *crossState = false;

    if ((useCrossState && *crossState) || !in_span (angle, dimArc->startAng, dimArc->sweepAng))
        {
        angle += msGeomConst_pi;
        if (!in_span (angle, dimArc->startAng, dimArc->sweepAng))
            {
            angle = (bsiDPoint3d_distance (&dimArc->start, dPoint) <
                     bsiDPoint3d_distance (&dimArc->end, dPoint)) ?
                     dimArc->startAng : dimArc->sweepAng;
            }
        else
            {
            if (!useCrossState)
                *crossState = true;
            }
        }

    arcPoint->x = dimArc->radius * cos(angle);
    arcPoint->y = dimArc->radius * sin(angle);
    arcPoint->z = 0.0;

    rMatrix->Multiply(*arcPoint);

    if (!*crossState)
        {
        radDir = *((DVec3d *) arcPoint);
        radDir.Normalize ();
        curDist = bsiDPoint3d_distance (&dimArc->center, dPoint);
        bsiDPoint3d_addScaledDVec3d (dPoint, &dimArc->center, &radDir, curDist);
        }

    bsiDPoint3d_addDPoint3dInPlace (arcPoint, &dimArc->center);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     12/90
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   generatePointerDimensionText
(
AdimProcess     *ep,         /* => Function used to process elements   */
DPoint3d        *extPoint,   /* => Leader origin extension point       */
DPoint3d        *dPoints,    /* => Array of leader line vertices       */
int             nPoints,     /* => Number of vertices in leader line   */
double          dimLength,   /* => Dimension length value              */
int             noValue,     /* => If set, ignore length value         */
int             termOpt      /* => Line terminator options             */
)
    {
    int          status;
    double       charwdth, line_len;
    DPoint2d     textSize;
    DimText      dimText;
    DPoint3d     rtxtorg, leader_end;
    DVec3d       dir, check;
    DPoint3dP    endPoint;

    if (noValue && ep->strDat.m_strings.GetPrimaryStrings()->empty() && !dimTextBlock_getRequiresTextBlock (ep, NULL))
        return (SUCCESS);

    DimensionElm const* dim = ep->GetDimElementCP();
    adim_getLengthStrings (ep, dimLength, dim->frmt.primaryAccuracy);
    adim_getTextSize (&textSize, ep, &ep->strDat, ADIM_TEXTSIZE_Exact);

    charwdth = ep->strDat.charWidth;

    endPoint = dPoints + (nPoints-1);
    
    if (dim->flag.horizontal)
        {
        DimStyleProp_Text_Location  textLocation = DIMSTYLE_VALUE_Text_Location_Inline;
        double                      dOffsetY = 0.0;

        bsiDVec3d_subtractDPoint3dDPoint3d (&check, endPoint, endPoint-1);

        ep->vuMatrix.Multiply(check);
        dir.y = dir.z = 0.0;
        dir.x = check.x > 0.0 ? 1.0 : -1.0;
        ep->vuMatrix.MultiplyTranspose(dir);

        bsiDPoint3d_addScaledDVec3d (&leader_end, endPoint, &dir, 2.0 * charwdth);
        bsiDPoint3d_addScaledDVec3d (&rtxtorg,    endPoint, &dir,
            2.0 * charwdth + dim->geom.textMargin);

        ADIM_SETNAME (ep->partName, ADTYPE_DIMLINE, ADSUB_LEADER);
        if (status = adim_generateLine (ep, endPoint, &leader_end, DIM_MATERIAL_DimLine))
            return (status);

        /* check horizontal inline text vertical position */
        mdlDim_overridesGetTextLocation (&textLocation, ep->pOverrides, ep->flags.embed);

        if (DIMSTYLE_VALUE_Text_Location_Inline == textLocation &&
            mdlDim_extensionsGetInlineTextLift(&dOffsetY, ep->pOverrides, 0.0))
            {
            DVec3d    xAxis;
            ep->vuMatrix.GetRow(xAxis,  0);

            dOffsetY *= textSize.y;

            adim_offsetText (&rtxtorg, &rtxtorg, &xAxis, dOffsetY, ep);
            }
        }
    else
        {
        line_len = bsiDPoint3d_distance (endPoint-1, endPoint);
        dir.NormalizedDifference (*endPoint, *( endPoint-1));

        if (dim->flag.embed || dim->flag.dual)
            bsiDPoint3d_addScaledDVec3d (&rtxtorg, endPoint-1, &dir,
                line_len + (double)dim->geom.textMargin);
        else
            bsiDPoint3d_addScaledDVec3d (&rtxtorg, endPoint-1, &dir, line_len - textSize.x);
        }

    check = dir;
    ep->vuMatrix.Multiply(check);
    if (check.x < 0)
        {
        bsiDPoint3d_addScaledDVec3d (&rtxtorg, &rtxtorg, &dir, textSize.x);
        bsiDVec3d_scale (&dir, &dir, -1.0);
        }

    dimText = *dim->GetDimTextCP(0);

    return (adim_generateLinearDimension (ep, &rtxtorg, &dir, dimText.offsetY));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt    adim_generateNoteDimension
(
AdimProcess     *ep,         /* => Function used to process elements   */
DPoint3d        *extPoint,   /* => Leader origin extension point       */
DPoint3d        *dPoints,    /* => Array of leader line vertices       */
int             nPoints,     /* => Number of vertices in leader line   */
double          dimLength,   /* => Dimension length value              */
int             noValue,     /* => If set, ignore length value         */
int             termOpt      /* => Line terminator options             */
)
    {
    DVec3d       termDir;
    int          status;
    int          leaderType, noteTerminator;

    mdlDim_extensionsGetNoteLeaderType (&leaderType, ep->pOverrides, 0);

    // Two-point bspline with inline rotation produces an undesirable display
    // because of a sharp bend where it meets the text. Make it a simple line.
    if (2 == nPoints && 1 == leaderType)
        {
        DimStyleProp_MLNote_TextRotation    rotation = DIMSTYLE_VALUE_MLNote_TextRotation_Horizontal;

        mdlDim_extensionsGetNoteTextRotation ((int*) &rotation, ep->pOverrides, 0);
        if (DIMSTYLE_VALUE_MLNote_TextRotation_Inline == rotation)
            {
            ElementHandle dimensionElement = ep->GetElemHandleCR();
            ElementRefP cellElmRef = mdlNote_getRootNoteCellElmRef (dimensionElement);
            if (cellElmRef)
                {
                size_t          elemSize   = cellElmRef->GetMemorySize();
                DgnElement *     cellElm    = (DgnElement *) _alloca (elemSize);
                ElementId       iRootDimID;

                if (cellElmRef->GetElement (cellElm, elemSize) &&
                    SUCCESS == mdlNote_getRootDimension (&iRootDimID, *cellElm) &&
                    ep->GetDimElementCP()->GetElementId() == iRootDimID)
                    leaderType = 0;
                }
            }
        }

    /*------------------------------------------------------------------
      Leader
    ------------------------------------------------------------------*/
    ADIM_SETNAME (ep->partName, ADTYPE_DIMLINE, ADSUB_NONE);
    switch (leaderType)
        {
        case 0:
            {
            if (status = adim_generateLineString (ep, dPoints, nPoints, 0, DIM_MATERIAL_DimLine))
                return (status);

            termDir.NormalizedDifference (*dPoints, dPoints[1]);
            break;
            }

        case 1:
            {
            DPoint3d    defTangent = {0.0, 0.0, 0.0};
            DPoint3d    tangents[2];
            mdlDim_overridesGetSegmentCurveStartTangent (&tangents[0], ep->pOverrides, 0, &defTangent);
            mdlDim_overridesGetSegmentCurveEndTangent (&tangents[1], ep->pOverrides, 0, &defTangent);
            if (status = adim_generateBSpline (&termDir, ep, nPoints, dPoints, &tangents[0], &tangents[1]))
                return (status);
            break;
            }
        }

    /*------------------------------------------------------------------
      Terminator
    ------------------------------------------------------------------*/
    mdlDim_extensionsGetNoteTerminator (&noteTerminator, ep->pOverrides, 1);

    ADIM_SETNAME (ep->partName, ADTYPE_TERM_LEFT, ADSUB_NONE);
    adim_generateTerminator (ep, dPoints, &termDir, noteTerminator);

    /*------------------------------------------------------------------
      Draw dimension text
      If noValue is true and there is no text in ep->strDat, return,
      there is no text to draw - its just a leader line.
    ------------------------------------------------------------------*/
    return generatePointerDimensionText (ep, extPoint, dPoints, nPoints, dimLength, noValue, termOpt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     06/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int      adim_generatePointerDimension
(
AdimProcess     *ep,         /* => Function used to process elements   */
DPoint3d        *extPoint,   /* => Leader origin extension point       */
DPoint3d        *dPoints,    /* => Array of leader line vertices       */
int             nPoints,     /* => Number of vertices in leader line   */
double          dimLength,   /* => Dimension length value              */
int             noValue,     /* => If set, ignore length value         */
int             termOpt      /* => Line terminator options             */
)
    {
    int          status;

    /*------------------------------------------------------------------
      Draw dimension line and terminator
    ------------------------------------------------------------------*/
    if (extPoint)
        {
        ADIM_SETNAME (ep->partName, ADTYPE_DIMLINE, ADSUB_NONE);
        if (status = adim_generateLine (ep, extPoint, dPoints, DIM_MATERIAL_DimLine))
            return (status);
        }
    else
        termOpt = 0;

    switch (termOpt)
        {
        case 0:                        /* first point towards center        */
            ADIM_SETNAME (ep->partName, ADTYPE_TERM_LEFT, ADSUB_NONE);
            if (status = adim_generateLineTerminator (ep, dPoints+1, dPoints,
                ep->GetDimElementCP()->tmpl.right_term, false))
                return (status);
            break;

        case 2:                        /* two terminators away from center  */
            ADIM_SETNAME (ep->partName, ADTYPE_TERM_LEFT, ADSUB_NONE);
            if (status = adim_generateLineTerminator (ep, dPoints, extPoint,
                ep->GetDimElementCP()->tmpl.right_term, false))
                return (status);

        case 1:                        /* first point away from center      */
            ADIM_SETNAME (ep->partName, ADTYPE_TERM_RIGHT, ADSUB_NONE);
            if (status = adim_generateLineTerminator (ep, extPoint, dPoints,
                ep->GetDimElementCP()->tmpl.right_term, false))
                return (status);
        }

    ADIM_SETNAME (ep->partName, ADTYPE_DIMLINE, ADSUB_NONE);
    if (status = adim_generateLineString (ep, dPoints, nPoints, 0, DIM_MATERIAL_DimLine))
        return (status);

    return generatePointerDimensionText (ep, extPoint, dPoints, nPoints, dimLength, noValue, termOpt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     12/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int generateCenterWitness
(
AdimProcess *ap,
DVec3d      *dir,
double      orgDist,
double      endDist
)
    {
    DSegment3d line;

    bsiDPoint3d_addScaledDVec3d (&line.point[0], &ap->dimArc.center, dir, orgDist);
    bsiDPoint3d_addScaledDVec3d (&line.point[1], &ap->dimArc.center, dir, endDist);

    return (adim_generateLine (ap, &line.point[0], &line.point[1], DIM_MATERIAL_DimLine));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/89
+---------------+---------------+---------------+---------------+---------------+------*/
static int      adim_generateDimCenter
(
AdimProcess     *ep
)
    {
    DVec3d      xvec, yvec, zvec, view_y;
    double      rtmp, orgDist, endDist;
    int         status, allWit;

    ep->rMatrix.GetColumn(zvec,  2);
    ep->vuMatrix.GetRow(view_y,  1);

    bsiDVec3d_crossProduct (&xvec, &view_y, &zvec);
    bsiDVec3d_crossProduct (&yvec, &zvec, &xvec);

    xvec.Normalize ();
    yvec.Normalize ();

    rtmp = fabs((double) ep->GetDimElementCP()->geom.centerSize);

    ADIM_SETNAME (ep->partName, ADTYPE_CENTER, ADSUB_NONE);

    if (ep->GetDimElementCP()->geom.centerSize != 0)
        {
        if (status = generateCenterWitness (ep, &xvec, -rtmp, rtmp))
            return (status);

        if (status = generateCenterWitness (ep, &yvec, -rtmp, rtmp))
            return (status);
        }

    orgDist = rtmp + (double)ep->GetDimElementCP()->geom.witOffset;
    endDist = ep->dimArc.radius + (double)ep->GetDimElementCP()->geom.witExtend;

    if (endDist > orgDist)
        {
        allWit  = ep->GetDimElementCP()->geom.centerSize < 0L ? true : false;

        if (allWit || ep->GetDimElementCP()->tmpl.centerRight)
            if (status = generateCenterWitness (ep, &xvec, orgDist, endDist))
                return (status);

        if (allWit || ep->GetDimElementCP()->tmpl.centerLeft)
            if (status = generateCenterWitness (ep, &xvec, -orgDist, -endDist))
                return (status);

        if (allWit || ep->GetDimElementCP()->tmpl.centerTop)
            if (status = generateCenterWitness (ep, &yvec, orgDist, endDist))
                return (status);

        if (allWit || ep->GetDimElementCP()->tmpl.centerBottom)
            if (status = generateCenterWitness (ep, &yvec, -orgDist, -endDist))
                return (status);
        }

    return (SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     12/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int generateRadialPointer
(
AdimProcess *ep,
DPoint3d    *dPoints,
int         nPoints,
double      dimLength
)
    {
    DVec3d  extDir;
    int       status, termOpt;

    switch (ep->GetDimElementCP()->tmpl.first_term)
        {
        case 0:          /* standard arrow in */
            if (status = adim_generatePointerDimension (ep, NULL, dPoints+1,
                nPoints-1, dimLength, 0, 0))
                return (status);
            break;

        case 1:          /* arrow in, extend to center  */
        case 3:          /* arrow out, extend to center */
            termOpt = ep->GetDimElementCP()->tmpl.first_term == 1     ? 0 : 1;
            *dPoints = ep->dimArc.center;
            if (status = adim_generatePointerDimension (ep, dPoints, dPoints+1,
                nPoints-1, dimLength, 0, termOpt))
                return (status);
            break;

        case 2:          /* extend across, two arrows out */
            extDir.NormalizedDifference (*( &ep->dimArc.center), dPoints[1]);
            bsiDPoint3d_addScaledDVec3d (dPoints, &ep->dimArc.center, &extDir, ep->dimArc.radius);
            if (status = adim_generatePointerDimension (ep, dPoints, dPoints+1,
                nPoints-1, dimLength, 0, 2))
                return (status);
            break;
        }

    return (SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     12/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int      adim_generateRadiusDimension
(
AdimProcess     *ep,
DPoint3d        *dPoints,
int             nPoints
)
    {
    double      dimLength;
    int         status, crossState;

    dimLength = ep->GetDimElementCP()->GetScale() * ep->dimArc.radius;
    if (ep->pdDimLengths != NULL)
        ep->pdDimLengths[ADIM_GETSEG (ep->partName)] = dimLength;

    crossState = ep->GetDimElementCP()->flag.crossCenter;
    getArcDimPoint (dPoints+1, dPoints+2, &ep->dimArc, &ep->rMatrix,
        &crossState, true);

    if (NULL != ep->pDerivedData && NULL != ep->pDerivedData->pArcDefPoint)
        ep->pDerivedData->pArcDefPoint[0] = dPoints[1];

    if (status = generateRadialPointer (ep, dPoints, nPoints, dimLength))
        return (status);

    if (ep->GetDimElementCP()->tmpl.centermark)
        status = adim_generateDimCenter (ep);

    return (status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/89
+---------------+---------------+---------------+---------------+---------------+------*/
static int      adim_generateDimDiameter
(
AdimProcess     *ep,         /* => Function used to process elements   */
DPoint3d        *dPoints,
int             nPoints,
DimText const   *dimText      /* => Dimension text information         */
)
    {
    DVec3d      dir, check;
    DPoint3d    firstPoint;
    double      dimLength;
    int         status;

    dimLength = ep->dimArc.radius * 2.0;
    if (ep->pdDimLengths != NULL)
        ep->pdDimLengths[ADIM_GETSEG (ep->partName)] = dimLength;

    bsiDVec3d_subtractDPoint3dDPoint3d (&check, &dPoints[2], &ep->dimArc.center);

    if (ep->GetDimElementCP()->Is3d())
        ep->rMatrix.MultiplyTranspose(check);

    if (0 >= bsiTrig_tolerancedComparison (bsiDVec3d_magnitude (&check), ep->dimArc.radius))
        {                       /* input point is inside circle */
        bsiDVec3d_subtractDPoint3dDPoint3d (&dir, &dPoints[2], &ep->dimArc.center);
        if (ep->GetDimElementCP()->Is3d())
            {
            ep->rMatrix.MultiplyTranspose(dir);
            dir.z = 0.0;
            ep->rMatrix.Multiply(dir);
            }

        dir.Normalize ();
        bsiDPoint3d_addScaledDVec3d (dPoints+2, &ep->dimArc.center, &dir, ep->dimArc.radius);
        bsiDPoint3d_addScaledDVec3d (dPoints+1, &ep->dimArc.center, &dir, -ep->dimArc.radius);

        firstPoint = dPoints[2];
        if (status = adim_generateDimension (NULL, ep, dPoints+1, dPoints+2,
            &firstPoint, dimLength, dimText,
            ep->GetDimElementCP()->tmpl.left_term, ep->GetDimElementCP()->tmpl.right_term))
            return (status);
        }
    else
        {
        bsiDPoint3d_subtractDPoint3dDPoint3d (dPoints+1, &dPoints[2], &ep->dimArc.center);
        if (ep->GetDimElementCP()->Is3d())
            {
            ep->rMatrix.MultiplyTranspose(dPoints[1]);
            dPoints[1].z = 0.0;
            ep->rMatrix.Multiply(dPoints[1]);
            }

        dPoints[0] = ep->dimArc.center;
        ((DVec3d *) dPoints+1)->Normalize ();
        bsiDPoint3d_addScaledDVec3d (dPoints+1, dPoints, (DVec3d *)dPoints+1, ep->dimArc.radius);

        dimLength *= ep->GetDimElementCP()->GetScale();
        if (status = generateRadialPointer (ep, dPoints, nPoints, dimLength))
            return (status);
        }

    if (ep->GetDimElementCP()->tmpl.centermark)
        status = adim_generateDimCenter (ep);

    return (status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/89
+---------------+---------------+---------------+---------------+---------------+------*/
static int      adim_generateDimParDiameter
(
AdimProcess     *ep,         /* => Function used to process elements   */
DPoint3d        *dPoints,
DimText const   *dimText      /* => Dimension text information         */
)
    {
    DPoint2d       tile;
    DPoint3d       start, end, rpoint, tmp, endPoint;
    DVec3d         dir, perpvec, tan1, tan2;
    double         distance, offset, dimLength;
    int            status;
    DimensionElm const* dim = ep->GetDimElementCP();
    dimLength = ep->dimArc.radius * 2.0;
    if (ep->pdDimLengths != NULL)
        ep->pdDimLengths[ADIM_GETSEG (ep->partName)] = dimLength;

    rpoint = dPoints[1];
    dir.NormalizedDifference (rpoint, *( &ep->dimArc.center));

    switch (dim->flag.alignment)
        {
        case 1:                        /* drawing axis */
            tmp = dir;
            dir.init (0.0, 0.0, 0.0);
            if (fabs (tmp.x) > fabs (tmp.y))
                dir.x = tmp.x > 0.0 ? 1.0: - 1.0;
            else
                dir.y = tmp.y > 0.0 ? 1.0: - 1.0;
            break;

        case 0:                        /* view axis */
            tmp = dir;
            dir.init (0.0, 0.0, 0.0);
            ep->vuMatrix.Multiply(tmp);
            if (fabs (tmp.x) > fabs (tmp.y))
                dir.x = tmp.x > 0.0 ? 1.0: - 1.0;
            else
                dir.y = tmp.y > 0.0 ? 1.0: - 1.0;
            ep->vuMatrix.MultiplyTranspose(dir);
            break;
        }

    if (dim->Is3d())
        {
        ep->rMatrix.MultiplyTranspose(dir);
        dir.z = 0.0;
        perpvec.x = - dir.y;
        perpvec.y = dir.x;
        perpvec.z = 0.0;
        ep->rMatrix.Multiply(dir);
        ep->rMatrix.Multiply(perpvec);
        }
    else
        {
        perpvec.x = -dir.y;
        perpvec.y = dir.x;
        perpvec.z = 0.0;
        }

    perpvec.Normalize ();
    dir.Normalize ();

    bsiDPoint3d_addScaledDVec3d (&tan1, &ep->dimArc.center, &perpvec, ep->dimArc.radius);
    dPoints[0] = tan1;
    bsiDPoint3d_addScaledDVec3d (&tan2, &ep->dimArc.center, &perpvec, -ep->dimArc.radius);

    /* Update non-associative dimension points */
    //WIP: BEIJING_AB
    //The code below should not be evaluated at stroke but while updating the dependancy linkage
    //dim->point[1].point = tan1;

    DVec3d rVec;
    bsiDVec3d_subtractDPoint3dDPoint3d (&rVec, &rpoint, &ep->dimArc.center);
    distance = bsiDVec3d_dotProduct (&dir, &rVec);

    bsiDPoint3d_addScaledDVec3d (&start, &tan1, &dir, distance);
    bsiDPoint3d_addScaledDVec3d (&end,   &tan2, &dir, distance);

    endPoint = dPoints[1];
    if (status = adim_generateDimension (NULL, ep, &start, &end, &endPoint,
        dimLength, dimText, dim->tmpl.left_term, dim->tmpl.right_term))
        {
        ep->ep.segment = 2; /* Pointno used in adim_locate */
        return (status);
        }

    adim_getTileSize (&tile, NULL, ep->GetElemHandleCR());

    offset = 0.5 * tile.y;
    bsiDPoint3d_addScaledDVec3d (&tan1,  &tan1,  &dir, offset);
    bsiDPoint3d_addScaledDVec3d (&start, &start, &dir, offset);

    ADIM_SETNAME (ep->partName, ADTYPE_EXT_LEFT, ADSUB_NONE);
    if (dim->tmpl.left_witness)
        if (status = adim_generateLine (ep, &tan1, &start, DIM_MATERIAL_DimLine))
            {
            ep->ep.segment = 1; /* Pointno used in adim_locate */
            return (status);
            }

    bsiDPoint3d_addScaledDVec3d (&tan2, &tan2, &dir, offset);
    bsiDPoint3d_addScaledDVec3d (&end, &end, &dir, offset);

    ADIM_SETNAME (ep->partName, ADTYPE_EXT_RIGHT, ADSUB_NONE);
    if (dim->tmpl.right_witness)
        if (status = adim_generateLine (ep, &tan2, &end, DIM_MATERIAL_DimLine))
            {
            ep->ep.segment = 1; /* Pointno used in adim_locate */
            return (status);
            }

    if (dim->tmpl.centermark)
        status = adim_generateDimCenter (ep);

    return (status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/89
+---------------+---------------+---------------+---------------+---------------+------*/
static int      adim_generateDimPerpDiameter
(
AdimProcess     *ep,         /* => Function used to process elements   */
DPoint3d        *dPoints,
DimText const   *dimText     /* => Dimension text information          */
)
    {
    DPoint2d       tile;
    DPoint3d       start, end, rpoint, endPoint;
    DVec3d         dir, perpvec, tan1, tan2;
    double         distance;
    double         offset, dimLength;
    int            status;

    dimLength = ep->dimArc.radius * 2.0;
    if (ep->pdDimLengths != NULL)
        ep->pdDimLengths[ADIM_GETSEG (ep->partName)] = dimLength;

    rpoint = dPoints[1];

    bsiDVec3d_subtractDPoint3dDPoint3d (&perpvec, &dPoints[1], &ep->dimArc.center);
    ep->rMatrix.MultiplyTranspose(perpvec);
    perpvec.z = 0.0;
    ep->rMatrix.Multiply(perpvec);

    perpvec.Normalize ();

    DVec3d rVec;
    bsiDVec3d_subtractDPoint3dDPoint3d (&rVec, &rpoint, &ep->dimArc.center);

    ep->rMatrix.GetColumn(dir,  2);
    distance = bsiDVec3d_dotProduct (&dir, &rVec);

    bsiDPoint3d_addScaledDVec3d (&tan1, &ep->dimArc.center, &perpvec, ep->dimArc.radius);
    *dPoints = tan1;
    bsiDPoint3d_addScaledDVec3d (&tan2, &ep->dimArc.center, &perpvec, ep->dimArc.radius);
    DimensionElm const* dim = ep->GetDimElementCP();
    /* Update non-associative dimension points */
    //WIP: This update seems un necessary. TODO test the same
    /*dim->point[1].point = tan1;
    dim->point[2].point = tan2;*/

    bsiDPoint3d_addScaledDVec3d (&start, &tan1, &dir, distance);
    bsiDPoint3d_addScaledDVec3d (&end,   &tan2, &dir, distance);

    endPoint = dPoints[1];
    if (status = adim_generateDimension (NULL, ep, &start, &end, &endPoint,
        dimLength, dimText, dim->tmpl.left_term, dim->tmpl.right_term))
        {
        ep->ep.segment = 2; /* Pointno used in adim_locate */
        return (status);
        }

    adim_getTileSize (&tile, NULL, ep->GetElemHandleCR());
    offset = 0.5 * tile.y;

    if (distance < 0.0)
        offset = - offset;

    bsiDPoint3d_addScaledDVec3d (&tan1,  &tan1,  &dir, offset);
    bsiDPoint3d_addScaledDVec3d (&start, &start, &dir, offset);

    if (dim->tmpl.left_witness)
        if (status = adim_generateLine (ep, &tan1, &start, DIM_MATERIAL_DimLine))
            {
            ep->ep.segment = 1; /* Pointno used in adim_locate */
            return (status);
            }

    bsiDPoint3d_addScaledDVec3d (&tan2, &tan2, &dir, offset);
    bsiDPoint3d_addScaledDVec3d (&end,  &end,  &dir, offset);

    if (dim->tmpl.right_witness)
        if (status = adim_generateLine (ep, &tan2, &end, DIM_MATERIAL_DimLine))
            {
            ep->ep.segment = 1; /* Pointno used in adim_locate */
            return (status);
            }

    if (dim->tmpl.centermark)
        status = adim_generateDimCenter (ep);

    return (SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     12/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public int BentleyApi::adim_getDimArcInfo
(
DimArcInfo          *arcInfo,     /* <= Arc(or ellipse) information for dim */
ElementHandleCR        dimElement
)
    {
    
    DimensionElm const* dim = &dimElement.GetElementCP()->ToDimensionElm();
    if (dim->nPoints < 2)
        return ERROR;
    arcInfo->center = dim->GetPoint (0);
    DPoint3d dPoint = dim->GetPoint (1);
    
    arcInfo->radius     = bsiDPoint3d_distance (&arcInfo->center, &dPoint);
    arcInfo->start      = arcInfo->end = dPoint;
    arcInfo->startAng   = 0.0;
    arcInfo->sweepAng   = msGeomConst_2pi;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       RadialDimensionHelper::ReEvaluateElement (EditElementHandleR dimElement) const
    {
    // stroke to fix can change the element size
    return mdlDim_fixRadialPoints (dimElement, NULL);

    
    /*-------------------------------------------------------------------
    If point 0 is associative, point 1 is not actually being used for
    display. Keep it in the right place so that it will be valid if
    the association is dropped.
    -------------------------------------------------------------------*/
    //WIP: BEIJING_DIMENSION_AB
    //The code below should be done only while association update. Not stroke.
    /*if (ep->GetDimElementCP()->nPoints > 2 && ep->GetDimElementCP()->GetDimTextCP(0)->flags.b.associative)
        ep->GetDimElementCP()->point[1].point = *(dPoints+1);*/
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     12/90
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       RadialDimensionHelper::StrokeDimension (AdimProcess& ep) const
    {
    DimensionElm const*  dim = ep.GetDimElementCP();
    switch (static_cast<DimensionType>(dim->dimcmd))//In 8.11 stroke code we used to look at dimtext[2] for the rest of them
        {
        case DimensionType::Diameter:
        case DimensionType::DiameterExtended:
        case DimensionType::Center:
            {
            if (dim->nPoints <2)
                return ERROR;
            break;
            }
        default:
            {
            if (dim->nPoints <3)
                return ERROR;
            }
        }
    
    ep.Init();
    /*---------------------------------------------------------------
      This stroke function can be called from depcallback when the
      element it is associated to has changed, so the dimension
      points need to be re-evaluated. In that case, we use the
      first assoc point to re-evaluate the rest of the points.
      TR-75505 : Center-mark is a special case where we have to
      always go thru this re-evaluation logic, and here's why.
      Center-mark has two points, the first one is associative-center
      and the second one is the accept point which is arbitrary.
      When the centermark is non-associative, the size (radius) of
      the centermark is simply the distance between the two points,
      which is obtained from adim_getDimArcInfo. However, when it is
      associative, we need to get the radius of the associated arc
      by making use of the first assoc point.
    ---------------------------------------------------------------*/
    if (dim->GetDimTextCP(0)->flags.b.associative && (DimensionType::Center == static_cast<DimensionType>(dim->dimcmd)))
        {
        AssocPoint      assoc;

        if (SUCCESS != AssociativePoint::ExtractPoint (assoc, ep.GetElemHandleCR(), 0, dim->nPoints))
            return ERROR;

        if (SUCCESS != BentleyApi::adim_getArcInfo (&ep.dimArc, &ep.rMatrix, NULL, &assoc, ep.GetElemHandleCR()))
            return ERROR;
        }
    else
        {
        if (adim_getDimArcInfo (&ep.dimArc, ep.GetElemHandleCR()))
            return ERROR;
        }

    if (dim->dimcmd == static_cast<byte>(DimensionType::Center))
        return (adim_generateDimCenter (&ep));

    m_hdlr.GetStrings (ep.GetElemHandleCR(), ep.strDat.m_strings, 0, NULL);

    DPoint3d        dPoints[MAX_ADIM_POINTS]; //TODO evaluate why we are doing this. the adim process already allocates memory.
    for (int index = 0; index <dim->nPoints; ++index)
        dPoints[index] = dim->GetPoint(index);
    
    int             status = SUCCESS;
    switch (static_cast<DimensionType>(dim->dimcmd))
        {
        case DimensionType::Diameter:
        case DimensionType::DiameterExtended:
            {
            int nPoints = dim->nPoints;
            DimText const *dimText = NULL;
            if (nPoints == 2)
                {
                bsiDPoint3d_interpolate (dPoints+2, dPoints, 0.5, dPoints+1);
                nPoints = 3;
                dimText = dim->GetDimTextCP(1);
                }
            else 
                dimText = dim->GetDimTextCP(2);
            status = adim_generateDimDiameter (&ep, dPoints, nPoints, dimText);
            break;
            }
        case DimensionType::DiameterParallel:
            status = adim_generateDimParDiameter (&ep, dPoints+1, dim->GetDimTextCP(2));
            break;

        case DimensionType::DiameterPerpendicular:
            status = adim_generateDimPerpDiameter (&ep, dPoints+1, dim->GetDimTextCP(2));
            break;

        case DimensionType::Radius:
        case DimensionType::RadiusExtended:
            status = adim_generateRadiusDimension (&ep, dPoints, dim->nPoints);
            break;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     12/90
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       NoteDimensionHelper::StrokeDimension (AdimProcess& ap) const
    {
    DimensionElm const* dim = ap.GetDimElementCP();
    if (dim->nPoints < 2)
        return ERROR;

    ap.Init();

    DPoint3d dPoints[MAX_ADIM_POINTS]; //TODO evaluate why we cannot use point buffer in AdimProcess
    for (int index = 0; index < dim->nPoints; ++index)
        dPoints[index] = dim->GetPoint (index);

    m_hdlr.GetStrings (ap.GetElemHandleCR(), ap.strDat.m_strings, dim->nPoints-1, NULL);
    return adim_generateNoteDimension (&ap, NULL, dPoints, dim->nPoints, 0.0, true, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool            NoteDimensionHelper::HasText () const
    {
    DimensionElm const* dim = &m_dimension.GetElementCP()->ToDimensionElm();
    if (dim->nPoints < 2)
        return false;
    
    NullOutput  output;
    NullContext context (&output);

    context.SetDgnProject (*m_dimension.GetDgnProject());

    AdimProcess ap (m_dimension, &context);
    ap.Init();

    if (SUCCESS != m_hdlr.GetStrings (m_dimension, ap.strDat.m_strings, dim->nPoints-1, NULL))
        return false;

    //The following line handles the most common case. Left the case where a note can have text because I have not seen one.
    //We do not allow it from UI in Microstation
    if (ap.strDat.GetPrimaryStrings()[0].empty() && !dimTextBlock_getRequiresTextBlock (&ap, NULL))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool            RadialDimensionHelper::HasText() const
    {
    DimensionElm const* dim = &m_dimension.GetElementCP()->ToDimensionElm();
    if (dim->nPoints < 2)
        return false;
    
    if (dim->dimcmd == static_cast<byte>(DimensionType::Center))
        return false;

    return true;
    }