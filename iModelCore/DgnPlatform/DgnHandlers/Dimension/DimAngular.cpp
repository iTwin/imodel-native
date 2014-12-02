/*----------------------------------------------------------------------+
|
|   $Source: DgnHandlers/Dimension/DimAngular.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    "DimStyleInternal.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/86
+---------------+---------------+---------------+---------------+---------------+------*/
static void angle_to_direction
(
DPoint3d  *direction,
double    angle
)
    {
    direction->x = cos(angle);
    direction->y = sin(angle);
    direction->z = 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void    BentleyApi::adim_getFloatingAngularFormat
(
AngleFormatVals *pFormat,       /* <=> */
int             *pAccuracy      /* <=> */
)
    {
    /*--------------------------------------------------------------------------
        This logic implements the AutoCad concept we call 'floating accuracy',
        where the accuracy value is used to determine the format.
    --------------------------------------------------------------------------*/

    switch (*pFormat)
        {
        case AngleFormatVals::DegMin:
            {
            if (0 >= *pAccuracy)
                {
                *pFormat    = AngleFormatVals::Degrees;
                *pAccuracy  = 0;
                }
            else
                {
                *pFormat    = AngleFormatVals::DegMin;
                *pAccuracy -= 1;
                }
            }
        case AngleFormatVals::DegMinSec:
            {
            if (0 >= *pAccuracy)
                {
                *pFormat    = AngleFormatVals::Degrees;
                *pAccuracy  = 0;
                }
            else
            if (1 == *pAccuracy)
                {
                *pFormat    = AngleFormatVals::DegMin;
                *pAccuracy  = 0;
                }
            else
                {
                *pFormat    = AngleFormatVals::DegMinSec;
                *pAccuracy -= 2;
                }

            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     1/90
+---------------+---------------+---------------+---------------+---------------+------*/
static void     adim_resetStartPoint
(
DPoint3d        *point,
AdimProcess     *ap
)
    {
    DPoint3d    centerPoint, dPoint;
    DVec3d      direction;
    double   rad;

    if (ap->GetDimElementCP()->tmpl.stacked)
        {
        centerPoint = ap->points[2];
        dPoint = ap->points[1];

        rad = bsiDPoint3d_distance (&centerPoint, &dPoint);

        direction.NormalizedDifference (dPoint, centerPoint);
        bsiDPoint3d_addScaledDVec3d (&dPoint, &centerPoint, &direction, rad + ap->stackHeight);

        ap->points[1] =dPoint;
        }
    else
        {
        ap->points[0] = *point;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     1/90
+---------------+---------------+---------------+---------------+---------------+------*/
int BentleyApi::adim_getSweepTrans
(
double    *sweepAngle,       /* <= Sweep angle                         */
RotMatrix *outMatrix,        /* <= Rotation matrix (if not NULL)       */
DPoint3d  *origin,           /* => Origin of vectors                   */
DPoint3d  *xAxis,            /* => End of first line                   */
DPoint3d  *yAxis,            /* => End of second line                  */
RotMatrix *cTrans,
bool       i3d
)
    {
    RotMatrix tmpMatrix, *rMatrix;
    DPoint3d  dPoint;
    DVec3d      xColumn, yColumn, zColumn, cTransZ;
    int i;

    rMatrix = outMatrix ? outMatrix : &tmpMatrix;

    xColumn.NormalizedDifference (*xAxis, *origin);

    if (i3d)
        {
        bsiDVec3d_subtractDPoint3dDPoint3d (&yColumn, yAxis, origin);

        bsiDVec3d_crossProduct (&zColumn, &xColumn, &yColumn);
        cTrans->GetColumn(cTransZ,  2);

        if (bsiDVec3d_magnitude (&zColumn) < mgds_fc_epsilon)
            zColumn = cTransZ;

        zColumn.Normalize ();
        bsiDVec3d_crossProduct (&yColumn, &zColumn, &xColumn);
        yColumn.Normalize ();
        bsiDVec3d_crossProduct (&zColumn, &xColumn, &yColumn);

        rMatrix->InitFromColumnVectors(xColumn, yColumn, zColumn);

        if (bsiDVec3d_dotProduct (&cTransZ, &zColumn) < 0.0)
            {
            for (i=0; i<3; i++)
                {
                rMatrix->form3d[i][1] = - rMatrix->form3d[i][1];
                rMatrix->form3d[i][2] = - rMatrix->form3d[i][2];
                }
             }
        }
    else
        {
        zColumn.x = zColumn.y = 0.0;
        zColumn.z = 1.0;
        bsiDVec3d_crossProduct (&yColumn, &zColumn, &xColumn);
        rMatrix->InitFromColumnVectors(xColumn, yColumn, zColumn);
        }

    bsiDPoint3d_subtractDPoint3dDPoint3d (&dPoint, yAxis, origin);
    rMatrix->MultiplyTranspose(dPoint);

    *sweepAngle = Angle::Atan2 (dPoint.y, dPoint.x);

    if (*sweepAngle < 0.0)
        *sweepAngle += msGeomConst_2pi;

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     1/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int generateArcTerminator
(
AdimProcess     *ep,
DPoint3d        *cntr,
double          rad,
RotMatrix       *trans,
double          angle,
int             clockwise,
int             termindx
)
    {
    DPoint3d  endpt, start;
    DVec3d   direction;
    double   addAngle;

    double tw = ep->GetDimElementCP()->geom.termWidth;
    if (tw >= rad)
        return (SUCCESS);

    /* point where dimension arc intersects witness line */
    endpt.x = rad * cos (angle);
    endpt.y = rad * sin (angle);
    endpt.z = 0.0;
    trans->Multiply(endpt);
    bsiDPoint3d_addDPoint3dInPlace (&endpt, cntr);

    /* find angle to other end of terminator */
    addAngle = Angle::Acos (- ((tw*tw) / (rad*rad*2.0) - 1.0));
    angle   += clockwise ? addAngle : -addAngle;

    /* point where other end of terminator crosses dimension arc */
    start.x = rad * cos (angle);
    start.y = rad * sin (angle);
    start.z = 0.0;
    trans->Multiply(start);
    bsiDPoint3d_addDPoint3dInPlace (&start, cntr);

    direction.NormalizedDifference (endpt, start);

    return (adim_generateTerminator (ep, &endpt, &direction, termindx));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
static int     getArcMidPoint
(
DPoint3d            *pPointOnArc,   /* <= mid point on arc */
DPoint3d const*     pCenter,       /* => arc center point */
double              sweepValue,     /* => angle sweep */
double              radius,         /* => arc radius */
RotMatrix const*    pRotMatrix     /* => dimension rotation */
)
    {
    DVec3d    dir;

    /* use arc mid point as leader start point */
    dir.x = cos (sweepValue / 2.0);
    dir.y = sin (sweepValue / 2.0);
    dir.z = 0.0;

    pRotMatrix->Multiply(dir);
    *pPointOnArc = *pCenter;
    bsiDPoint3d_addScaledDVec3d (pPointOnArc, pPointOnArc, &dir, radius);

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
static double  getAngularStackOffset
(
const DimensionElm  *pDimension,
const DPoint3d      *pCheck,
DPoint2dCP          pTextSize,
const double        charwdth,
const bool          byPassHorizontal
)
    {
    double  stackHeight;

    if (!pDimension->tmpl.stacked)
        return  0.0;

    if (pDimension->geom.stackOffset)
        {
        stackHeight = (double)pDimension->geom.stackOffset;
        return  stackHeight;
        }
    else
        {
        stackHeight = charwdth + pTextSize->y;

        if (pDimension->flag.horizontal && !byPassHorizontal)
            {
            if (fabs (pCheck->x * pTextSize->x) > fabs (pCheck->y * pTextSize->y))
                stackHeight = charwdth + fabs (pTextSize->y / pCheck->x);
            else
                stackHeight = charwdth + fabs ((pTextSize->x + charwdth) / pCheck->y);
            }
        }

    return  stackHeight;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
AngleFormatVals BentleyApi::adim_getAngleFormat (ElementHandleCR dimElement)
    {
    DimensionElm const* pDim = &dimElement.GetElementCP()->ToDimensionElm();
    if (pDim->flag.centesimal)
        return AngleFormatVals::Centesimal;
    else
    if (pDim->frmt.radians)
        return AngleFormatVals::Radians;
    else
        return static_cast<AngleFormatVals>(pDim->frmt.angleFormat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       generateAngleText
(
AdimProcess     *ep,
DPoint3d        *origin,
DVec3d          *direction
)
    {
    int     iStatus;

    if (-1 != (iStatus = adim_generateTextUsingDescr (ep, origin, direction)))
        return  iStatus;

    WStringP    str = ep->strDat.m_strings.GetString (DIMTEXTPART_Primary, DIMTEXTSUBPART_Main);

    return  adim_generateText (ep, str->c_str(), origin, direction, adim_getDimTextJustificationHorizontal (ep), NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void    adim_stringFromAngle
(
WChar*          angleString,    /* <=  string */
int             numChars,       /*  => numChars in output buffer */
double          angle,          /*  => angle in degrees */
AngleFormatVals format,         /*  => 0 degrees, 1 deg-min-sec, 2 centesimal */
int             precision,      /*  => 1-8 number of digits after the decimal, -1 for active */
bool            leadingZero,    /*  => false=strip, true=allow */
bool            trailingZeros   /*  => false=strip, true=allow */
)
    {
    AngleFormatterPtr  formatter = AngleFormatter::Create();

    formatter->SetAngleModeFromLegacy (format);
    formatter->SetAnglePrecisionFromLegacy (precision);

    formatter->SetLeadingZero   (TO_BOOL(leadingZero));
    formatter->SetTrailingZeros (TO_BOOL(trailingZeros));
    formatter->SetAllowNegative (false);

    WString resultStr = formatter->ToString(angle);

    wcsncpy (angleString, resultStr.c_str(), numChars);
    angleString[numChars-1] = '\0';
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  03/02
+---------------+---------------+---------------+---------------+---------------+------*/
static bool            getShowTolerance
(
const AdimProcess*    ep
)
    {
    return  ep->GetDimElementCP()->version >= 7   &&
            ep->GetDimElementCP()->flag.tolerance && ep->GetDimElementCP()->flag.tolmode == 0 &&
            NULL != mdlDim_getOptionBlock (ep->GetElemHandleCR(), ADBLK_TOLERANCE, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  03/02
+---------------+---------------+---------------+---------------+---------------+------*/
static void    getMeasureStrings
(
AdimProcess*    ep,
DPoint2dP       pTextSize,
DPoint2dP       pTileSize,
double*         pdCharWidth,
const double    dSweepValue,
const double    dRadiusForLength,
const bool      bShowTolerance
)
    {
    double  dimLength;
    ElementHandleCR dimElement =  ep->GetElemHandleCR();
    DimensionElm const* dim = &dimElement.GetElementCP()->ToDimensionElm();
    
    if (dim->frmt.angleMeasure)
        {
        AngleFormatVals angleFormat     = adim_getAngleFormat (dimElement);
        int             angleAccuracy   = dim->frmt.primaryAccuracy;
        double          angle           = dSweepValue * msGeomConst_degreesPerRadian;
        WChar           dim_string[MAX_DIMSTR];

        if (ep->pdDimLengths != NULL)
            ep->pdDimLengths[ADIM_GETSEG(ep->partName)] = angle;

        if (DIMSTYLE_VALUE_Value_DMSPrecisionMode_Floating == dim->frmt.dmsAccuracyMode)
            adim_getFloatingAngularFormat (&angleFormat, &angleAccuracy);

        /*-------------------------------------------------------------------------------
        If the dimension is version 7 (MSJ) and uses plus/minus tolerance, divert into
        angular tolerance generation.
        -------------------------------------------------------------------------------*/
        if (bShowTolerance)
            {
            DimUnitBlock    dimUnits;
            static UChar    precMap[9] = {0x00, 0x81, 0x82, 0x84, 0x88, 0x90, 0xa0, 0xb0, 0xc0};

            /*---------------------------------------------------------------------------
            The following section is designed to add support for plus/minus tolerances
            on angular dimensions - with the lowest possible impact.

            _getLengthStrings() generates all three strings. The dimension length
            it generates is used only to set the overall size of the string correctly.
            The value is then replaced - to get support fo centesimal degrees etc.

            Since getLengthString is assuming UOR's and linear precision, we need to
            scale the value by SU*PU and map the angular precision values to linear.
            ---------------------------------------------------------------------------*/
            if (8 < angleAccuracy)
                angleAccuracy = 8;

            adim_getDimensionUnits (&dimUnits, NULL, NULL, dimElement, true);

            dimLength = angle * dimUnits.uorPerMast;

            WStringP mainStrP = ep->strDat.m_strings.GetString (DIMTEXTPART_Primary, DIMTEXTSUBPART_Main);
            WString  backup (mainStrP->c_str());

            //On investigation the setting an unsetting of primary accuracy is for the
            //get length string function.
            adim_getLengthStrings (ep, dimLength, precMap[angleAccuracy]);

            ep->strDat.upperSize.x += *pdCharWidth;

            mainStrP->assign (backup);
            adim_stringFromAngle (dim_string, _countof (dim_string), angle, angleFormat, angleAccuracy, dim->flag.leadingZero, dim->flag.trailingZeros);

            adim_insertLengthString (*mainStrP, dim_string);
            adim_getTextSize (pTextSize, ep, &ep->strDat, ADIM_TEXTSIZE_Nominal);
            *pdCharWidth = ep->strDat.charWidth;
            }
        else
            {
            /*---------------------------------------------------------------------------
            This was the only angle measure option prior to MSJ
            ---------------------------------------------------------------------------*/
            adim_stringFromAngle (dim_string, _countof (dim_string), angle, angleFormat, angleAccuracy, dim->flag.leadingZero, dim->flag.trailingZeros);

            WStringP mainStrP = ep->strDat.m_strings.GetString (DIMTEXTPART_Primary, DIMTEXTSUBPART_Main);
            adim_insertLengthString (*mainStrP, dim_string);

            if (! dimTextBlock_getRequiresTextBlock (ep, NULL))
                {
                // keep this around for legacy support
                adim_getStringSize (pTextSize, mainStrP->c_str(), pTileSize, dim->text.font, ep);
                }
            else
                adim_getTextSize (pTextSize, ep, &ep->strDat, ADIM_TEXTSIZE_Nominal);
            }
        }
    else
        {
        dimLength = dSweepValue * dRadiusForLength * dim->GetScale();
        adim_getLengthStrings (ep, dimLength, dim->frmt.primaryAccuracy);
        adim_getTextSize (pTextSize, ep, &ep->strDat, ADIM_TEXTSIZE_Nominal);

        *pdCharWidth = ep->strDat.charWidth;

        if (ep->pdDimLengths != NULL)
            ep->pdDimLengths[ADIM_GETSEG(ep->partName)] = dimLength;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  03/02
+---------------+---------------+---------------+---------------+---------------+------*/
static void    getCheckFromAngle
(
DPoint3d*           pCheck,
const double        angle,
const RotMatrix*    pTrans,
const AdimProcess*  ep
)
    {
    DPoint3d    dir;

    angle_to_direction (&dir, angle);
    pCheck->x = - dir.y;
    pCheck->y = dir.x;
    pCheck->z = 0.0;

    pTrans->Multiply(*pCheck);  /* to database coords */
    ep->vuMatrix.Multiply(*pCheck);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void    applyAutoBallNChainForHorizontalText
(
DPoint3d            *pTextOrigin,
double              offsetx,
DPoint2dCP          pTextSize,
double              arcLength,
double              radius,
DPoint3d  const     *pCenter,
RotMatrix const     *pTrans,
AdimProcess const   *pAdimProcess
)
    {
    DVec3d          normal, arcMid, originOffset, slantOffset;
    double          compSlantAngle, textShift, dimHeight = 0.0;
    double          slantAngle = 0.174532925;   // 10
    
    double          normalAngle = 0.5 * arcLength / radius;
    
    ElementHandleCR dimElement =  pAdimProcess->GetElemHandleCR();
    DimensionElm const* dim = &dimElement.GetElementCP()->ToDimensionElm();
    double      slantLength = 2 * dim->geom.termWidth;

    mdlDim_getHeightDirect (&dimHeight, dimElement, 0);

    angle_to_direction (&normal, normalAngle);
    pTrans->Multiply(normal);
    pAdimProcess->vuMatrix.Multiply(normal);

    /*-----------------------------------------------------------------------------------
    Text offset calculated in BentleyApi::adim_calcTextOffset does not include the slant angle needed
    specifically for angular dimension with horizontal text.  Now we need to slant the
    offset by 10 when dim arc is above dimenion height.  As such, offsetY is completely
    ignored here.

    When dim arc is below dimension height, we want to always keep text above dimension
    height, and we also snap the leader to normal (i.e. no slant angle).
    -----------------------------------------------------------------------------------*/
    if (dimHeight < 0.0)
        {
        slantAngle = 0.0;
        slantLength -= dimHeight;
        }

    normalAngle = acos (normal.x);
    if (normalAngle > msGeomConst_piOver2 + mgds_fc_epsilon)
        slantAngle = -slantAngle;
    compSlantAngle = normalAngle - slantAngle;

    slantOffset.x = slantLength * cos(compSlantAngle);
    slantOffset.y = slantLength * sin(compSlantAngle);

    originOffset.x = offsetx+ slantOffset.x;
    originOffset.y = slantOffset.y;
    originOffset.z = 0.0;

    // always push text outward normally
    if (normal.y < -mgds_fc_epsilon)
        originOffset.y = -originOffset.y;

    // add a text margin
    textShift = pAdimProcess->GetDimElementCP()->geom.textMargin;

    // apply text justification
    textShift -= BentleyApi::adim_computeLeftEdgeOffset(pAdimProcess, 0.0, pTextSize->x);

    originOffset.x += textShift;

    // build offset bvector from arc center to text origin on the view plane
    bsiDVec3d_scale (&arcMid, &normal, radius);
    bsiDPoint3d_addDPoint3dDPoint3d (&originOffset, &arcMid, &originOffset);

    // tranform the offset bvector to the world
    pAdimProcess->vuMatrix.MultiplyTranspose(originOffset);

    bsiDPoint3d_addDPoint3dDPoint3d (pTextOrigin, pCenter, &originOffset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/06
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        isAutoBallAndChainInEffect
(
AdimProcess const   *pAdimProcess,
int                 textJust
)
    {
    if (DIMTEXT_OFFSET != textJust && (pAdimProcess->flags.allDontFit || pAdimProcess->flags.textNotFit))
        {
        DimStyleProp_BallAndChain_Mode  bncMode = DIMSTYLE_VALUE_BallAndChain_Mode_None;

        mdlDim_getBallNChainMode (&bncMode, pAdimProcess->GetElemHandleCR());

        return  DIMSTYLE_VALUE_BallAndChain_Mode_Auto == bncMode;
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/06
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    isLineNearlyHorizontal
(
DPoint3d const  *pPoint1,
DPoint3d const  *pPoint2,
AdimProcess const *pAdimProcess
)
    {
    DVec3d      bvector;

    bsiDVec3d_subtractDPoint3dDPoint3d (&bvector, pPoint1, pPoint2);
    bvector.Normalize ();

    pAdimProcess->vuMatrix.Multiply(bvector);

    // 15 range
    return  bvector.x > 0.75;
    }

/*---------------------------------------------------------------------------------**//**
* Text is placed horizontal
* @bsimethod                                                    petri.niiranen  03/02
+---------------+---------------+---------------+---------------+---------------+------*/
static void    getHorizontalTextPosition
(
AdimProcess*            ep,
DPoint3d*               pTextOrg,
DVec3d*                 pTextDirection,
DPoint3d*               pNormal,
double*                 pdTextSw,
double*                 pdTextOff,
DPoint3d  const* const  pCenter,
DPoint3d  const* const  pCheck,
DPoint3d  const*        pWitPoint1,
RotMatrix const* const  pTrans,
double           const  dDimLineRadius,
double           const  dArcLength,
DPoint2dCP              pTextSize,
double           const  dCharWidth,
double                  minLeader,
bool             const  leadered,
int                     textJust,
bool                    pushRight,
DPoint2d&               offset
)
    {
    double      textCenterAngle, linear_off;
    double      intersect_len   = 0.0;
    double      textMargin      = ep->GetDimElementCP()->geom.textMargin;
    double      distanceToShift = 0.0;
    DVec3d      textCenterDirection;
    bool        shiftTextAside = ep->flags.pushTextOutside;
    DimStyleProp_Text_Location textLocation = DIMSTYLE_VALUE_Text_Location_Inline;

    mdlDim_overridesGetTextLocation (&textLocation, ep->pOverrides, ep->flags.embed);
    /*-----------------------------------------------------------------------------------
    We need to shift text aside (left or right) when text is moved outside due to fitting.
    This is to reduce text collision with dimension arcs.  We need this for inline and
    above text justifications.  Outside and TopLeft justifications will have text moved
    later on in computeTextShift.
    -----------------------------------------------------------------------------------*/
    if (DIMSTYLE_VALUE_FitOption_KeepTextInside == ep->flags.fitOption ||
        DIMSTYLE_VALUE_Text_Location_Outside == textLocation ||
        DIMSTYLE_VALUE_Text_Location_TopLeft == textLocation)
        shiftTextAside = false;

    // -------------------------------
    // horizontal text
    // -------------------------------
    memset (pTextDirection, 0, sizeof (*pTextDirection));
    pTextDirection->x = 1.0;
    ep->vuMatrix.MultiplyTranspose(*pTextDirection);

    if (leadered && isAutoBallAndChainInEffect(ep, textJust))
        {
        *pdTextSw = *pdTextOff = 0.0;
        return  applyAutoBallNChainForHorizontalText(pTextOrg, offset.x, pTextSize, dArcLength, dDimLineRadius, pCenter, pTrans, ep);
        }

    // Get the length along the arc overlapped by the textbox.
    // NEEDSWORK : The tangent direction used was precomputed by using the full string length
    //             which is not the right direction to use. We have to use the direction at the
    //             center of the 'future' text location
    intersect_len   = BentleyApi::adim_getIntersectLength (pCheck, pTextSize, 0.0);

    ep->stackHeight = getAngularStackOffset (ep->GetDimElementCP(), pCheck, pTextSize, dCharWidth, leadered);

    // offset along dimline
    BentleyApi::adim_calcTextOffset (offset, textJust, pushRight, dArcLength, shiftTextAside ? 0.0 : intersect_len, dCharWidth,
                         minLeader + textMargin, 0, pTextSize, pTextDirection, (DVec3d *) pCheck, ep);

    // -------------------------------
    // text origin
    // -------------------------------
    linear_off   = offset.x;
    linear_off  += leadered || shiftTextAside ? 0.0 : intersect_len / 2.0;
    textCenterAngle = linear_off / dDimLineRadius;

    if (ep->flags.pushTextOutside && ep->flags.ignoreMinLeader && DIMTEXT_OFFSET != textJust && fabs(textCenterAngle) > msGeomConst_pi)
        textCenterAngle = textCenterAngle < 0.0 ? -msGeomConst_pi : msGeomConst_pi;

    angle_to_direction     (&textCenterDirection, textCenterAngle);
    pTrans->Multiply(textCenterDirection);
    bsiDPoint3d_addScaledDVec3d    (pTextOrg, pCenter, &textCenterDirection, (dDimLineRadius + offset.y));

    *pNormal = textCenterDirection;

    // At this point,
    // for non leadered : pTextOrg is the origin at the center of the text. Move origin to the left-center
    //                    point (ie. by half of text width)
    // for leadered     : pTextOrg is the origin at the nearer edge of the text. Move origin to the leader
    //                    end point (ie. by the distance of textMargin)
    if (leadered)
        {
        distanceToShift = textMargin;
        }
    else if (shiftTextAside)
        {
        /*-------------------------------------------------------------------------------
        Text now can collide with dim arcs or witness lines if use center point as origin.
        Shift text left/right to attach either side at minleader point.  Now textCenterAngle
        is actually min leader angle.  Shift text left or right side based on minleader
        arc direction: attach left side if minleader running towards left; attach right
        side if minleader running right.
        -------------------------------------------------------------------------------*/
        if (fabs(textCenterAngle) >= msGeomConst_pi || dDimLineRadius <= pTextSize->y)
            {
            DVec3d    delta;
            bsiDVec3d_subtractDPoint3dDPoint3d (&delta, pTextOrg, pCenter);
            ep->vuMatrix.Multiply(*((DPoint3dP)&delta));

            // small enough that we simply move text out of the "circle", to right
            distanceToShift = dDimLineRadius - delta.x + textMargin;

            // or to left if overlapped by start witness line
            if (isLineNearlyHorizontal(pWitPoint1, pCenter, ep))
                distanceToShift -= pTextSize->x + dDimLineRadius + 2 * textMargin;
            }
        else
            {
            DPoint3d    minLeaderTangent;

            getCheckFromAngle (&minLeaderTangent, textCenterAngle, pTrans, ep);

            if ((linear_off > 0.0 && minLeaderTangent.x < 0.25) || (linear_off < 0.0 && minLeaderTangent.x > 0.25))
                distanceToShift = -pTextSize->x - textMargin;
            else
                distanceToShift = textMargin;
            }
        }
    else
        {
        distanceToShift = -pTextSize->x / 2.0;
        }

    // Apply any text justification override
    distanceToShift += - (BentleyApi::adim_computeLeftEdgeOffset (ep, 0.0, pTextSize->x));

    bsiDPoint3d_addScaledDVec3d (pTextOrg, pTextOrg, pTextDirection, distanceToShift);

    *pdTextSw  = (intersect_len + 2.0 * textMargin) / dDimLineRadius;
    *pdTextOff = textCenterAngle - (intersect_len / 2.0 + textMargin) / dDimLineRadius;
    }

/*---------------------------------------------------------------------------------**//**
* Text is aligned with dimension line above or inline
* @bsimethod                                                    petri.niiranen  03/02
+---------------+---------------+---------------+---------------+---------------+------*/
static void    getAlignedTextPosition
(
AdimProcess*            ep,
DPoint3d*               pTextOrg,
DVec3d*                 pTextDirectionOut,
DPoint3d*               pNormalPoint,
double*                 pdTextSw,
double*                 pdTextOff,
DPoint3d  const* const  pCenter,
DPoint3d  const* const  pCheck,
RotMatrix const* const  pTrans,
double    const         dDimLineRadius,
double    const         dArcLength,
DPoint2dCP              pTextSize,
double    const         dCharWidth,
double                  minLeader,
bool      const         leadered,
int                     textJust, 
DPoint2dCR              offset
)
    {
    double      reversalScalar;
    double      linear_off, textOriginDist;
    double      textMargin  = ep->GetDimElementCP()->geom.textMargin;
    double      center_angle, noFitShift=0;
    DPoint3d    minLeaderPoint;
    DPoint3d    check       = *pCheck;
    bool        bFits = true, multiline   = ep->flags.textBlockPopulated;
    bool        isMinLeaderTooLarge = false, isAutoBnc = false;
    DimStyleProp_Text_Location textLocation = DIMSTYLE_VALUE_Text_Location_Inline;

    mdlDim_overridesGetTextLocation (&textLocation, ep->pOverrides, ep->flags.embed);

    // text direction
    ep->vuMatrix.MultiplyTranspose(check);
    pTrans->MultiplyTranspose(check);
    pTextDirectionOut->x     = check.x;
    pTextDirectionOut->y     = check.y;
    pTextDirectionOut->z     = 0.0;

    // NOTE : pTrans   = Dimension Coordinates to Global Coordinates
    // NOTE : vuMatrix = Global Coordinates to View Coordinates

    // don't justify text when ball and chain is used
    if (leadered)
        {
        mdlDim_overridesClearSegmentPropertyBit (ep->pOverrides, ADIM_GETSEG (ep->partName), SEGMENT_Override_TextJustification);
        isAutoBnc = isAutoBallAndChainInEffect (ep, textJust);
        }

    // linear_off represents the (unflipped) length along the arc from the start extension line to the start of the text
    textOriginDist = linear_off = offset.x;

    // Check if the text string fits between the extension lines. If the text does not fit,
    // it needs to be placed outside the extension lines.
    if (ep->flags.textNotFit || (ep->flags.allDontFit && ep->flags.pushTextOutside))
        bFits = false;

    /*-------------------------------------------------------------------
      1. Decide what the text direction is going to be before checking if
         flipping is necessary.
    -------------------------------------------------------------------*/
    if (!bFits)
        {
        DVec3d      minLeaderDirection;

        if (isAutoBnc || DIMSTYLE_VALUE_FitOption_KeepTextInside == ep->flags.fitOption)
            {
            // text center is at dim arc center
            linear_off = 0.5 * dArcLength;
            center_angle = linear_off / dDimLineRadius;

            switch  (adim_getDimTextJustificationHorizontal (ep))
                {
                case TextElementJustification::LeftMiddle:
                    noFitShift =  pTextSize->x / 2;
                    break;
                case TextElementJustification::RightMiddle:
                    noFitShift = -pTextSize->x / 2;
                    break;
                default:
                    noFitShift =  0.0;
                }
            }
        else
            {
            if (textJust  == DIMTEXT_START   ||
                textJust  == DIMTEXT_CENTER  ||
                (textJust == DIMTEXT_OFFSET &&
                linear_off              <= 0.0             ))
                noFitShift = (ep->flags.ignoreMinLeader || minLeader < textMargin) ? pTextSize->x : pTextSize->x + textMargin;
            else
                noFitShift = (ep->flags.ignoreMinLeader || minLeader < textMargin) ? 0.0 : -textMargin;

            center_angle = (linear_off + noFitShift) / dDimLineRadius;

            if (!leadered && DIMSTYLE_VALUE_FitOption_KeepTextInside != ep->flags.fitOption && ep->flags.ignoreMinLeader && fabs(center_angle) > msGeomConst_pi)
                {
                center_angle = center_angle < 0.0 ? -msGeomConst_pi : msGeomConst_pi;
                isMinLeaderTooLarge = true;
                }
            }

        // Compute the min leader point for later use
        angle_to_direction (&minLeaderDirection, center_angle);
        pTrans->Multiply(minLeaderDirection);
        bsiDPoint3d_addScaledDVec3d (&minLeaderPoint, pCenter, &minLeaderDirection, dDimLineRadius);
        }
    else
        {
        center_angle = (linear_off + pTextSize->x / 2.0) / dDimLineRadius;
        }

    // Determine the origin to textcenter bvector in global. This will be used for applying y-offset radially.
    DVec3d      textCenterDirection;
    angle_to_direction (&textCenterDirection, center_angle);
    pTrans->Multiply(textCenterDirection);

    // Determine the text flow direction in view coordinates so that we can check for readability
    DVec3d  textDirectionInView;
    getCheckFromAngle (&textDirectionInView, center_angle, pTrans, ep);

    /*-------------------------------------------------------------------
      2. Determine the distance from the start extension line to text start
         by flipping the text if necessary
    -------------------------------------------------------------------*/
    if (textDirectionInView.x < -mgds_fc_epsilon)
        {
        reversalScalar  = -1.0;

        if (!bFits && (isAutoBnc || DIMSTYLE_VALUE_FitOption_KeepTextInside == ep->flags.fitOption))
            {
            noFitShift = -noFitShift;
            }
        else if (! leadered)
            {
            // Since the text needs to be flipped, the text start location moves by the text width
            linear_off += pTextSize->x;

            // Since the text needs to be flipped, the noFitShift values need to be recomputed.
            if (!bFits && !isMinLeaderTooLarge && DIMSTYLE_VALUE_FitOption_KeepTextInside != ep->flags.fitOption)
                {
                if (textJust  == DIMTEXT_START   ||
                   textJust == DIMTEXT_CENTER  ||
                    (textJust == DIMTEXT_OFFSET &&
                     linear_off             <= 0.0             ))
                    noFitShift = ep->flags.ignoreMinLeader ? 0.0 : textMargin;
                else
                    noFitShift = ep->flags.ignoreMinLeader ? -pTextSize->x : -(pTextSize->x + textMargin);
                }

            // Compute the text origin distance based on text justification overrides
            textOriginDist = linear_off + BentleyApi::adim_computeLeftEdgeOffset (ep, 0.0, pTextSize->x);
            }

        pTextDirectionOut->x = -textDirectionInView.x;
        pTextDirectionOut->y = -textDirectionInView.y;
        }
    else
        {
        reversalScalar  = 1.0;

        *pTextDirectionOut    = textDirectionInView;

        if (! leadered)
            {
            // Compute the text origin distance based on text justification overrides
            textOriginDist = linear_off - BentleyApi::adim_computeLeftEdgeOffset (ep, 0.0, pTextSize->x);
            }
        }

    if (isMinLeaderTooLarge && fabs(textOriginDist/dDimLineRadius) > msGeomConst_pi)
        textOriginDist = textOriginDist < 0.0 ? -dDimLineRadius*msGeomConst_pi : dDimLineRadius*msGeomConst_pi;

    // Transform text direction from view to global
    ep->vuMatrix.MultiplyTranspose(*pTextDirectionOut);

    /*-------------------------------------------------------------------
      3. Do the necessary shifting and flipping
    -------------------------------------------------------------------*/

    // Compute the text origin.
    // If the text does not fit and is not 'Above', position it along the tangent at minleader.
    // Otherwise, position it along the dim arc.
    if (bFits)
        {
        DVec3d  textOriginDirection;
        angle_to_direction (&textOriginDirection, textOriginDist / dDimLineRadius);

        // Transform text origin direction to dimension coordinates
        pTrans->Multiply(textOriginDirection);
        bsiDPoint3d_addScaledDVec3d (pTextOrg, pCenter, &textOriginDirection, (dDimLineRadius + offset.y));

        *pNormalPoint = *pTextOrg;
        }
    else
        {
        bsiDPoint3d_addScaledDVec3d (pTextOrg, &minLeaderPoint, pTextDirectionOut, - reversalScalar * noFitShift
                             - BentleyApi::adim_computeLeftEdgeOffset (ep, 0.0, pTextSize->x));
        bsiDPoint3d_addScaledDVec3d (pTextOrg, pTextOrg, &textCenterDirection, offset.y);

        if (isAutoBnc)
            {
            double  bottomShift = pTextSize->y / 2 + ep->GetDimElementCP()->geom.textLift;

            bsiDPoint3d_addScaledDVec3d (pNormalPoint, &minLeaderPoint, &textCenterDirection, offset.y - bottomShift);
            }
        else
            {
            *pNormalPoint = minLeaderPoint;
            }
        }

    // text inside fit option proceedes leadered option by default
    if (DIMSTYLE_VALUE_Text_Location_Inline != textLocation && DIMSTYLE_VALUE_FitOption_KeepTextInside == ep->flags.fitOption)
        {
        DPoint3d    textMid;

        getArcMidPoint (&textMid, pCenter, dArcLength/dDimLineRadius, dDimLineRadius, pTrans);
        bsiDPoint3d_addScaledDVec3d (pTextOrg, &textMid, pTextDirectionOut, -pTextSize->x / 2.0);

        ((DVec3d *) pNormalPoint)->NormalizedDifference (textMid, *pCenter);
        }
    else if (leadered && !isAutoBnc)
        {
        // For the leadered case, pTextOrg is currently at the elbow start point.
        // Move origin to the left-center point (ie. by the distance of textMargin)
        if (! multiline)
            bsiDPoint3d_addScaledDVec3d (pTextOrg, pTextOrg, pTextDirectionOut, textMargin);
        else
            {
            switch  (adim_getDimTextJustificationHorizontal (ep))
                {
                case TextElementJustification::RightMiddle:
                    bsiDPoint3d_addScaledDVec3d (pTextOrg, pTextOrg, pTextDirectionOut, -textMargin);
                    break;

                case TextElementJustification::LeftMiddle:
                    bsiDPoint3d_addScaledDVec3d (pTextOrg, pTextOrg, pTextDirectionOut, textMargin);
                    break;
                }
            }
        }

    *pdTextSw  = reversalScalar * (pTextSize->x + 2.0 * textMargin) / dDimLineRadius;
    *pdTextOff = (linear_off - reversalScalar * textMargin) / dDimLineRadius;

    ep->stackHeight = getAngularStackOffset (ep->GetDimElementCP(), pCheck, pTextSize, dCharWidth, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/06
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    isTextLiftedAtMinLeaderPoint
(
DimStyleProp_Text_Location  textLocation,
AdimProcess const           *pAdimProcess
)
    {
    if (!pAdimProcess->GetDimElementCP()->flag.horizontal && pAdimProcess->flags.textNotFit && pAdimProcess->flags.pushTextOutside)
        {
        // use min leader point for shifting for auto-minleader or underline extension
        if (!pAdimProcess->flags.ignoreMinLeader && DIMSTYLE_VALUE_Text_Location_Inline != textLocation)
            {
            bool        extendDimline = false;

            mdlDim_extensionsGetExtendDimLineUnderText (&extendDimline, pAdimProcess->pOverrides, false);

            return  extendDimline;
            }

        return  true;
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* Compute additional upward shift required for non-inline text
* @bsimethod                                                    petri.niiranen  03/02
+---------------+---------------+---------------+---------------+---------------+------*/
static void    computeTextShift
(
AdimProcess*              ep,
DPoint3d*                 pTextOrg,
DVec3d*                   pTextDirection,
DVec3d       const*       pNormalPoint,
DPoint3d     const* const pCenter,
double       const        dDimLineRadius,
DPoint2dCP                pTextSize,
DPoint2dCP                pTextTileSize
)
    {
    DimStyleProp_Text_Location textLocation = DIMSTYLE_VALUE_Text_Location_Inline;
    double      lift;
    double      dShift = 0.0;
    DPoint3d    rtxtmid, onArc;
    DVec3d      normal, rotatedNormal;

    mdlDim_overridesGetTextLocation (&textLocation, ep->pOverrides, ep->flags.embed);

    if (isTextLiftedAtMinLeaderPoint(textLocation, ep))
        {
        normal.NormalizedDifference (*pNormalPoint, *pCenter);
        bsiDPoint3d_addScaledDVec3d  (&onArc, pCenter, &normal, dDimLineRadius);

        lift = ep->GetDimElementCP()->geom.textMargin;
        }
    else if (DIMSTYLE_VALUE_FitOption_KeepTextInside == ep->flags.fitOption)
        {
        normal = *pNormalPoint;
        lift = 0.0;
        }
    else
        {
        switch (adim_getDimTextJustificationHorizontal(ep))
            {
            case TextElementJustification::CenterMiddle:
                dShift = 0.0;
                break;
            case TextElementJustification::RightMiddle:
                dShift = -pTextSize->x / 2.0;
                break;
            default:
                dShift = pTextSize->x / 2.0;
            }

        bsiDPoint3d_addScaledDVec3d (&rtxtmid, pTextOrg, pTextDirection, dShift);

        normal.NormalizedDifference (rtxtmid, *pCenter);
        bsiDPoint3d_addScaledDVec3d  (&onArc, pCenter, &normal, dDimLineRadius);

        lift = bsiDPoint3d_distance (&onArc, &rtxtmid);
        }

    rotatedNormal = normal;
    ep->vuMatrix.Multiply(rotatedNormal);

    /*---------------------------------------------------------------------------
        Linear dimensions use the factor in ep->dim->geom.textLift to control 
        the text offset above/below the dimension line.  For some legacy reason
        angular dimensions always use tileSize.h/2
    ---------------------------------------------------------------------------*/
    double hardCodedLift = pTextTileSize->y / 2.0;

    if (DIMSTYLE_VALUE_Text_Location_Above == textLocation)
        {
        // above shift is applicable only if text is aligned or
        // if text is horizontal and normal is vertical
        if (!ep->GetDimElementCP()->flag.horizontal || fabs (fabs (rotatedNormal.y) - 1.0) < mgds_fc_epsilon)
            {
            dShift = hardCodedLift + pTextSize->y / 2.0 + lift;
            adim_offsetText (pTextOrg, pTextOrg, pTextDirection, dShift, ep);
            }
        }
    else if (DIMSTYLE_VALUE_Text_Location_TopLeft == textLocation)
        {
        if (!ep->GetDimElementCP()->flag.horizontal)
            {
            dShift = hardCodedLift + pTextSize->y / 2.0 + lift;

            // If the normal is in the 3rd or 4th quadrants, shift text above dimarc
            dShift *= rotatedNormal.y < mgds_fc_epsilon ? -1.0 : 1.0;
            bsiDPoint3d_addScaledDVec3d (pTextOrg, pTextOrg, &normal, dShift);
            }
        else
            {
            dShift  = fabs (rotatedNormal.x) * (pTextSize->x / 2.0 + ep->GetDimElementCP()->geom.textMargin) +
                      fabs (rotatedNormal.y) * (pTextSize->y / 2.0 + ep->GetDimElementCP()->geom.textLift);

            // If the normal is in the 3rd or 4th quadrants, shift text above dimarc
            dShift *= rotatedNormal.y < mgds_fc_epsilon ? -1.0 : 1.0;
            bsiDPoint3d_addScaledDVec3d (pTextOrg, pTextOrg, &normal, dShift);
            }
        }
    else
        {
        if ( ! ep->GetDimElementCP()->flag.horizontal)
            dShift = hardCodedLift + pTextSize->y / 2.0 + lift;
        else
            dShift = fabs (rotatedNormal.x * (pTextSize->x / 2.0 + ep->GetDimElementCP()->geom.textMargin)) +
                     fabs (rotatedNormal.y * (pTextSize->y / 2.0 + ep->GetDimElementCP()->geom.textLift));

        bsiDPoint3d_addScaledDVec3d (pTextOrg, pTextOrg, &normal, dShift);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     1/90
+---------------+---------------+---------------+---------------+---------------+------*/
static void    getRadialExtensionLine
(
DSegment3dP witline,
DPoint3d        *cntr,       /* => Witness line start point            */
DPoint3d        *end,        /* => Witness line end point              */
double          rad,         /* => dimension arc radius                */
AdimProcess const *ep          /* => Function used to process elements   */
)
    {
    DVec3d      dir;
    double      offset, extend, rad2;

    offset = ep->GetDimElementCP()->geom.witOffset;
    extend = ep->GetDimElementCP()->geom.witExtend;

    rad2 = bsiDPoint3d_distance (cntr, end);
    dir.NormalizedDifference (*end, *cntr);

    if (rad2 > rad)
        {
        offset *= -1.0;
        extend *= -1.0;
        }

    mdlDim_overridesGetPointWitnessOffset (&offset, ep->pOverrides, ADIM_GETSEG(ep->partName), offset);
    mdlDim_overridesGetPointWitnessExtend (&extend, ep->pOverrides, ADIM_GETSEG(ep->partName), extend);

    if (ep->GetDimElementCP()->geom.witOffset < 0.0)
        {
        bsiDPoint3d_addScaledDVec3d (&witline->point[0], cntr, &dir, rad + offset);
        }
    else
        {
        if ( ! adim_useWitnessLineOffset (ep))
            offset = 0.0;

        bsiDPoint3d_addScaledDVec3d (&witline->point[0], cntr, &dir, rad2 + offset);
        }

    bsiDPoint3d_addScaledDVec3d (&witline->point[1], cntr, &dir, rad + extend);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/06
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    isDimlineExtendedUnderText
(
DimStyleProp_Text_Location  textLocation,
AdimProcess const           *pAdimProcess
)
    {
    bool        bExtUnderline = false;

    if (pAdimProcess->flags.pushTextOutside && DIMSTYLE_VALUE_Text_Location_Inline != textLocation && !pAdimProcess->GetDimElementCP()->flag.horizontal)
        mdlDim_extensionsGetExtendDimLineUnderText (&bExtUnderline, pAdimProcess->pOverrides, false);

    return  bExtUnderline;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/05
+---------------+---------------+---------------+---------------+---------------+------*/
static void    getCenteredTextDirection
(
DVec3d              *pTextDir,
DPoint3d            *pTextInclined,
DVec3d             *pCurrentTangent,
double              currentSwept,
RotMatrix const     *pTrans,
AdimProcess const   *pAdimProcess
)
    {
    getCheckFromAngle (pCurrentTangent, 0.5 * currentSwept, pTrans, pAdimProcess);

    /* find the text bvector inclined from tangent direction */
    if (pAdimProcess->GetDimElementCP()->flag.horizontal)
        {
        pTextDir->x = 1.0;
        pTextDir->y = 0.0;
        pTextDir->z = 0.0;

        /*-------------------------------------------------------------------------------
        text direction is used to determin auto ball & chain's inline leader so flip
        direction for text in 2nd and 3rd quadrants.
        -------------------------------------------------------------------------------*/
        if (pCurrentTangent->y < -mgds_fc_epsilon)
            bsiDVec3d_negate (pTextDir, pTextDir);

        *pTextInclined = *pCurrentTangent;
        }
    else
        {
        *pTextDir = *pCurrentTangent;

        pTextInclined->x = 1.0;
        pTextInclined->y = 0.0;
        pTextInclined->z = 0.0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/06
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    isDimensionArcIntersectedByText
(
DVec3d const      *pTangent,
AdimProcess const   *pAdimProcess
)
    {
    DimStyleProp_Text_Location  textLocation = DIMSTYLE_VALUE_Text_Location_Inline;

    mdlDim_overridesGetTextLocation (&textLocation, pAdimProcess->pOverrides, pAdimProcess->flags.embed);

    // all dimensions with inline texts intersect dim arcs
    if (DIMSTYLE_VALUE_Text_Location_Inline != textLocation)
        {
        /*-------------------------------------------------------------------------------
        When text is not inline, these are the conditions under which the text does not
        intersect the dimension arc:

        1) Aligned text has above, outside, or top left orientation, or
        2) Horizontal text has outside or top left orientation, or
        3) Horizontal text has above orientation and the arc center tangent is "perfectly"
           horizontal.  Check the cosine value (i.e. x instead of y) to match the same
           criteria in computeTextShift.
        -------------------------------------------------------------------------------*/
        if (!pAdimProcess->GetDimElementCP()->flag.horizontal ||
            DIMSTYLE_VALUE_Text_Location_Above != textLocation ||
            fabs(fabs(pTangent->x) - 1.0) < mgds_fc_epsilon)
            return  false;
        }

    // all other cases cause text to intersect dim arc.
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/06
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    isTextAwayFromDimArc
(
DVec3d const      *pTangent,
AdimProcess const   *pAdimProcess
)
    {
    if (0 == pAdimProcess->GetDimElementCP()->flag.termMode)
        {
        bool        tightFitTextAbove = false;
        mdlDim_extensionsGetTightFitTextAbove (&tightFitTextAbove, pAdimProcess->pOverrides, false);

        if (tightFitTextAbove && !isDimensionArcIntersectedByText(pTangent, pAdimProcess))
            return  true;
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/06
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    isVectorNearlyOrthogonal
(
DVec3d const      *pTangent
)
    {
    // use 5 degrees as a criteria:
    static double  sin5degrees = 0.0871557;

    return  fabs(pTangent->x) < sin5degrees || fabs(pTangent->y) < sin5degrees;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/06
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    isInclinedAngdimWithHorizontalText
(
DPoint2dCP          pTextSize,
DVec3dCP            pTangent,
AdimProcess const*  pAdimProcess
)
    {
    // only dimensions with horizontal texts can be "inclined".
    if (pAdimProcess->GetDimElementCP()->flag.horizontal)
        {
        if (isVectorNearlyOrthogonal(pTangent))
            {
            /*---------------------------------------------------------------------------
            Horizontal and vertical dimensions are generally not inclined, as long as
            the dimension arcs intersect left & right sides of the texts.  When dimension
            arcs intersect bottom of the texts, they should be treated as inclined.
            ---------------------------------------------------------------------------*/
            if (fabs(BentleyApi::adim_getIntersectLength(pTangent, pTextSize, 0.0) - pTextSize->x) < mgds_fc_epsilon)
                return  false;
            }

        return  true;
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/06
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    isMinLeaderFit
(
double              *pEffectiveWidth,
DPoint2dCP          pTextSize,
DVec3dCP            pInclinedDir,
double              sweptAngle,
double              radius,
double              dimlineLength,
double              minLeader,
AdimProcess const   *pAdimProcess
)
    {
    bool        bFit = false;

    *pEffectiveWidth = BentleyApi::adim_getIntersectLength (pInclinedDir, pTextSize, 2 * pAdimProcess->GetDimElementCP()->geom.textMargin);

    // effective width calculated from above has included text margin, so exclude it from min leader:
    minLeader -= pAdimProcess->GetDimElementCP()->geom.textMargin;
    if (minLeader < 0.0)
        minLeader = 0.0;

    // first check for spaces to be enough to hold terminators+insideMinLeaders
    bFit = *pEffectiveWidth + 2 * minLeader < dimlineLength;

    return  bFit;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/06
+---------------+---------------+---------------+---------------+---------------+------*/
static double      getAcadFitMargin
(
double              *pChordMargin,
AdimProcess const   *pAdimProcess
)
    {
    double  fitMargin = 0.8 * pAdimProcess->GetDimElementCP()->geom.textMargin;

    *pChordMargin = pAdimProcess->GetDimElementCP()->geom.termWidth + 0.4 * pAdimProcess->GetDimElementCP()->geom.textMargin;

    return  fitMargin;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void    checkFittingBasedOnTextJustification
(
AdimProcess     *pAdimProcess,
double          dimlineLength,
double          textWidth,
double          textMargin,
double          charWidth,
DimText const   *pDimText
)
    {
    double      checkWidth = textWidth + 2.0 * pAdimProcess->GetDimElementCP()->geom.textMargin;

    /*-----------------------------------------------------------------------------------
    For left/right justified text, count in the 3*char fixed length for fitting.
    When text is horizontal though, the outcome looks better by ignoring left/right
    justification (i.e. a tighter fit moves text out before it is near wit lines).
    -----------------------------------------------------------------------------------*/
    if (DIMTEXT_CENTER == pDimText->flags.b.just || pAdimProcess->GetDimElementCP()->flag.horizontal)
        pAdimProcess->flags.textNotFit = dimlineLength < checkWidth;
    else
        pAdimProcess->flags.textNotFit = dimlineLength < checkWidth + 3 * charWidth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/06
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    checkFitting
(
AdimProcess*    pAdimProcess,
DPoint2dP       pTextSize,
double          charWidth,
DVec3d          *pInclinedDir,
double          sweptAngle,
double          radius,         /* radius at dimension line */
double          dimrad,         /* radius for length calculation */
double          minLeader,
double          extraMargin,
DimText const   *pDimText
)
    {
    bool        bFit = true;
    bool        bFitProjectedTextBox = false;
    double      chordLength = 0.0, chordMargin = 0.0, fitMargin = 0.0;
    double      dimlineLength, effectiveWidth;

    // do not use projected text width unless it is a horizontal text in an inclined dimension
    pAdimProcess->dProjectedTextWidth = -1.0;
    pAdimProcess->flags.pushTextOutside = false;
    pAdimProcess->flags.textNotFit = false;

    pAdimProcess->flags.tightFitTextAbove = isTextAwayFromDimArc(pInclinedDir, pAdimProcess);

    mdlDim_extensionsGetFitInclinedTextBox (&bFitProjectedTextBox, pAdimProcess->pOverrides, false);

    // keep legacy dim length being arc length and effective text width being actual width
    dimlineLength = radius * sweptAngle;
    effectiveWidth = pTextSize->x;

    // preserve the behavior for fixed terminator modes (i.e. inside, outside, and reversed):
    if (0 != pAdimProcess->GetDimElementCP()->flag.termMode)
        {
        checkFittingBasedOnTextJustification (pAdimProcess, dimlineLength, effectiveWidth, pAdimProcess->GetDimElementCP()->geom.textMargin, charWidth, pDimText);
        return  bFit;
        }

    chordLength = 2 * radius;
    if (sweptAngle < msGeomConst_pi)
        chordLength *= sin(sweptAngle / 2);

    if (pAdimProcess->flags.tightFitTextAbove)
        {
        /* text (above or aside) does not cross dimension line: fit text between terms: */
        double  termsSize = 2 * pAdimProcess->GetDimElementCP()->geom.termWidth;

        if (pAdimProcess->GetDimElementCP()->flag.horizontal)
            {
            effectiveWidth = BentleyApi::adim_projectTextSize(pTextSize, pInclinedDir, pAdimProcess->GetDimElementCP()->geom.textMargin);

            pAdimProcess->dProjectedTextWidth = effectiveWidth;

            dimlineLength = chordLength;
            }
        else
            {
            effectiveWidth += 2 * pAdimProcess->GetDimElementCP()->geom.textMargin;
            }

        bFit = effectiveWidth + termsSize < dimlineLength;
        }
    else if (bFitProjectedTextBox && isInclinedAngdimWithHorizontalText(pTextSize, pInclinedDir, pAdimProcess))
        {
        /*-----------------------------------------------------------------------------------
        Check both the minleader and text box fitting:
        Imagine a pair of linear witness lines going through the tip points of terminators.
        The effective (i.e projected) width of the text box should be within a margin distance
        before touching the imaginary witness lines.  This chord margin is close enough to
        what ACAD's angular dimension behaves although not as accurate as we'd like to have.
        -----------------------------------------------------------------------------------*/
        fitMargin = getAcadFitMargin (&chordMargin, pAdimProcess);

        if (sweptAngle >= msGeomConst_pi)
            chordMargin = 0.0;

        bFit = isMinLeaderFit (&effectiveWidth, pTextSize, pInclinedDir, sweptAngle, radius, dimlineLength, minLeader, pAdimProcess);

        /*---------------------------------------------------------------------------
        Second, do a collision check for text box against imaginary witness lines.
        That is, treat angular dimension as a linear dimension and check the space
        between text box and imaginary witness lines.  This seemingly nonsense logic
        is the best match we can find for DWG compatibility.
        ---------------------------------------------------------------------------*/
        effectiveWidth = BentleyApi::adim_projectTextSize(pTextSize, pInclinedDir, pAdimProcess->GetDimElementCP()->geom.textMargin);

        pAdimProcess->dProjectedTextWidth = effectiveWidth;

        if (!pAdimProcess->flags.allDontFit)
            {
            bFit = effectiveWidth + 2 * chordMargin < chordLength;

            dimlineLength = chordLength;
            }
        }
    else if (DIMSTYLE_VALUE_FitOption_MoveTermsFirst != pAdimProcess->flags.fitOption)
        {
        // the rest cases should be only horizontal, vertical and aligned angular dimensions
        fitMargin = getAcadFitMargin (&chordMargin, pAdimProcess);

        dimlineLength = chordLength;

        bFit = isMinLeaderFit (&effectiveWidth, pTextSize, pInclinedDir, sweptAngle, radius, dimlineLength, minLeader, pAdimProcess);
        }

    pAdimProcess->flags.allDontFit = !bFit;

    if (pAdimProcess->flags.allDontFit)
        {
        // only bare terminator widths are used for secondary check (ie terminator fitting)
        if (pAdimProcess->flags.ignoreMinLeader)
            minLeader = pAdimProcess->GetDimElementCP()->geom.termWidth;
        
        DPoint2d offset;
        offset.x = pDimText->offset;
        offset.y = pDimText->offsetY;
        pAdimProcess->flags.fitTermsInside = adim_areTerminatorsBetweenExtensionLines (pAdimProcess, effectiveWidth, fitMargin, chordLength, minLeader, offset, pDimText->flags.b.just);
        }

    /* now check for text alone fitting - part of legacy fitting */
    if (DIMSTYLE_VALUE_FitOption_KeepTextInside != pAdimProcess->flags.fitOption || DIMTEXT_CENTER != pDimText->flags.b.just)
        {
        checkFittingBasedOnTextJustification (pAdimProcess, dimlineLength, effectiveWidth, extraMargin, charWidth, pDimText);
        }

    return  bFit;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   generateAlignedAutoBallAndChain
(
DPoint3d            *pStart,
DPoint3d            *pEnd,
AdimProcess         *pAdimProcess
)
    {
    StatusInt       status = SUCCESS;
    ElementHandleCR dimElement =  pAdimProcess->GetElemHandleCR();

    DimOffsetBlock  *pOffsetBlock = (DimOffsetBlock *) mdlDim_getOptionBlock (dimElement, ADBLK_EXTOFFSET, NULL);

    if (NULL != pOffsetBlock)
        {
        ADIM_SETNAME (pAdimProcess->partName, ADTYPE_CHAIN, ADSUB_NONE);

        status = adim_generateDimLine (pAdimProcess, pStart, pEnd, DIM_MATERIAL_DimLine, pOffsetBlock->flags.terminator, 0, TRIM_LEFT, true, true, true);

        if (SUCCESS == status)
            {
            /* don't revert terminator for ball & chain */
            status = adim_generateLineTerminator (pAdimProcess, pStart, pEnd, pOffsetBlock->flags.terminator, true);
            }
        }

    return  status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          generate_angular_text                                   |
|                                                                       |
| author        JVB                                     12/86           |
|                                                                       |
+----------------------------------------------------------------------*/
static int generateAngularText
(
AdimProcess  *ep,
bool         *pLeadered,
DPoint3d     *pTextDir,
DimText const *dimText,
double       *textoff,
double       *textsw,
double       *charwdth,
double       rad,               /* radius at dimension line */
double       dimrad,            /* radius for length calculation */
DPoint3d     *cntr,
DPoint3d     *pt1,              /* 1st witness line start point */
DPoint3d     *pt2,              /* 2nd witness line start point */
double       sw,
RotMatrix    *trans,
double       sweepValue
)
    {
    DimStyleProp_Text_Location   textLocation = DIMSTYLE_VALUE_Text_Location_Inline;

    DPoint2d      tile;
    DPoint2d      textSize;
    double        arclen, linear_off, center_angle;
    int           status;
    double        extraMargin, insideMinLeader, outsideMinLeader;
    DVec3d        rtxtorg, tangent, check, textDir, textInclined, normPoint;
    bool          showTolerance = false;

    *pLeadered = false;
    showTolerance = getShowTolerance (ep);
    arclen = sw * rad;

    ElementHandleCR dimElement =  ep->GetElemHandleCR();
    DimensionElm const* dim = &dimElement.GetElementCP()->ToDimensionElm();

    /*-----------------------------------------------------------------------------------
        text size
    -----------------------------------------------------------------------------------*/
    adim_getTileSize (&tile, NULL, dimElement);
    *charwdth  = tile.y;
    extraMargin = adim_needExtraTextMargin(ep) ? dim->geom.textMargin : 0.0;

    /* get effective inside minimum leader */
    BentleyApi::adim_getEffectiveMinLeaders (&insideMinLeader, NULL, ep);
    ep->flags.ignoreMinLeader = mdlDim_isMinLeaderIgnored (ep->pOverrides, false);

    /*-----------------------------------------------------------------------------------
        text strings
    -----------------------------------------------------------------------------------*/
    getMeasureStrings (ep, &textSize, &tile, charwdth, sweepValue, dimrad, showTolerance);

    /*-----------------------------------------------------------------------------------
        text & terminators fitting
    -----------------------------------------------------------------------------------*/
    getCenteredTextDirection (&textDir, &textInclined, &tangent, sw, trans, ep);
    checkFitting (ep, &textSize, *charwdth, &textInclined, sw, rad, dimrad, insideMinLeader, extraMargin, dimText);
    /* get effective outside minimum leader which depends on terminator being inside vs outside */
    BentleyApi::adim_getEffectiveMinLeaders (NULL, &outsideMinLeader, ep);

    /*-----------------------------------------------------------------------------------
        text direction
    -----------------------------------------------------------------------------------*/
    DPoint2d offset;
    offset.x= dimText->offset;
    offset.y= dimText->offsetY;

    int textJust = dimText->flags.b.just;
    bool pushRight = dimText->flags.b.pushTextRight;
    BentleyApi::adim_calcTextOffset (offset, textJust, pushRight, arclen, textSize.x, *charwdth, outsideMinLeader + extraMargin, 0, &textSize, &textDir, &tangent, ep);

    linear_off = offset.x;

    if (offset.x < -textSize.x || offset.x> arclen)
        BentleyApi::adim_setCurrentSegmentTextIsOutside (ep, textJust);

    *pLeadered = BentleyApi::adim_checkForLeader (NULL, ep, offset.y, &textSize);
    if (*pLeadered)
        {
        center_angle = sweepValue / 2.0;
        getCheckFromAngle (&check, center_angle, trans, ep);
        }
    else
        {
        center_angle = (linear_off + textSize.x / 2.0) / rad;
        getCheckFromAngle (&check, center_angle, trans, ep);
        offset.y = 0.0;
        }

    /*-----------------------------------------------------------------------------------
        text position
    -----------------------------------------------------------------------------------*/
    if (dim->flag.horizontal)
        {
        getHorizontalTextPosition (ep, &rtxtorg, &tangent, &normPoint, textsw, textoff, cntr, &check, pt1, trans, rad, arclen, &textSize,
                                   *charwdth, outsideMinLeader, *pLeadered, textJust, pushRight, offset);
        }
    else
        {
        getAlignedTextPosition    (ep, &rtxtorg, &tangent, &normPoint, textsw, textoff, cntr, &check, trans, rad, arclen, &textSize,
                                   *charwdth, outsideMinLeader, *pLeadered, textJust, offset);
        }

    mdlDim_overridesGetTextLocation (&textLocation, ep->pOverrides, ep->flags.embed);
    if (DIMSTYLE_VALUE_Text_Location_Inline != textLocation && ! *pLeadered)
        {
        computeTextShift (ep, &rtxtorg, &tangent, &normPoint, cntr, rad, &textSize, &tile);
        }

    // At this point, the correct text origin has been computed for the non-leadered case.
    // For the leadered case, the text origin corresponds to the text being on the right
    // hand side of the elbow. The function adim_generateBallAndChain() will flip the
    // text origin to the left hand side if necessary.

    /*-----------------------------------------------------------------------------------
        Text generation
    -----------------------------------------------------------------------------------*/
     if (dim->frmt.angleMeasure && showTolerance == false)
        {
        adim_rotateText (&tangent, &rtxtorg, &textSize, ep);

        /*-------------------------------------------------------------------------------
            measure angle
        -------------------------------------------------------------------------------*/
        if (*pLeadered)
            {
            DPoint3d    pointOnArc;

            // Use arc mid point as leader start point.
            getArcMidPoint (&pointOnArc, cntr, sweepValue, rad, trans);
            if (!dim->flag.horizontal && isAutoBallAndChainInEffect(ep, textJust))
                generateAlignedAutoBallAndChain (&normPoint, &pointOnArc, ep);
            else
                adim_generateBallAndChain (&rtxtorg, ep, &rtxtorg, &pointOnArc, &tangent, &textSize, offset.y);
            }

        ADIM_SETNAME (ep->partName, ADTYPE_TEXT_SINGLE, ADSUB_NONE);

        if (SUCCESS != (status = generateAngleText (ep, &rtxtorg, &tangent)))
            {
            return  status;
            }

        if (SUCCESS != (status = adim_generateTextSymbols (ep, *charwdth, &textSize, &rtxtorg, &tangent)))
            {
            return  status;
            }

        adim_harvestTextBoxForDerivedData (ep);
        }
    else
        {
        DPoint3d    pointOnArc;

        /*-------------------------------------------------------------------------------
            measure distance
        -------------------------------------------------------------------------------*/
        getArcMidPoint  (&pointOnArc, cntr, sweepValue, rad, trans);

        /* We always pass the 'pointOnArc' into the generateLinear call.  In the ball and chain case, it
           is used as the anchor point for the chain.  In the other case, it is used as a flag that the
           text is already properly positioned */
        if (SUCCESS != (status = adim_generateLinearDimensionWithOffset (ep, &pointOnArc, &rtxtorg, &tangent, dimText->offsetY)))
            {
            return  status;
            }
        }

    if (dim->tmpl.above_symbol)
        ep->stackHeight += *charwdth * 2.0;

    ep->dProjectedTextWidth = textSize.x;

    *pTextDir = tangent;

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/06
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    isDimensionArcSplit
(
double              sweptAngle,
RotMatrix const     *pTrans,
AdimProcess const   *pAdimProcess
)
    {
    DVec3d        tangent;

    getCheckFromAngle (&tangent, sweptAngle / 2.0, pTrans, pAdimProcess);

    return isDimensionArcIntersectedByText (&tangent, pAdimProcess);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     1/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int     generateArcDimensionLine
(
AdimProcess     *ep,
DimText const   *dimText,
DPoint3d        *pCenter,           /* => center point                 */
DVec3d          *pTextDir,
double          rad,                /* => radius at dimension line     */
double          dimSweep,           /* => sweep angle of dimension     */
double          textSweep,          /* => arc sweep covered by text    */
double          textOffset,
RotMatrix       *trans,             /* => transform of dimension       */
int             firstSeg,
bool            offsetAllowed       /* => ball and chain offset is present */
)
    {
    RotMatrix   termTrans;
    double      startAngle, endAngle;
    int         terminator_clockwise, status, leftIndex;
    bool        suppressTermsOutside = false;
    double      outsideMinLeader = 0.0, minAngle = 0.0;
    DimStyleProp_Text_Location   textLocation = DIMSTYLE_VALUE_Text_Location_Inline;
    mdlDim_overridesGetTextLocation (&textLocation, ep->pOverrides, ep->flags.embed);
    
    BentleyApi::adim_getEffectiveMinLeaders (NULL, &outsideMinLeader, ep);
    minAngle = fabs(outsideMinLeader) / rad;
    if (ep->flags.ignoreMinLeader && fabs(minAngle) > msGeomConst_pi)
        minAngle = minAngle < 0.0 ? -msGeomConst_pi : msGeomConst_pi;

    memcpy (&termTrans, trans, sizeof(termTrans));

    if (!offsetAllowed &&
        ((ep->flags.allDontFit && !ep->flags.fitTermsInside) ||
          fabs (textSweep) + ep->GetDimElementCP()->geom.margin / rad > dimSweep ||
          textOffset < 0.0 || textOffset > dimSweep))
        {
        bool        extendUnderline = isDimlineExtendedUnderText(textLocation, ep);

        /*---------------------------------------------------------------------------
        Suppress terminators & dim lines if they flip outside due to fitting options
        yet text still stays inside.
        ---------------------------------------------------------------------------*/
        if (DIMTEXT_OFFSET != dimText->flags.b.just && ep->flags.textNotFit)
            mdlDim_extensionsGetSuppressUnfitTerminatorsFlag (&suppressTermsOutside, ep->pOverrides, false);

        // revert terminator orientation when pushed by fit options
        terminator_clockwise = ep->flags.allDontFit && ep->flags.fitTermsInside;

        if (ep->GetDimElementCP()->flag.joiner || ep->flags.fitTermsInside)
            {
            ADIM_SETNAME (ep->partName, ADTYPE_DIMLINE, ADSUB_NONE);

            startAngle = DIMTEXT_END == dimText->flags.b.just ? 0.0 : -minAngle;
            endAngle = terminator_clockwise ? dimSweep + minAngle : 2.0 * minAngle + dimSweep;

            if (status = adim_generateArc (ep, pCenter, rad, rad, trans, startAngle, endAngle))
                return (status);

            suppressTermsOutside = false;
            }
        else
            {
            /* text doesn't fit */
            bool    suppressLeftLeader = suppressTermsOutside && (textOffset > dimSweep);
            bool    suppressRightLeader = suppressTermsOutside && (textOffset < 0.0);

            startAngle = -minAngle;
            endAngle = minAngle;

            ADIM_SETNAME (ep->partName, ADTYPE_DIMLINE, ADSUB_NONE);

            if (!suppressLeftLeader && !mdlDim_overridesGetSegmentFlagSuppressLeftDimLine (NULL, ep->pOverrides, ADIM_GETSEG(ep->partName), false))
                if (status = adim_generateArc (ep, pCenter, rad, rad, trans, startAngle, endAngle))
                    return (status);

            startAngle = dimSweep;

            if (!suppressRightLeader && !mdlDim_overridesGetSegmentFlagSuppressRightDimLine (NULL, ep->pOverrides, ADIM_GETSEG(ep->partName), false))
                if (status = adim_generateArc (ep, pCenter, rad, rad, trans, startAngle, endAngle))
                    return (status);
            }

        /* extend extention arc to underline of text */
        if (extendUnderline)
            {
            DPoint3d    point1, point2;
            double      underlineLength = ep->dProjectedTextWidth + 2 * ep->GetDimElementCP()->geom.textMargin;

            //startAngle = textOffset < dimSweep ? -minAngle : dimSweep + minAngle;
            startAngle = -minAngle;

            /* get normal bvector to project radius to leader point */
            point1.x = rad * cos (startAngle);
            point1.y = rad * sin (startAngle);
            point1.z = 0.0;
            trans->Multiply(point1);
            bsiDPoint3d_addDPoint3dInPlace (&point1, pCenter);

            /* find underline end point */
            if (startAngle * textSweep < 0.0)
                underlineLength = -underlineLength;
            bsiDPoint3d_addScaledDVec3d (&point2, &point1, pTextDir, underlineLength);

            if (SUCCESS != (status = adim_generateLine(ep, &point1, &point2, DIM_MATERIAL_DimLine)))
                return  status;
            }
        }
    else
        {
        // revert terminator orientation when pushed by fit options
        terminator_clockwise = !ep->flags.allDontFit || ep->flags.fitTermsInside;

        if ((DIMSTYLE_VALUE_Text_Location_Inline == textLocation ||
            isDimensionArcSplit(dimSweep, trans, ep)) &&
            !offsetAllowed)
            {
            /* split the arc if textlocation is in-line or if it is above and text alignment is horizontal */

            if (DIMTEXT_OFFSET != dimText->flags.b.just && ep->flags.allDontFit && !ep->flags.fitTermsInside)
                {
                mdlDim_extensionsGetSuppressUnfitTerminatorsFlag (&suppressTermsOutside, ep->pOverrides, false);
                if (suppressTermsOutside)
                    return  SUCCESS;
                }

            if (textSweep > 0.0)
                {
                ADIM_SETNAME (ep->partName, ADTYPE_DIMLINE, ADSUB_NONE);

                startAngle = 0.0;
                endAngle = terminator_clockwise ? textOffset : -minAngle;

                if (!mdlDim_overridesGetSegmentFlagSuppressLeftDimLine (NULL, ep->pOverrides, ADIM_GETSEG(ep->partName), false))
                    if (status = adim_generateArc (ep, pCenter, rad, rad, trans, startAngle, endAngle))
                        return (status);

                startAngle = terminator_clockwise ? textOffset + textSweep : dimSweep;
                endAngle = terminator_clockwise ? dimSweep - (textOffset + textSweep) : minAngle;

                if (!mdlDim_overridesGetSegmentFlagSuppressRightDimLine (NULL, ep->pOverrides, ADIM_GETSEG(ep->partName), false))
                    if (status = adim_generateArc (ep, pCenter, rad, rad, trans, startAngle, endAngle))
                        return (status);
                }
            else
                {
                ADIM_SETNAME (ep->partName, ADTYPE_DIMLINE, ADSUB_NONE);

                startAngle = 0.0;
                endAngle = terminator_clockwise ? textOffset + textSweep : -minAngle;

                if (!mdlDim_overridesGetSegmentFlagSuppressLeftDimLine (NULL, ep->pOverrides, ADIM_GETSEG(ep->partName), false))
                    if (status = adim_generateArc (ep, pCenter, rad, rad, trans, startAngle, endAngle))
                        return (status);

                startAngle = terminator_clockwise ? textOffset : dimSweep;
                endAngle = terminator_clockwise ? dimSweep - textOffset : minAngle;

                if (!mdlDim_overridesGetSegmentFlagSuppressRightDimLine (NULL, ep->pOverrides, ADIM_GETSEG(ep->partName), false))
                    if (status = adim_generateArc (ep, pCenter, rad, rad, trans, startAngle, endAngle))
                        return (status);
                }
            }
        else
            {
            /* textlocation is above or outside or top-left and text fits */
            terminator_clockwise = true;

            ADIM_SETNAME (ep->partName, ADTYPE_DIMLINE, ADSUB_NONE);

            if (status = adim_generateArc (ep, pCenter, rad, rad, trans, 0.0, dimSweep))
                return (status);
            }
        }

    if (suppressTermsOutside)
        return  SUCCESS;

    if (firstSeg && ep->GetDimElementCP()->version >= 7 && ep->GetDimElementCP()->tmpl.stacked == 0 &&
       (ep->GetDimElementCP()->dimcmd == static_cast<byte>(DimensionType::AngleLocation)  || ep->GetDimElementCP()->dimcmd == static_cast<byte>(DimensionType::ArcLocation)))
        {
        leftIndex = ep->GetDimElementCP()->tmpl.first_term;
        }
    else
        {
        leftIndex = ep->GetDimElementCP()->tmpl.left_term;
        }

    ADIM_SETNAME (ep->partName, ADTYPE_TERM_LEFT, ADSUB_NONE);
    if (status = generateArcTerminator (ep, pCenter, rad, &termTrans, 0.0,
                                        terminator_clockwise, leftIndex))
        return (status);

    ADIM_SETNAME (ep->partName, ADTYPE_TERM_RIGHT, ADSUB_NONE);
    return (generateArcTerminator (ep, pCenter, rad, &termTrans, dimSweep,
                                   !terminator_clockwise, ep->GetDimElementCP()->tmpl.right_term));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     1/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int     generateChordWitness
(
AdimProcess  *ep,            /* => Function used to process elements   */
DPoint3d     *witStart,
DVec3d      *witDir,
double       witLen,
bool         useWitSymb      /* => Use alternate symbology             */
)
    {
    DSegment3d witline;
    double              extend, offset;

    offset = ep->GetDimElementCP()->geom.witOffset;
    extend = ep->GetDimElementCP()->geom.witExtend;

    if (witLen < 0.0)
        {
        offset = -offset;
        extend = -extend;
        }

    mdlDim_overridesGetPointWitnessOffset (&offset, ep->pOverrides, ADIM_GETSEG(ep->partName), offset);
    mdlDim_overridesGetPointWitnessExtend (&extend, ep->pOverrides, ADIM_GETSEG(ep->partName), extend);

    bsiDPoint3d_addScaledDVec3d (&witline.point[0], witStart, witDir, offset);
    bsiDPoint3d_addScaledDVec3d (&witline.point[1], witStart, witDir, witLen + extend);

    return (adim_generateExtensionLine (ep, &witline.point[0], &witline.point[1], useWitSymb));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     1/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int     generateChordDimension
(
AdimProcess     *ep,
DimText const   *dimText,
DPoint3d        *cntr,              /* => center point                 */
double          rad,                /* => radius at dimension line     */
double          dimrad,             /* => radius measured points       */
double          sw,                 /* => sweep angle of dim line      */
RotMatrix       *trans,             /* => transform of dimension       */
DPoint3d        *pt1,               /* => base of first extension      */
DPoint3d        *pt2                /* => base of second extension     */
)
    {
    double      textoff, textsw, charwdth;
    DVec3d      chordCenter, extDir, angleCenter, textDir;
    double      dlOffset;
    int         status;
    bool        offsetAllowed;

    /*-----------------------------------------------------------------------------------
    Calculate chord center and chord normal
    -----------------------------------------------------------------------------------*/
    bsiDPoint3d_interpolate (&chordCenter, pt1, 0.5, pt2);
    extDir.NormalizedDifference (chordCenter, *cntr);
    dlOffset = rad - dimrad;

    /*-----------------------------------------------------------------------------------
    Generate chord extension lines.
    -----------------------------------------------------------------------------------*/
    ADIM_SETNAME (ep->partName, ADTYPE_EXT_LEFT, ADSUB_NONE);
    if (pt1 && !ep->GetDimElementCP()->GetDimTextCP(1)->flags.b.noWitness &&
        (status = generateChordWitness (ep, pt1, &extDir, dlOffset, ep->GetDimElementCP()->GetDimTextCP(1)->flags.b.altSymb))
                                        != SUCCESS)
        return (status);

    ADIM_SETNAME (ep->partName, ADTYPE_EXT_RIGHT, ADSUB_NONE);
    if (pt2 && !dimText->flags.b.noWitness &&
        (status = generateChordWitness (ep, pt2, &extDir, dlOffset, dimText->flags.b.altSymb))
                                        != SUCCESS)
        return (status);

    /*-----------------------------------------------------------------------------------
    Offset the dimension center point in the direction of the chord normal so that the
    remainder of the dimension can be generated with dimrad=rad. The dimension line
    arc will use the same radius of curvature and length as the dimensioned arc.
    -----------------------------------------------------------------------------------*/
    bsiDPoint3d_addScaledDVec3d (&angleCenter, cntr, &extDir, dlOffset);
    rad = dimrad;

    if (status = generateAngularText (ep, &offsetAllowed, &textDir, dimText, &textoff, &textsw,
                                      &charwdth, rad, dimrad, &angleCenter, pt1, pt2, sw, trans, sw))
        return (status);

    return  generateArcDimensionLine (ep, dimText, &angleCenter, &textDir, rad, sw, textsw, textoff, trans, false, offsetAllowed);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     1/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int generateRadialWitness
(
AdimProcess  *ep,            /* => Function used to process elements   */
DPoint3d     *cntr,          /* => Witness line start point            */
DPoint3d     *end,           /* => Witness line end point              */
double       rad,            /* => dimension arc radius                */
bool         useWitSymb      /* => Use alternate symbology             */
)
    {
    DSegment3d witline;
    getRadialExtensionLine (&witline, cntr, end, rad, ep);

    return (adim_generateExtensionLine (ep, &witline.point[0], &witline.point[1], useWitSymb));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     1/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int generateAngularDimension
(
AdimProcess     *ep,
DimText const   *dimText,
DPoint3d        *cntr,
double          rad,
double          dimrad,
double          sw,
RotMatrix       *trans,
DPoint3d        *pt1,
DPoint3d        *pt2,
int             firstSeg,
double          sweepValue
)
    {
    double         textoff, textsw, charwdth;
    int            status;
    bool           offsetAllowed;
    DVec3d         textDir;

    ADIM_SETNAME (ep->partName, ADTYPE_EXT_LEFT, ADSUB_NONE);
    if ((firstSeg || ep->GetDimElementCP()->tmpl.stacked) &&
        pt1 && !ep->GetDimElementCP()->GetDimTextCP(1)->flags.b.noWitness)
        if (status = generateRadialWitness (ep, cntr, pt1, rad, ep->GetDimElementCP()->GetDimTextCP(1)->flags.b.altSymb))
            return (status);

    ADIM_SETNAME (ep->partName, ADTYPE_EXT_RIGHT, ADSUB_NONE);
    if (pt2 && !dimText->flags.b.noWitness)
        if (status = generateRadialWitness (ep, cntr, pt2, rad, dimText->flags.b.altSymb))
            return (status);

    if (status = generateAngularText (ep, &offsetAllowed, &textDir, dimText, &textoff, &textsw,
        &charwdth, rad, dimrad, cntr, pt1, pt2, sw, trans, sweepValue))
        return (status);

    return  generateArcDimensionLine (ep, dimText, cntr, &textDir, rad, sw, textsw, textoff, trans, firstSeg, offsetAllowed);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     1/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int     generateAngleDim
(
DPoint3d       *currpoint,   /* => current data point                  */
AdimProcess    *ep,          /* => process function                    */
DimText const  *dimText,     /* => Dimension text information          */
int            firstSeg,
double          *pAcc        /* => NULL or sweep accumulator           */
)
    {
    RotMatrix   trans;
    DPoint3d    startPoint, centerPoint, endPoint, arcPoint;
    double      rad;
    double      sweep, arcrad;
    int         status;

    if (LegacyMath::RpntEqual (currpoint, ep->points+2) ||
        LegacyMath::RpntEqual (currpoint, ep->points))
        return (ERROR);

    ep->flags.allDontFit =
    ep->flags.fitTermsInside =
    ep->flags.pushTextOutside = false;

#if defined (PROJ_TO_PLANE)
    {
    DPoint3d    zVec, delta, center;

    center = ep->points[2];
    ( ep.rMatrix)->GetColumn(zVec,  2);

    bsiDVec3d_subtractDPoint3dDPoint3d (&delta, &center, ep->points+0);
    bsiDPoint3d_addScaledDVec3d (ep->points+0, ep->points+0, &zVec, bsiDVec3d_dotProduct (&delta, &zVec));

    bsiDVec3d_subtractDPoint3dDPoint3d (&delta, &center, ep->points+1);
    bsiDPoint3d_addScaledDVec3d (ep->points+1, ep->points+1, &zVec, bsiDVec3d_dotProduct (&delta, &zVec));

    bsiDVec3d_subtractDPoint3dDPoint3d (&delta, &center, currpoint);
    bsiDPoint3d_addScaledDVec3d (currpoint, currpoint, &zVec, bsiDVec3d_dotProduct (&delta, &zVec));
    }
#endif

    if (ep->GetDimElementCP()->extFlag.uAngClockwiseSweep)
        {
        startPoint  = *currpoint;
        endPoint    = ep->points[0];
        }
    else
        {
        startPoint  = ep->points[0];
        endPoint    = *currpoint;
        }

    arcPoint    = ep->points[1];
    centerPoint = ep->points[2];

    rad    = bsiDPoint3d_distance (&arcPoint, &centerPoint);
    arcrad = bsiDPoint3d_distance (&ep->points[0], &centerPoint);   // radius for length calculation

    if (rad <= mgds_fc_epsilon)
        rad = 1.0;

    if (arcrad <= mgds_fc_epsilon)
        arcrad = 1.0;

    adim_getSweepTrans (&sweep, &trans, &centerPoint, &startPoint, &endPoint, &ep->rMatrix, ep->GetDimElementCP()->Is3d());

    if (pAcc != NULL)
        *pAcc += sweep;

    if (ep->GetDimElementCP()->dimcmd == static_cast<byte>(DimensionType::ArcSize) && ep->GetDimElementCP()->version >= 7 && ep->GetDimElementCP()->nPoints == 3 &&
        !ep->GetDimElementCP()->frmt.angleMeasure && ep->GetDimElementCP()->tmpl.altExt && sweep < msGeomConst_pi)
        status = generateChordDimension (ep, dimText, &centerPoint, rad, arcrad, sweep, &trans, ep->points, currpoint);
    else
        status = generateAngularDimension (ep, dimText, &centerPoint, rad, arcrad, sweep, &trans,
                                ep->points, currpoint, firstSeg, pAcc ? *pAcc : sweep);

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     1/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int     adim_generateAngleDim1
(
DPoint3d        *currpoint, /* => current data point                   */
AdimProcess     *ep,        /* => process function                     */
DimText const   *dimText,   /* => Dimension text information           */
double          *pAcc       /* => NULL or sweep accumulator            */
)
    {
    return (generateAngleDim (currpoint, ep, dimText, true, pAcc));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     1/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int     adim_generateAngleDim2
(
DPoint3d       *currpoint,  /* => current data point                   */
AdimProcess    *ep,         /* => process function                     */
DimText const  *dimText,    /* => Dimension text information           */
double         *pAcc       /* => NULL or sweep accumulator            */
)
    {
    return (generateAngleDim (currpoint, ep, dimText, false, pAcc));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    adim_projectAnglePoint
(
DPoint3d        *pOut,
const DPoint3d  *pBase,
const DPoint3d  *pSource,
const double    offset
)
    {
    DVec3d      dir;

    bsiDVec3d_subtractDPoint3dDPoint3d (&dir, pSource, pBase);
    dir.Normalize ();
    bsiDPoint3d_addScaledDVec3d (pOut, pBase, &dir, -offset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    adim_changeQuadrant
(
AdimProcess&    adimProcess,
DPoint3d        *pPoints,
const double    offset
)
    {
    UInt16      quad;
    DPoint3d    tmpPnts[3], tmpStore;

    /*-----------------------------------------------------------------------------------
        pPoints + 2 => dim point 0, pivot point
        pPoints     => dim point 1
        pPoints + 3 => dim point 2
    -----------------------------------------------------------------------------------*/
    tmpPnts[0] = pPoints[2];
    tmpPnts[1] = pPoints[0];
    tmpPnts[2] = pPoints[3];

    mdlDim_overridesGetOverallAngleQuadrant (&quad, adimProcess.pOverrides, 0);

    if (adimProcess.GetDimElementCP()->extFlag.uAngClockwiseSweep)
        {
        if (1 == quad)
            quad = 3;
        else if (3 == quad)
            quad = 1;
        }

    /*-----------------------------------------------------------------------------------
        For now just set projected point to be witness offset distance apart from
        pivot point.
    -----------------------------------------------------------------------------------*/
    switch  (quad)
        {
        case 1:
            {
            tmpStore   = tmpPnts[2];
            adim_projectAnglePoint (&tmpPnts[2], &tmpPnts[0], &tmpPnts[1], offset);
            tmpPnts[1] = tmpStore;

            break;
            }
        case 2:
            {
            adim_projectAnglePoint (&tmpPnts[1], &tmpPnts[0], &tmpPnts[1], offset);
            adim_projectAnglePoint (&tmpPnts[2], &tmpPnts[0], &tmpPnts[2], offset);

            break;
            }

        case 3:
            {
            tmpStore   = tmpPnts[1];
            adim_projectAnglePoint (&tmpPnts[1], &tmpPnts[0], &tmpPnts[2], offset);
            tmpPnts[2] = tmpStore;

            break;
            }

        case 0:
        default:
            break;
        }

    // restore
    pPoints[2] = tmpPnts[0];
    pPoints[0] = tmpPnts[1];
    pPoints[3] = tmpPnts[2];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     1/90
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       AngularDimensionHelper::StrokeDimension (AdimProcess& ep) const
    {
    DimensionElm const*  dim = ep.GetDimElementCP();
    if (dim->nPoints <3)
        return ERROR;

    DimStyleProp_FitOptions fitOption = DIMSTYLE_VALUE_FitOption_MoveTermsFirst;
    
    ep.Init ();
    ep.points[2] = dim->GetPoint (0);
    ep.points[0] = dim->GetPoint (1);
    ep.points[3] = dim->GetPoint (2);

    // compute absolute height from pivot point using unswapped points
    double dAbsHeight = bsiDPoint3d_distance (&ep.points[2], &ep.points[0]) + dim->GetDimTextCP(1)->offset;

    // swap points if necessary
    adim_changeQuadrant (ep, ep.points, LegacyMath::DEqual(dim->geom.witOffset, 0.0) ? 1 : fabs(dim->geom.witOffset));

    // compute dimension line height
    DPoint3d end     = ep.points[0];
    DPoint3d origin  = ep.points[2];
    DVec3d  dir;
    dir.NormalizedDifference (end, origin);
    if (dir.IsZero())
        dir.x = 1.0;
    bsiDPoint3d_addScaledDVec3d (&end, &origin, &dir, dAbsHeight);
    ep.points[1] = end;

    double *pAcc=NULL, sweepAccumulator = 0.0;     /* sweep accumulator */
    if (dim->version >= 7 && dim->tmpl.stacked == 0 &&
       (dim->dimcmd == static_cast<byte>(DimensionType::AngleLocation)  || dim->dimcmd == static_cast<byte>(DimensionType::ArcLocation)))
        {
        pAcc = &sweepAccumulator;
        }

    ep.ep.segment = 1;
    ADIM_SETSEG (ep.partName, (ep.ep.segment-1));
    m_hdlr.GetStrings(ep.GetElemHandleCR(), ep.strDat.m_strings, 2, NULL);
    
    mdlDim_getFitOption (&fitOption, ep.GetElemHandleCR());
    ep.flags.fitOption = fitOption;

    int status = SUCCESS;
    if (status = adim_generateAngleDim1 (ep.points+3, &ep, dim->GetDimTextCP(2), pAcc))
        return  status;

    adim_resetStartPoint (ep.points+3, &ep);

    if (ep.ep.segment < ep.stack.segNo)
        ep.stack.height += ep.stackHeight;

    for (int i=3; i<dim->nPoints; i++)
        {
        ep.ep.segment = i-1;

        dimTextBlock_setPopulated (&ep, false);
        ep.m_textBlock = TextBlock::Create (*ep.GetDgnModelP());
        ep.points[3] = dim->GetPoint (i);
        
        ADIM_SETSEG (ep.partName, (ep.ep.segment-1));
        m_hdlr.GetStrings ( ep.GetElemHandleCR(), ep.strDat.m_strings, i, NULL);
        if (status = adim_generateAngleDim2 (ep.points+3, &ep, dim->GetDimTextCP(i), pAcc))
            return  status;

        adim_resetStartPoint (ep.points+3, &ep);

        if (ep.ep.segment < ep.stack.segNo)
            ep.stack.height += ep.stackHeight;
        }

    return status;
    }
