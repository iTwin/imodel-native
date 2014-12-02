/*----------------------------------------------------------------------+
|
|   $Source: DgnHandlers/Dimension/DimLinear.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    "DimStyleInternal.h"

#define SAFE_DOUBLE(x) ((x <= DBL_EPSILON && x >= -DBL_EPSILON) ? 1.0 : x)
#define NONZERO(v)      (fabs(v) > 0.01)

/*----------------------------------------------------------------------+
|                                                                       |
| ** Functions for linear size/location dimensions                      |
|                                                                       |
+----------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  08/00
+---------------+---------------+---------------+---------------+---------------+------*/
static void    computeLeaderOffset
(
DPoint3d*                   offsetOut,
DPoint3d    const* const    offsetIn,
AdimProcess const* const    ep,
double  const               offsetY
)
    {
    DVec3d    offset3d;

    offset3d.x = offset3d.z = 0.0;
    offset3d.y = offsetY;

    ep->rMatrix.Multiply(offset3d);
    bsiDPoint3d_addDPoint3dDPoint3d (offsetOut, offsetIn, &offset3d);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::adim_useWitnessLineOffset (AdimProcess const* ep)
    {
    if (NULL == ep->context)
        return true;

    switch (ep->context->GetDrawPurpose ())
        {
        case DrawPurpose::Pick:
        case DrawPurpose::RangeCalculation:
        case DrawPurpose::FitView:
            return false; // Don't offset when picking/snapping/computing range...

        default:
            return true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
static void    offsetWitnessLine
(
DSegment3dP      witline,
DVec3d          *dir,
AdimProcess     *ep,
double          textLift,
int             nthWitnessLine
)
    {
    double          offset, extend;
    DVec3d          vec;
    DPoint2d        tile;

    adim_getTileSize (&tile, NULL, ep->GetElemHandleCR());

    mdlDim_overridesGetPointWitnessOffset (&offset, ep->pOverrides, nthWitnessLine, ep->GetDimElementCP()->geom.witOffset);
    mdlDim_overridesGetPointWitnessExtend (&extend, ep->pOverrides, nthWitnessLine, ep->GetDimElementCP()->geom.witExtend);

    if (textLift)
        bsiDPoint3d_addScaledDVec3d (&witline->point[1], &witline->point[1], dir, textLift);

    bsiDVec3d_subtractDPoint3dDPoint3d (&vec, &witline->point[1], &witline->point[0]);

    if (bsiDVec3d_dotProduct (&vec, dir) < 0.0)
        {
        offset = -offset;
        extend = -extend;
        }

    if (ep->GetDimElementCP()->geom.witOffset < 0.0)
        bsiDPoint3d_addScaledDVec3d (&witline->point[0], &witline->point[1], dir, offset);
    else
    if (adim_useWitnessLineOffset (ep))
        bsiDPoint3d_addScaledDVec3d (&witline->point[0], &witline->point[0], dir, offset);

    bsiDPoint3d_addScaledDVec3d (&witline->point[1], &witline->point[1], dir, extend);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
static void     adim_offsetStackPoint
(
DPoint3d        *pnt,
double          dist,
RotMatrix       *rMatrix
)
    {
    DVec3d      yvec;

    rMatrix->GetColumn(yvec,  1);
    bsiDPoint3d_addScaledDVec3d (pnt, pnt, &yvec, dist);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/04
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        isHorizontalDimensionLine
(
AdimProcess const*  pAdimProcess
)
    {
    DVec3d    dimXAxis, viewXAxis;

    pAdimProcess->rMatrix.GetColumn(dimXAxis,  0);
    pAdimProcess->vuMatrix.GetRow(viewXAxis,  0);

    /* is dimension x-axis parallel to the view x-axis ? */
    return LegacyMath::Vec::AreParallel(&dimXAxis, &viewXAxis);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 10/03
+---------------+---------------+---------------+---------------+---------------+------*/
static double          computeHorizontalTextLocationShiftForLinearAndAngularDims
(
AdimProcess*            ep,
DPoint3d*               pTextDir,
DPoint2dP               pTextSize,
int                     textLocation
)
    {
    double              dVerShift = 0.0;
    double              dHorShift = 0.0;
    double              dNetShift = 0.0;
    DVec3d              dimdir;
    DVec3d              extdir;

    // Compute the shifts for totally horizontal and totally vertical cases
    dHorShift = 0.5 * pTextSize->x + ep->GetDimElementCP()->geom.textMargin;
    dVerShift = 0.5 * pTextSize->y + ep->GetDimElementCP()->geom.textLift;

    ep->rMatrix.GetColumn(dimdir,  0);
    ep->rMatrix.GetColumn(extdir,  1);

    // Get the true extension direction by looking at the sign of the extension offset
    if (ep->GetDimElementCP()->GetDimTextCP(0)->offset < 0.0)
        bsiDVec3d_scale (&extdir, &extdir, -1.0);

    ep->vuMatrix.Multiply(dimdir);
    ep->vuMatrix.Multiply(extdir);

    switch (textLocation)
        {
        case DIMSTYLE_VALUE_Text_Location_Outside:
            {
            dNetShift = (dimdir.x * (extdir.y > 0.0 ?  1.0 : -1.0) * dVerShift +
                         dimdir.y * (extdir.x > 0.0 ? -1.0 :  1.0) * dHorShift);
            break;
            }

        case DIMSTYLE_VALUE_Text_Location_TopLeft:
            {
            dNetShift = fabs (dimdir.x) * dVerShift + fabs (dimdir.y) *  dHorShift;

            // If the dimension direction is in the 2nd or 3rd quadrants, shift the
            // text above the dimension line
            if (dimdir.x < mgds_fc_epsilon)
                dNetShift *= -1.0;
            break;
            }

        case DIMSTYLE_VALUE_Text_Location_Above:
            {
            // Shift the text above the dimline only if the dimline is totally horizontal
            if (isHorizontalDimensionLine (ep))
                dNetShift = dimdir.x * dVerShift;
            break;
            }

        case DIMSTYLE_VALUE_Text_Location_Inline:
            {
            // Use the special dwg inline text lift only if the dimline is totally horizontal
            if (isHorizontalDimensionLine (ep) && mdlDim_extensionsGetInlineTextLift (&dNetShift, ep->pOverrides, 0.0))
                dNetShift *= pTextSize->y;
            break;
            }
        }

    return dNetShift;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 10/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void shiftAlignedTextLocationLinearAndAngularDims
(
AdimProcess*            ep,                //  =>
DPoint3dP               pTextOrigin,       // <=
DVec3dP                 pTextDir,          //  =>
DPoint2dP               pTextSize,         //  =>
int                     textLocation       //  =>
)
    {
    switch (textLocation)
        {
        case DIMSTYLE_VALUE_Text_Location_Above:
        case DIMSTYLE_VALUE_Text_Location_TopLeft:
            {
            double      dOffsetY = pTextSize->y / 2.0 + ep->GetDimElementCP()->geom.textLift;
            adim_offsetText (pTextOrigin, pTextOrigin, pTextDir, dOffsetY, ep);
            break;
            }
        case DIMSTYLE_VALUE_Text_Location_Outside:
            {
            double      dOffsetY = pTextSize->y / 2.0 + ep->GetDimElementCP()->geom.textLift;
            DVec3d      extdir;
            ep->rMatrix.GetColumn(extdir,  1);

            // Get the true extension direction by looking at the sign of the extension offset
            if (ep->GetDimElementCP()->GetDimTextCP(0)->offset < 0.0)
                bsiDVec3d_scale (&extdir, &extdir, -1.0);

            bsiDPoint3d_addScaledDVec3d (pTextOrigin, pTextOrigin, &extdir, dOffsetY);
            break;
            }
        case DIMSTYLE_VALUE_Text_Location_Inline:
            {
            double      dShift = 0.0;
            if (mdlDim_extensionsGetInlineTextLift (&dShift, ep->pOverrides, 0.0))
                dShift *= pTextSize->y;
            adim_offsetText (pTextOrigin, pTextOrigin, pTextDir, dShift, ep);
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
@description    Generate dimension text and symbols.
*
* @param        ep          IN OUT  adim process information
* @param        startpt     IN      dimension line mid point or NULL if not with leader
* @param        origin      IN      dimension text origin point
* @param        direction   IN      text running direction
* @return       element process status
* @bsimethod                                                    petri.niiranen  08/00
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt BentleyApi::adim_generateLinearDimensionWithOffset
(
AdimProcess*            ep,
DPoint3d const* const   startpt,
DPoint3d const* const   origin,
DVec3d   const* const   direction,
const double            offsetY
)
    {
    DPoint3d        upper_origin, lower_origin, rtxtend;
    DVec3d          rotatedDirection;
    DPoint3d        chainedTextOrigin, rotatedOrigin;
    DPoint2d        textSize;
    DimStringData   *dstr;
    double          charWidth;
    double          upper_offset;
    double          lower_offset;
    double          dual_adheight;
    double          lineWidth;
    double          dOffsetY = 0.0;
    int             status = 0;

    dstr      = &ep->strDat;
    charWidth = dstr->charWidth;

    adim_getTextSize (&textSize, ep, dstr, ADIM_TEXTSIZE_Nominal);

    /*-----------------------------------------------------------------------------------
        Rotation
    -----------------------------------------------------------------------------------*/
    rotatedDirection = *direction;
    rotatedOrigin = *origin;
    adim_rotateText (&rotatedDirection, &rotatedOrigin, &textSize, ep);

    /*-----------------------------------------------------------------------------------
        Dual
    -----------------------------------------------------------------------------------*/
    if (ep->GetDimElementCP()->flag.dual)
        {
        bool        usesTextBlock = ep->flags.textBlockPopulated;
        bool        reallyMultiline = usesTextBlock;
        DimStyleProp_Text_Location textLocation = DIMSTYLE_VALUE_Text_Location_Inline;

        if (reallyMultiline)
            {
            size_t lineCount = ep->m_textBlock->GetLineCount (ep->m_textBlock->Begin (), ep->m_textBlock->End ());
            reallyMultiline = (lineCount > 1);
            }

        dual_adheight = textSize.y / 2.0 + ep->GetDimElementCP()->geom.textLift;
        textSize.y    += ep->GetDimElementCP()->geom.textLift * 2.0;

        adim_generateBallAndChain (&chainedTextOrigin, ep, &rotatedOrigin, startpt, &rotatedDirection, &textSize, offsetY);

        if (ep->GetDimElementCP()->flag.horizontal && DIMSTYLE_VALUE_Text_Location_Inline == textLocation &&
            isHorizontalDimensionLine (ep) &&
            mdlDim_extensionsGetInlineTextLift(&dOffsetY, ep->pOverrides, 0.0))
            {
            dOffsetY *= textSize.y;
            adim_offsetText (&chainedTextOrigin, &chainedTextOrigin, direction, dOffsetY, ep);
            }

        ADIM_SETNAME (ep->partName, ADTYPE_TEXT_UPPER, ADSUB_NONE);
        if (SUCCESS != (status = adim_generateTextSymbols (ep, charWidth, &textSize, &chainedTextOrigin, &rotatedDirection)))
            {
            return  status;
            }

        adim_offsetText (&upper_origin, &chainedTextOrigin, &rotatedDirection, dual_adheight, ep);
        adim_offsetText (&lower_origin, &chainedTextOrigin, &rotatedDirection, -dual_adheight, ep);

        // ---------------------------------
        // Multiline or not.
        // Note : If the primary dimension truly has multiple lines of text, then it can potentially overlap
        // the secondary text. Therefore the secondary text is not displayed. Check to make sure the primary
        // text really has multiple lines of text. A single line of text with stacked fraction is stored as
        // multiline text. So get the exact line count from the line arranger.
        // ---------------------------------
        if (reallyMultiline)
            {
            // don't compute offsets with mltext. don't display secondary text

            if (SUCCESS != (status = adim_generateSingleDimension (ep, true, &upper_origin, &rotatedDirection)))
                {
                return  status;
                }
            }
        else if (usesTextBlock)
            {
            // legacy style dual dimensions with multiline text. don't compute offset for primary (ml)

            if (SUCCESS != (status = adim_generateSingleDimension (ep, true, &upper_origin, &rotatedDirection)))
                return status;

            lower_offset = (textSize.x - dstr->lowerSize.x) / 2.0;
            bsiDPoint3d_addScaledDVec3d (&lower_origin, &lower_origin, &rotatedDirection, lower_offset);

            ADIM_SETNAME (ep->partName, ADTYPE_TEXT_LOWER, ADSUB_NONE);
            if (SUCCESS != (status = adim_generateSingleDimension (ep, false, &lower_origin, &rotatedDirection)))
                return  status;
            }
        else
            {
            // legacy style dual dimensions with single line text

            upper_offset = (textSize.x - dstr->upperSize.x) / 2.0;
            bsiDPoint3d_addScaledDVec3d (&upper_origin, &upper_origin, &rotatedDirection, upper_offset);

            if (SUCCESS != (status = adim_generateSingleDimension (ep, true, &upper_origin, &rotatedDirection)))
                return status;

            lower_offset = (textSize.x - dstr->lowerSize.x) / 2.0;
            bsiDPoint3d_addScaledDVec3d (&lower_origin, &lower_origin, &rotatedDirection, lower_offset);

            ADIM_SETNAME (ep->partName, ADTYPE_TEXT_LOWER, ADSUB_NONE);
            if (SUCCESS != (status = adim_generateSingleDimension (ep, false, &lower_origin, &rotatedDirection)))
                return  status;
            }

        // ---------------------------------
        // Separation line
        // ---------------------------------
        mdlDim_overridesGetTextLocation (&textLocation, ep->pOverrides, ep->flags.embed);
        if (DIMSTYLE_VALUE_Text_Location_Inline == textLocation)
            {
            lineWidth = textSize.x - dstr->symWidth;
            bsiDPoint3d_addScaledDVec3d (&rtxtend, &chainedTextOrigin, &rotatedDirection, lineWidth);

            ADIM_SETNAME (ep->partName, ADTYPE_DIMLINE, ADSUB_NONE);
            status = adim_generateLine (ep, &chainedTextOrigin, &rtxtend, DIM_MATERIAL_DimLine);
            }
        }
    else
        {
        /*-------------------------------------------------------------------------------
            Single
        -------------------------------------------------------------------------------*/
        chainedTextOrigin = rotatedOrigin;

        /*-------------------------------------------------------------
          Shift the origin with respect to text location flag only
          when there is no ball and chain => startpt == NULL
        -------------------------------------------------------------*/
        if (NULL == startpt)
            {
            DimStyleProp_Text_Location textLocation = DIMSTYLE_VALUE_Text_Location_Inline;
            mdlDim_overridesGetTextLocation (&textLocation, ep->pOverrides, ep->flags.embed);

            if (mdlDim_isRadialDimension (ep->GetElemHandleCR()) || mdlDim_isOrdinateDimension (ep->GetElemHandleCR()))
                {
                if (DIMSTYLE_VALUE_Text_Location_Inline != textLocation)
                    adim_offsetText (&chainedTextOrigin, &chainedTextOrigin, &rotatedDirection, textSize.y / 2.0 + ep->GetDimElementCP()->geom.textLift, ep);
                }
            else
                {
                if (ep->GetDimElementCP()->flag.horizontal)
                    {
                    DVec3d      dimDir;
                    ep->rMatrix.GetColumn(dimDir,  0);
                    adim_offsetText (&chainedTextOrigin, &chainedTextOrigin, &dimDir, computeHorizontalTextLocationShiftForLinearAndAngularDims (ep, &rotatedDirection, &textSize, textLocation), ep);
                    }
                else
                    {
                    shiftAlignedTextLocationLinearAndAngularDims (ep, &chainedTextOrigin, &rotatedDirection, &textSize, textLocation);
                    }
                }
            }

        adim_generateBallAndChain (&chainedTextOrigin, ep, &chainedTextOrigin, startpt, &rotatedDirection, &textSize, offsetY);

        // Legacy stroke
        ADIM_SETNAME (ep->partName, ADTYPE_TEXT_SINGLE, ADSUB_NONE);

        if (SUCCESS != (status = adim_generateTextSymbols (ep, charWidth, &textSize, &chainedTextOrigin, &rotatedDirection)))
            return  status;

        status = adim_generateSingleDimension (ep, true, &chainedTextOrigin, &rotatedDirection);
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
int BentleyApi::adim_generateLinearDimension
(
AdimProcess  *ep,            /* => Function used to process elements   */
DPoint3d     *origin,        /* => Dimension start point               */
DVec3d       *direction,     /* => Direction of dimension text         */
const double  offsetY        /* => segment text info                   */
)
    {
    return  adim_generateLinearDimensionWithOffset (ep, NULL, origin, direction, offsetY);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    termsAreInside
(
bool            defaultVal, /* => */
AdimProcess    *ep          /* => */
)
    {
    bool    returnVal = defaultVal;

    switch (ep->GetDimElementCP()->flag.termMode)
        {
        case 0:         /* Automatic */
            returnVal = ep->flags.fitTermsInside ? true : defaultVal;
            break;
        case 1:         /* Reversed */
            returnVal = !defaultVal;
            break;
        case 2:         /* Inside */
            returnVal = true;
            break;
        case 3:         /* Outside */
            returnVal = false;
            break;
        }

    return returnVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int     generateVerticalDimension
(
AdimProcess     *ep,
DPoint3d        *dimOrg,
DPoint3d        *dimEnd,
DPoint3d        *witBase,
double          textWidth,
double          margin,
int             leftTerm,
int             rightTerm,
DPoint2dCR      offset,
int             textJust
)
    {
    int       status;
    bool      multiline;
    DVec3d    witDir, textDir, check, dimDirection;
    DPoint3d  textOrg;

    multiline = ep->flags.textBlockPopulated;

    ADIM_SETNAME (ep->partName, ADTYPE_DIMLINE, ADSUB_NONE);
    if (status = adim_generateDimLine (ep, dimOrg, dimEnd, DIM_MATERIAL_DimLine,
                                       leftTerm, rightTerm, TRIM_BOTH, true,
                                       termsAreInside (true, ep), true))
        return (status);

    /* Get the dimension line direction */
    bsiDVec3d_subtractDPoint3dDPoint3d (&dimDirection, dimEnd, dimOrg);
    dimDirection.Normalize ();

    /* Locate the dimension's position along the dimension line */
    if (DIMTEXT_START == textJust)
        textOrg = *dimOrg;
    else if (DIMTEXT_CENTER == textJust)
        bsiDPoint3d_interpolate (&textOrg, dimOrg, 0.5, dimEnd);
    else if (DIMTEXT_END == textJust)
        textOrg = *dimEnd;
    else
        bsiDPoint3d_addScaledDVec3d (&textOrg, dimOrg, &dimDirection, offset.x);

    // TR-59847 : Use dim matrix's y-bvector as text direction instead of
    // end point to witBase bvector
    ep->rMatrix.GetColumn(witDir,  1);

    check = witDir;

    ep->vuMatrix.Multiply(check);

    if (check.x < -mgds_fc_epsilon || fabs (check.y + 1.0) < 0.0001)
        bsiDVec3d_scale (&textDir, &witDir, -1.0);
    else
        textDir = witDir;

    /* Compute the origin along the dimension's perpendicular dir. The text
     * will be placed away from the dimension line by text margin distance.
     * If it is not multiline text, the text origin should correspond to the
     * left-center position.
     * If it is multiline text, the text origin should correspond to the
     * txt justification stored in the dimension.
     */
    if (LegacyMath::RpntEqual (&textDir, &witDir))
        {
        if (!multiline)
            {
            bsiDPoint3d_addScaledDVec3d (&textOrg, &textOrg, &textDir, margin);
            }
        else
            {
            switch  (adim_getDimTextJustificationHorizontal (ep))
                {
                case TextElementJustification::RightMiddle:
                bsiDPoint3d_addScaledDVec3d (&textOrg, &textOrg, &textDir, margin + textWidth);
                break;

                case TextElementJustification::CenterMiddle:
                bsiDPoint3d_addScaledDVec3d (&textOrg, &textOrg, &textDir, margin + textWidth / 2.0);
                break;

                case TextElementJustification::LeftMiddle:
                bsiDPoint3d_addScaledDVec3d (&textOrg, &textOrg, &textDir, margin);
                break;
                }
            }
        }
    else
        {
        if (!multiline)
            {
            bsiDPoint3d_addScaledDVec3d (&textOrg, &textOrg, &textDir, -(textWidth + margin));
            }
        else
            {
            switch  (adim_getDimTextJustificationHorizontal (ep))
                {
                case TextElementJustification::RightMiddle:
                bsiDPoint3d_addScaledDVec3d (&textOrg, &textOrg, &textDir, -margin);
                break;

                case TextElementJustification::CenterMiddle:
                bsiDPoint3d_addScaledDVec3d (&textOrg, &textOrg, &textDir, -(margin + textWidth / 2.0));
                break;

                case TextElementJustification::LeftMiddle:
                bsiDPoint3d_addScaledDVec3d (&textOrg, &textOrg, &textDir, -(textWidth + margin));
                break;
                }
            }
        }

    if (status = adim_generateLinearDimension (ep, &textOrg, &textDir, offset.y))
        return (status);

    ADIM_SETNAME (ep->partName, ADTYPE_TERM_LEFT, ADSUB_NONE);
    if (status = adim_generateLineTerminator (ep, dimEnd, dimOrg, leftTerm, false))
        return (status);

    ADIM_SETNAME (ep->partName, ADTYPE_TERM_RIGHT, ADSUB_NONE);
    return (adim_generateLineTerminator (ep, dimOrg, dimEnd, rightTerm, false));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
static void getRightWitness
(
DSegment3dP wit2,             /* <= Right (2nd) witness line            */
DPoint3d  *wit1End,          /* => Left (1st) witness end point        */
DPoint3d  *wit2Org,          /* => Right (2nd) witness origin          */
DVec3d  *xDir,               /* => Dimension X axis                    */
DVec3d  *yDir                /* => Dimension Y axis                    */
)
    {
    if (LegacyMath::DEqual (bsiDVec3d_dotProduct (xDir, yDir), 0.0))
        {
        DVec3d      diagVec;
        double      xDist, yDist;

        bsiDVec3d_subtractDPoint3dDPoint3d (&diagVec, wit2Org, wit1End);

        xDist = bsiDVec3d_dotProduct (xDir, &diagVec);
        yDist = bsiDVec3d_dotProduct (yDir, &diagVec);

        bsiDPoint3d_addScaledDVec3d (&wit2->point[1], wit1End,    xDir, xDist);
        bsiDPoint3d_addScaledDVec3d (&wit2->point[0], &wit2->point[1], yDir, yDist);
        return;
        }

    wit2->point[0] = *wit2Org;
    DPoint3d closePoint1, closePoint2;
    if (bsiGeom_closestApproachOfRays (NULL, NULL, &closePoint1, &closePoint2,
            wit1End, yDir, wit2Org, xDir))
        {
        if (closePoint1.distance(&closePoint2) <= 1) // 1 uor was the tolerance used earlier
            {
            double          wit2Dist;
            bsiDVec3d_subtractDPoint3dDPoint3d ((DVec3d*)&closePoint1, wit2Org, &closePoint1);
            wit2Dist = bsiDVec3d_dotProduct ((DVec3d*)&closePoint1, xDir);
            bsiDPoint3d_addScaledDVec3d (&wit2->point[1], wit1End, xDir, wit2Dist);
            return;
            }
        }
    
    wit2->point[1] = wit2->point[0];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
static double adjustStackHeight
(
DimensionElm const *dim,           /* => dimension to adjust offset for */
DimStringData   *strDatP        /* => */
)
    {
    double stack = 0.0;

    if (dim->flag.boxText || dim->flag.capsuleText)
        stack += strDatP->charWidth;

    if (dim->tmpl.above_symbol == 1)
        stack += strDatP->upperSize.y;

    if (!dim->flag.embed)
        stack += strDatP->charWidth;

    return (stack);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
static double  getStackOffset
(
DimensionElm const  *dim,       /* => dimension to compute offset for */
DPoint2dP            textSizeP, /* => */
DimStringData       *strDatP,   /* => */
const DPoint3d      *check      /* => direction of text */
)
    {
    double  textMargin = (double)dim->geom.textMargin;
    double  textMar2   = textMargin * 2.0;
    double  offset     = 0.0;

    if (dim->geom.stackOffset != 0.0)
        offset = (double) dim->geom.stackOffset;
    else
        {
        if (dim->flag.horizontal)
            {
            if (fabs (check->x*textSizeP->x) > fabs (check->y*textSizeP->y))
                offset = fabs ((textSizeP->y + textMar2) / SAFE_DOUBLE(check->x));
            else
                offset = fabs ((textSizeP->x + textMar2) / SAFE_DOUBLE(check->y));
            }
        else
            {
            offset  = textSizeP->y + textMargin;
            }

        offset += adjustStackHeight (dim, strDatP);
        }

    return  offset;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    SamWilson                       09/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt projectToLine
(
DPoint3d        *pPoint,        // <= IN: point on plane; OUT: point projected onto line
const DVec3d    *pNormal,       //  => plane normal
const DPoint3d  *pLineStart,    //  => point on line
const DPoint3d  *pLineEnd       //  => 2nd point on line
)
    {
    DPlane3d plane = DPlane3d::FromOriginAndNormal (*pPoint, *pNormal);
    DRay3d ray;
    bsiDRay3d_initFromDPoint3dStartEnd (&ray, pLineStart, pLineEnd);
    double param;
    return ray.Intersect(*pPoint, param, plane) ? SUCCESS: ERROR;
    }

/*----------------------------------------------------------------------------------*//**
* Given range box enclosing text, detect if dimension line intersects the text range.
* If so, return where to lift and drop the pen when drawing the line in order to
* avoid crossing the text. See Note(2)
*
* @bsimethod                    SamWilson                       09/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt intersectDimensionLineWithTextBox
(
DPoint3d            *pEnd1,         // <= end of first segment
DPoint3d            *pStart2,       // <= start of second segment
const DPoint3d      *pStart,        //  => start of dimension line
const DPoint3d      *pEnd,          //  => end of dimension line
AdimRotatedTextBox  *pTextBox,      //  => intersect with this box
AdimProcess         *ep             //  => Function used to process elements
)
    {
    StatusInt   status = SUCCESS;
    bool        notCoplanar=false;
    int         nIsect;
    DPoint3d    corners[4], pts[2];
    DVec3d      upDir;
    RotMatrix   rText;

    //  Text stroker didn't set data for me?
    if (pTextBox->height == 0.0)
/*<=*/  return ERROR;

    /*-----------------------------------------------------------------------------------
        corners <- corners of the fitted/rotated text box (in world coordinate space)
        upDir   <- text 'y' local direction
    -----------------------------------------------------------------------------------*/
    if (true)
        {
        DPoint3d    boxorg;
        DVec3d      dimdir;
        double      boxh, boxw;
        double      xMargin = fabs (ep->GetDimElementCP()->geom.textMargin);
        double      yMargin = fabs (ep->strDat.charWidth) / 2.0;

        dimdir.NormalizedDifference (*pEnd, *pStart);
        ep->vuMatrix.Multiply(dimdir);

        // prevent unnecessary intersections due to ymargin. Note the special
        // case of hor-above in which the text does not intersect with dimline
        // only when the dimline is horizontal; in all other dimline orientations
        // the text is placed inline.
        if (!ep->GetDimElementCP()->flag.horizontal || fabs (fabs (dimdir.x) - 1.0) < mgds_fc_epsilon)
            yMargin = 0.0;

        // NB: we assume pTextBox->baseDir is stated in world CS, not in view LCS!

        //  The direction "up" along the side of the text box
        bsiDVec3d_crossProduct (&upDir, &pTextBox->zvec, &pTextBox->baseDir);

        //  Expand the box by the margin
        bsiDPoint3d_addScaledDVec3d (&boxorg, &pTextBox->baseFirstChar, &pTextBox->baseDir, -1.0*xMargin);
        bsiDPoint3d_addScaledDVec3d (&boxorg, &boxorg, &upDir, -1.0*yMargin);
        boxw = pTextBox->width  + 2.0*xMargin;
        boxh = pTextBox->height + 2.0*yMargin;

        //  Plot the corners of the text box
        corners[0] = boxorg;
        bsiDPoint3d_addScaledDVec3d (&corners[1], &corners[0], &pTextBox->baseDir,  boxw);
        bsiDPoint3d_addScaledDVec3d (&corners[2], &corners[1], &upDir,              boxh);
        bsiDPoint3d_addScaledDVec3d (&corners[3], &corners[2], &pTextBox->baseDir, -boxw);

#if defined (DEBUG_DISPLAY_TEXTBOX)
            {
            DPoint3d boxPts[5];

            boxPts[0] = corners[0];
            boxPts[1] = corners[1];
            boxPts[2] = corners[2];
            boxPts[3] = corners[3];
            boxPts[4] = corners[0];

            adim_generateLineString (ep, boxPts, 5, 1, DIM_MATERIAL_DimLine);
            }
#endif
        }
    /*-----------------------------------------------------------------------------------
        pts   <-   intersections of dimension line w/ text box, if any
        nPts  <- #      "
        notCoplanar <- 3D: are text and dimension in different (and maybe skewed) planes?
    -----------------------------------------------------------------------------------*/
    if (true)
        {
        int         i;
        DPoint3d    dimLine[2];
        Transform   toWorld;
        bool        notInXY = (NONZERO (corners[0].z) || NONZERO (pStart->z) || NONZERO (pEnd->z));

        dimLine[0] = *pStart;
        dimLine[1] = *pEnd;

        // 3D:  Not working in X-Y plane? Project onto view LCS.
        //
        if (notInXY)
            {
            //  Restate the problem in the LCS of the text.
            Transform   toPlane;
            rText.InitFromColumnVectors(pTextBox->baseDir, upDir, pTextBox->zvec);
            LegacyMath::TMatrix::ComposeLocalOriginOperations (&toPlane,  NULL, NULL,  &rText, &corners[0]);
            LegacyMath::TMatrix::ComposeLocalOriginOperations (&toWorld, &corners[0], &rText, NULL, NULL);  // (for later)

            toPlane.Multiply (corners,  4);
            toPlane.Multiply (dimLine,  2);

            notCoplanar = (NONZERO (dimLine[0].z) || NONZERO (dimLine[1].z));
            }
        //  We can now assume that we are now working in the plane (ignore z-coordinates)

        //  Find up to two points where dimension line intersects the text box edges
        //
        nIsect = 0;
        for (i=0; i<4 && nIsect<2; ++i)
            {
            double      where = 0;
            if (bsiVector2d_intersectLines ((DPoint2d*)&pts[nIsect],
                                                        NULL, &where,
                                                        (DPoint2d*)&dimLine[0], (DPoint2d*)&dimLine[1],
                                                        (DPoint2d*)&corners[i], (DPoint2d*)&corners[(i+1)%4])
                && where>=0 && where<=1.0)
                {
                pts[nIsect].z = 0.0;

                // 3D: project back into World CS
                if (notInXY)
                    toWorld.Multiply (&pts[nIsect],  1);

                //  record the hit
                nIsect++;
                }
            }
        }

    /*-----------------------------------------------------------------------------------
        Tell the caller where to break the dimension line.
    -----------------------------------------------------------------------------------*/
    if (nIsect < 2)                 // (if there's 1 intersection, it must be at a corner point -- ignore it.)
        {
        *pEnd1 = *pStart2 = *pEnd;  // line runs all the way w/o intersecting text box
        return -1; // return non-success, there is no intersection
        }
    else
        {
        //  The intersection closest to the start point is where we lift the pen ...
        //
        int firstHit = (bsiDPoint3d_distance (&pts[0], pStart) < bsiDPoint3d_distance (&pts[1], pStart))? 0: 1;
        *pEnd1      = pts[firstHit];
        *pStart2    = pts[(firstHit+1)%2];

        //  3D: if text and dimension are not coplanar
        //
        if (notCoplanar)
            {
            //  Must project apparent intersection to the dimension line
            if ((status = projectToLine (pEnd1,   &pTextBox->zvec, pStart, pEnd)) == SUCCESS)
                 status = projectToLine (pStart2, &pTextBox->zvec, pStart, pEnd);
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt intersectDimensionLineWithText
(
DPoint3d        *pEnd1,         // <= end of first segment
DPoint3d        *pStart2,       // <= start of second segment
const DPoint3d  *pStart,        //  => start of dimension line
const DPoint3d  *pEnd,          //  => end of dimension line
AdimProcess     *ep             //  => Function used to process elements
)
    {
    StatusInt   status;
    StatusInt   upperStatus = intersectDimensionLineWithTextBox (pEnd1, pStart2, pStart, pEnd, &ep->textBox[0], ep);

    if (!ep->GetDimElementCP()->flag.dual)
        {
        status = upperStatus;
        }
    else
        {
        DPoint3d    lowerEnd1, lowerStart2;
        StatusInt   lowerStatus;

        lowerStatus = intersectDimensionLineWithTextBox (&lowerEnd1, &lowerStart2, pStart, pEnd, &ep->textBox[1], ep);

        if (SUCCESS == upperStatus)
            {
            if (SUCCESS == lowerStatus)
                {
                // upper and lower succeeded, choose between the two
                if (bsiDPoint3d_distanceSquared (&lowerEnd1, pStart) < bsiDPoint3d_distanceSquared (pEnd1, pStart))
                    *pEnd1   = lowerEnd1;

                if (bsiDPoint3d_distanceSquared (&lowerStart2, pEnd) < bsiDPoint3d_distanceSquared (pStart2, pEnd))
                    *pStart2 = lowerStart2;

                status = SUCCESS;
                }
            else
                {
                // upper succeeded, lower failed
                status = SUCCESS;
                }
            }
        else
            {
            if (SUCCESS == lowerStatus)
                {
                // upper failed, lower succeeded
                *pEnd1   = lowerEnd1;
                *pStart2 = lowerStart2;

                status = SUCCESS;
                }
            else
                {
                // upper failed, lower failed
                status = ERROR;
                }
            }
        }

    return status;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    SamWilson                       09/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    clearTextBox
(
AdimProcess    *ep           /* => Function used to process elements   */
)
    {
    memset (&ep->textBox, 0, sizeof (ep->textBox));
    }

/*---------------------------------------------------------------------------------**//**
* Determine whether the dimension text resides on dimensions left side.
* @bsimethod                                                    petri.niiranen  02/02
+---------------+---------------+---------------+---------------+---------------+------*/
static bool         isOnLeftSide
(
AdimProcess const*  ep,
double      const   width,
double      const   lineLength,
double const        offsetx,
int                 textJust
)
    {
    if (!textJust)
        {
        bool    const    multiline  = ep->flags.textBlockPopulated;
        bool    const    horizontal = ep->GetDimElementCP()->flag.horizontal;
        double           leftEdge;

        if (horizontal && multiline)
            leftEdge = offsetx - width / 2.0;
        else
            leftEdge = BentleyApi::adim_computeLeftEdgeOffset (ep, offsetx, width);

        return  leftEdge < lineLength;
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* Determine whether the dimension text resides on dimensions right side.
* @bsimethod                                                    petri.niiranen  02/02
+---------------+---------------+---------------+---------------+---------------+------*/
static bool         isOnRightSide
(
AdimProcess const*  ep,
double      const   width,
double      const   lineLength,
double      const   offsetx,
int                 textJust
)
    {
    if (!textJust)
        {
        bool    const    multiline  = ep->flags.textBlockPopulated;
        bool    const    horizontal = ep->GetDimElementCP()->flag.horizontal;
        double           leftEdge;

        if (horizontal && multiline)
            leftEdge = offsetx - width / 2.0;
        else
            leftEdge = BentleyApi::adim_computeLeftEdgeOffset (ep, offsetx, width);

        return  lineLength < (leftEdge + width);
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        isDimlineNearlyHorizontal
(
DPoint3d const      *pCheck
)
    {
    /* horizontal within 15 tolerance */
    return  fabs(pCheck->x) < mgds_fc_epsilon ? false : fabs(pCheck->y/pCheck->x) < 0.267949192;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        isDimlineNearlyVertical
(
DPoint3d const      *pCheck
)
    {
    /* vertical within 15 tolerance */
    return  fabs(pCheck->x) < mgds_fc_epsilon ? true : fabs(pCheck->y/pCheck->x) > 3.732050808;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        isInclinedDimensionWithHorizontalText
(
AdimProcess const   *pAdimProcess,
DPoint3d const      *pCheck
)
    {
    DimStyleProp_Text_Location  textLocation = DIMSTYLE_VALUE_Text_Location_Inline;

    mdlDim_overridesGetTextLocation (&textLocation, pAdimProcess->pOverrides, pAdimProcess->flags.embed);

    if (pAdimProcess->GetDimElementCP()->flag.horizontal &&
        (DIMSTYLE_VALUE_Text_Location_Inline == textLocation || DIMSTYLE_VALUE_Text_Location_Above == textLocation))
        {
        /* Check near horizontal dimension with 15 tolerance */
        return  !isDimlineNearlyHorizontal(pCheck);
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        isDimensionLineIntersectedByText
(
AdimProcess const   *pAdimProcess
)
    {
    bool                        isAlignedOffline, isHorizontalAbove;
    DimStyleProp_Text_Location  textLocation = DIMSTYLE_VALUE_Text_Location_Inline;

    mdlDim_overridesGetTextLocation (&textLocation, pAdimProcess->pOverrides, pAdimProcess->flags.embed);

    /* is it an aligned text that is not inline? */
    isAlignedOffline  = !pAdimProcess->GetDimElementCP()->flag.horizontal && DIMSTYLE_VALUE_Text_Location_Inline != textLocation;

    /* is it a horizontal text that is above horizontal dimension line? */
    isHorizontalAbove = pAdimProcess->GetDimElementCP()->flag.horizontal && DIMSTYLE_VALUE_Text_Location_Above == textLocation && isHorizontalDimensionLine(pAdimProcess);

    return  !(isAlignedOffline || isHorizontalAbove);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        isTextAboveWithNoMinLeader
(
AdimProcess const   *pAdimProcess
)
    {
    if (0 == pAdimProcess->GetDimElementCP()->flag.termMode)
        {
        bool        noMinleader = false;

        mdlDim_extensionsGetTightFitTextAbove (&noMinleader, pAdimProcess->pOverrides, noMinleader);

        if (noMinleader && !isDimensionLineIntersectedByText(pAdimProcess))
            return  true;
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        checkInclinedDimensionTextWidth
(
double              *pCheckSize,
AdimProcess const   *pAdimProcess,
DPoint2dCP           pTextSize,
DPoint3d const      *pCheck
)
    {
    /*-----------------------------------------------------------------------------------
    A dimension that runs neither horizontal nor nearly vertical is inclined.  Text width
    of such a dimension should be projected to dimension line and be checked for fitting.
    A vertical dimension has a 15 criterion.
    -----------------------------------------------------------------------------------*/
    if (isInclinedDimensionWithHorizontalText(pAdimProcess, pCheck) && !isDimlineNearlyVertical(pCheck))
        {
        /* update intersecting length with projected full text width */
        *pCheckSize = BentleyApi::adim_projectTextSize(pTextSize, pCheck, pAdimProcess->GetDimElementCP()->geom.textMargin);

        return  true;
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::adim_needExtraTextMargin
(
AdimProcess const   *pAdimProcess
)
    {
    /*-----------------------------------------------------------------------------------
    An extra text margin is added to preserve legacy behavior.  With new fit options,
    this extra margin is not needed.
    -----------------------------------------------------------------------------------*/
    if (pAdimProcess->flags.ignoreMinLeader || pAdimProcess->flags.tightFitTextAbove)
        return  false;

    switch (pAdimProcess->flags.fitOption)
        {
        case DIMSTYLE_VALUE_FitOption_KeepTextInside:
        case DIMSTYLE_VALUE_FitOption_MoveTextFirst:
        case DIMSTYLE_VALUE_FitOption_MoveBoth:
        case DIMSTYLE_VALUE_FitOption_MoveEither:
            return  false;
        }

    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  03/02
+---------------+---------------+---------------+---------------+---------------+------*/
static void    getHorizontalTextPosition
(
AdimProcess*          ep,
DPoint3d*             pTextOrg,
bool                  *pIsBallNChain,
UInt16                *pChainType,
DVec3d*               pTextDir,
double*               pdOffsetAtLeftEdge,
DPoint2dP             pTextSize,
DPoint3d const* const pOrigin,
DVec3d   const* const pDirection,
DPoint3d const* const pCheck,
double   const        dLineLength,
double   const        dOutsideMinleader,
double   const        dCharWidth,
DPoint2dR             offset,
int                   textJust,
bool                  pushRight
)
    {
    ElementHandleCR dimElement = ep->GetElemHandleCR();
    DimensionElm const* pDim = &dimElement.GetElementCP()->ToDimensionElm();
    double      dTextMargin = pDim->geom.textMargin;
    double      dMinLeader  = dOutsideMinleader;
    /* keep legacy behavior: extraMargin=dTextMargin */
    double      extraMargin = adim_needExtraTextMargin(ep) ? dTextMargin : 0;
    double      intersect_len;
    bool        multiline, bFitInclinedTextBox;

    mdlDim_extensionsGetFitInclinedTextBox (&bFitInclinedTextBox, ep->pOverrides, false);

    /*-----------------------------------------------------------------------------------
    Get effective text size to fit between witness lines.  It may be projected text width
    or length of text box intersected by dimension line.
    -----------------------------------------------------------------------------------*/
    if (ep->flags.allDontFit && ep->dProjectedTextWidth > 0.0)
        {
        /* use the projected full text width */
        intersect_len = ep->dProjectedTextWidth;
        }
    else
        {
        // use similar triangles to find extent of line crossing text box (See Note(1))
        intersect_len = BentleyApi::adim_getIntersectLength (pCheck, pTextSize, dTextMargin * 2.0);
        }

    // -- text direction
    pTextDir->y = pTextDir->z = 0.0;
    pTextDir->x = 1.0;
    ep->vuMatrix.MultiplyTranspose(*pTextDir);

    ep->flags.textNotFit = dLineLength < intersect_len + 2.0 * extraMargin;
    

    BentleyApi::adim_calcTextOffset (offset, textJust, pushRight, dLineLength, intersect_len, dCharWidth, dMinLeader, pDim->flag.termMode, pTextSize, pTextDir, pDirection, ep);

    *pIsBallNChain = BentleyApi::adim_checkForLeader (pChainType, ep, offset.y, pTextSize);

    multiline = ep->flags.textBlockPopulated;
    if (*pIsBallNChain)
        {
        // ----------------------------------------------
        // Compute text location:
        // - 1st along dimension line
        // - 2nd perpendicular to dimension line
        // - 3rd margin in text direction
        // ----------------------------------------------
        bsiDPoint3d_addScaledDVec3d (pTextOrg, pOrigin,  pDirection, offset.x);
        computeLeaderOffset (pTextOrg, pTextOrg, ep, offset.y);
        bsiDPoint3d_addScaledDVec3d (pTextOrg, pTextOrg, pTextDir, dTextMargin);

        /* Apply multiline's textnode specific shift */
        if (multiline)
            {
            switch  (adim_getDimTextJustificationHorizontal (ep))
                {
                case TextElementJustification::CenterMiddle:
                    bsiDPoint3d_addScaledDVec3d (pTextOrg, pTextOrg, pTextDir, pTextSize->x * 0.5);
                    break;

                case TextElementJustification::RightMiddle:
                    bsiDPoint3d_addScaledDVec3d (pTextOrg, pTextOrg, pTextDir, pTextSize->x);
                    break;
                }
            }

        *pdOffsetAtLeftEdge = offset.x;
        }
    else
        {
        if (adim_getDimTextJustificationHorizontal(ep) == TextElementJustification::CenterMiddle && !multiline)
            {
            /*---------------------------------------------------------------------------
            We could have replaced below "if (!muliline)" general code with this logic
            plus a conditional offset assignment for CC vs LC justification. But at this
            late build for v8.5 as well as a porting back to v8.1 priority build such
            change can take some risk.  We leave the right of way change till post 8.5.
            ---------------------------------------------------------------------------*/
            bsiDPoint3d_addScaledDVec3d (pTextOrg, pOrigin, pDirection, offset.x);
            bsiDPoint3d_addScaledDVec3d (pTextOrg, pTextOrg, pTextDir, -pTextSize->x * 0.5);
            *pdOffsetAtLeftEdge = offset.x - intersect_len * 0.5;
            }
        else if (! multiline)
            {
            DVec3d      tmp;
            double      dSign       = 1.0;
            double      dJustOffset = 0.0;

            // compute default origin and center text
            bsiDPoint3d_addScaledDVec3d (pTextOrg, pOrigin, pDirection, offset.x + intersect_len * 0.5);
            bsiDPoint3d_addScaledDVec3d (pTextOrg, pTextOrg, pTextDir, -pTextSize->x * 0.5);
            *pdOffsetAtLeftEdge = offset.x;

            // --------------------------------------------------------------------
            // compute origin that matches to current justification
            // We need to know text reversal status in order to compute justification
            // shift correctly.
            // This sign business is due to legacy support - see note in adim_getOffsetAtLeftEdge
            // --------------------------------------------------------------------
            if (pCheck->x <= -mgds_fc_epsilon || fabs (pCheck->y + 1.0) < 0.0001)
                dSign = -1.0;

            // --------------------------------------------------------------------
            // TODO: Test dimension line
            //  when dimension line is inclining in about 20+-5 deg angle intersect_len
            //  or something is in wrong value causing gap between line and text.
            //  the gap is about twice the margin.
            // --------------------------------------------------------------------

            bsiDVec3d_scale (&tmp, pTextDir, BentleyApi::adim_computeLeftEdgeOffset (ep, 0.0, pTextSize->x + 2.0 * dTextMargin));
            dJustOffset = bsiDVec3d_dotProduct (pDirection, &tmp);
            bsiDPoint3d_addScaledDVec3d (pTextOrg, pTextOrg, pDirection, dSign * dJustOffset);

            *pdOffsetAtLeftEdge += dSign * dJustOffset;
            }
        else
            {
            // compute default origin
            if (DIMTEXT_OFFSET == textJust)
                {
                /* text offset is right at text justification point - an input value */
                bsiDPoint3d_addScaledDVec3d (pTextOrg, pOrigin, pDirection, offset.x);
                *pdOffsetAtLeftEdge  = offset.x;
                *pdOffsetAtLeftEdge -= intersect_len * 0.5;
                }
            else
                {
                /* text offset is at the intersection of dimension line with text box - a calculated value */
                bsiDPoint3d_addScaledDVec3d (pTextOrg, pOrigin, pDirection, offset.x + intersect_len * 0.5);
                *pdOffsetAtLeftEdge  = offset.x;
                }

            // fix text "centroid" on dimension line
            switch  (adim_getDimTextJustificationHorizontal (ep))
                {
                case TextElementJustification::LeftMiddle:
                    bsiDPoint3d_addScaledDVec3d (pTextOrg, pTextOrg, pTextDir, -pTextSize->x * 0.5);
                    break;

                case TextElementJustification::RightMiddle:
                    bsiDPoint3d_addScaledDVec3d (pTextOrg, pTextOrg, pTextDir, pTextSize->x * 0.5);
                    break;
                }
            }
        }

    /*-------------------------------------------------------------
        Determine nesessary stack offset and store in ep for
        use in future calls
    -------------------------------------------------------------*/
    ep->stackHeight = getStackOffset (pDim, pTextSize, &ep->strDat, pCheck);
    pTextSize->x    = intersect_len;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  04/02
+---------------+---------------+---------------+---------------+---------------+------*/
static void    computeSingleLineLocationAndOffset
(
DPoint3d*                location,
double*                  locationOffset,
AdimProcess const* const ep,
DPoint3d    const* const origin,
DVec3d      const* const direction,
DPoint2dCP               textSize,
bool               const reversed,
double const             offsetx
)
    {
    double          addOffset, shift;
    double const    margin = ep->GetDimElementCP()->geom.textMargin;

    // this is the true text origin on dimension line
    addOffset = reversed ? textSize->x - margin : margin;

    // compute justification shift
    shift     = BentleyApi::adim_computeLeftEdgeOffset (ep, 0.0, textSize->x);

    // compute origin that matches to current justification
    bsiDPoint3d_addScaledDVec3d (location, origin, direction, offsetx + addOffset + shift);

    // left edge at
    *locationOffset = offsetx + shift;
    }

/*---------------------------------------------------------------------------------**//**
* @param        location        is text origin location with margin shift
* @param        locationOffset  is text box left edge for rendering dimension line
* @bsimethod                                                    petri.niiranen  04/02
+---------------+---------------+---------------+---------------+---------------+------*/
static void    computeMultilineLocationAndOffset
(
DPoint3d*                location,
double*                  locationOffset,
AdimProcess const* const ep,
DPoint3d    const* const origin,
DVec3d      const* const direction,
DPoint2dCP               textSize,
bool               const reversed,
bool               const bIsBallNChain,
int                const chainType,
double             const offsetx,
int                      textJust
)
    {
    double    margin = ep->GetDimElementCP()->geom.textMargin;
    double    addOffset = margin;

    *locationOffset = offsetx;

    // Note that this function first looks for the text justification
    // in the multiline linkage which will not be disturbed by the
    // dimension stroker.
    switch  (adim_getDimTextJustificationHorizontal (ep))
        {
        case TextElementJustification::RightMiddle:
            {
            if (! reversed)
                {
                addOffset = (textSize->x - margin);
                }

            break;
            }

        case TextElementJustification::CenterMiddle:
            {
            if (bIsBallNChain && chainType == DIMSTYLE_VALUE_BallAndChain_ChainType_None)
                {
                addOffset = - (textSize->x/2.0);
                if (reversed)
                     addOffset *= -1.0;
                }
            else if (DIMTEXT_OFFSET == textJust)
                {
                /* when text offset is right at text justification point - an input value */
                addOffset = 0.0;
                }
            else
                {
                /* when text offset is at the intersection of dimension line with text box - a calculated value */
                addOffset = textSize->x/2.0;
                }
            break;
            }

        default: // Left Just
            {
            if (reversed)
                {
                addOffset = (textSize->x - margin);
                }
            break;
            }
        }

    // default text location w/ margin
    bsiDPoint3d_addScaledDVec3d (location, origin, direction, offsetx + addOffset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  03/02
+---------------+---------------+---------------+---------------+---------------+------*/
static void    getAlignedTextPosition
(
AdimProcess*            ep,
DPoint3d*               pTextOrg,
bool                    *pIsBallNChain,
UInt16                  *pChainType,
DVec3d*                 pTextDir,
double*                 pLeftOffset,
DPoint2dP               pTextSize,
DPoint3d const* const   pOrigin,
DVec3d   const* const   pDirection,
DPoint3d const* const   pCheck,
double   const          dLineLength,
double   const          dOutsideMinleader,
double   const          dCharWidth,
DPoint2d&               offset,
int                     textJust,
bool                    pushRight
)
    {
    double  const dTextMargin = ep->GetDimElementCP()->geom.textMargin;
    double        dMinLeader  = dOutsideMinleader;
    /* keep legacy behavior: extraMargin=dTextMargin */
    double        extraMargin = adim_needExtraTextMargin(ep) ? dTextMargin : 0;
    bool    const multiline   = ep->flags.textBlockPopulated;
    bool          textReversed;

    textReversed = (pCheck->x <= -mgds_fc_epsilon || fabs (pCheck->y + 1.0) < 0.0001); 
    if (textReversed)
        bsiDVec3d_scale (pTextDir, pDirection, -1.0);
    else
        *pTextDir    = *pDirection;

    pTextSize->x += dTextMargin * 2.0;

    ep->flags.textNotFit = dLineLength < pTextSize->x + 2.0 * extraMargin;

    BentleyApi::adim_calcTextOffset (offset, textJust, pushRight, dLineLength, pTextSize->x, dCharWidth, dMinLeader, ep->GetDimElementCP()->flag.termMode, pTextSize, pTextDir, pDirection, ep);

    *pIsBallNChain = BentleyApi::adim_checkForLeader (pChainType, ep, offset.y, pTextSize);

    if (! multiline)
        {
        computeSingleLineLocationAndOffset (pTextOrg, pLeftOffset, ep, pOrigin, pDirection, pTextSize, textReversed, offset.x);
        }
    else
        {
        computeMultilineLocationAndOffset (pTextOrg, pLeftOffset, ep, pOrigin, pDirection, pTextSize, textReversed, *pIsBallNChain, *pChainType, offset.x, textJust);
        }

    if (*pIsBallNChain)
        {
        if (textReversed && !multiline && *pChainType != DIMSTYLE_VALUE_BallAndChain_ChainType_None)
            bsiDPoint3d_addScaledDVec3d (pTextOrg, pTextOrg, pTextDir, pTextSize->x);

        computeLeaderOffset (pTextOrg, pTextOrg, ep, offset.y);
        }

    ep->stackHeight = getStackOffset (ep->GetDimElementCP(), pTextSize, &ep->strDat, pCheck);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        needJoinerDimLineForBnc
(
int                 chainType,
AdimProcess const   *pAdimProcess
)
    {
    /*-----------------------------------------------------------------------------------
    A joiner dim line is not needed when terminators are pushed outside, and auto ball &
    chain without a leader is in effect.
    -----------------------------------------------------------------------------------*/
    if (0 == pAdimProcess->GetDimElementCP()->flag.termMode && pAdimProcess->flags.pushTextOutside &&
        DIMSTYLE_VALUE_BallAndChain_ChainType_None == chainType)
        {
        DimStyleProp_BallAndChain_Mode  bncMode = DIMSTYLE_VALUE_BallAndChain_Mode_None;

        mdlDim_getBallNChainMode (&bncMode, pAdimProcess->GetElemHandleCR());

        return  DIMSTYLE_VALUE_BallAndChain_Mode_Auto != bncMode;
        }

    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        isTextPlacedOutside
(
AdimProcess const   *pAdimProcess,
double              effectiveWidth,
double              dimLineLength,
DPoint2dCR          offset,
int                 textJust
)
    {
    /*-----------------------------------------------------------------------------------
    Check to see if text center is manually placed outside of witness lines.
    -----------------------------------------------------------------------------------*/
    if (DIMTEXT_OFFSET == textJust)
        {
        double  halfWidth = 0.5 * effectiveWidth;
        double  halfHeight = 0.5 * pAdimProcess->GetDimElementCP()->text.height + pAdimProcess->GetDimElementCP()->geom.textLift;
        double  centerOffset = 0;

        if (fabs(offset.y) > halfHeight)
            return  true;

        switch (adim_getDimTextJustificationHorizontal(pAdimProcess))
            {
            case TextElementJustification::RightMiddle:
                centerOffset = offset.x - halfWidth;
                break;
            case TextElementJustification::CenterMiddle:
                centerOffset = offset.x;
                break;
            default: // left justified
                centerOffset = offset.x + halfWidth;
                break;
            }

        return  centerOffset < 0.0 || centerOffset > dimLineLength;
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        areAllBetweenExtensionLines
(
double              *pEffectiveWidth,
AdimProcess         *pAdimProcess,
DPoint2dP           pTextSize,
double              minLeader,
double              dimLineLength,
DPoint3d            *pCheck
)
    {
    /*-----------------------------------------------------------------------------------
    Test to see if text, margin and terminators can all fit between witness lines.
    -----------------------------------------------------------------------------------*/
    bool            bFit = true, bFitProjectedTextBox = false;
    bool            bAutoFit = pAdimProcess->GetDimElementCP()->flag.termMode == 0;
    double          checkSize;

    pAdimProcess->flags.tightFitTextAbove = isTextAboveWithNoMinLeader (pAdimProcess);
    // do not use projected text width unless it is a horizontal text in an inclined dimension
    pAdimProcess->dProjectedTextWidth = -1.0;
    // will find if text to be pushed outside before calling BentleyApi::adim_calcTextOffset
    pAdimProcess->flags.pushTextOutside = false;
    pAdimProcess->flags.textNotFit = false;

    mdlDim_extensionsGetFitInclinedTextBox (&bFitProjectedTextBox, pAdimProcess->pOverrides, false);

    // get text margined width which will be used later on to decide what action to take for text & terms.
    *pEffectiveWidth = pTextSize->x + 2 * pAdimProcess->GetDimElementCP()->geom.textMargin;

    if (bAutoFit && bFitProjectedTextBox && isInclinedDimensionWithHorizontalText(pAdimProcess, pCheck))
        {
        /* inclined dimension with horizontal text: find text cut size: */
        *pEffectiveWidth = BentleyApi::adim_getIntersectLength (pCheck, pTextSize, 2 * pAdimProcess->GetDimElementCP()->geom.textMargin);

        bFit = *pEffectiveWidth + 2 * minLeader < dimLineLength;

        /* the projected full text width may also be too big to fit between witness lines: */
        if (checkInclinedDimensionTextWidth(pEffectiveWidth, pAdimProcess, pTextSize, pCheck))
            {
            pAdimProcess->dProjectedTextWidth = *pEffectiveWidth;

            if (*pEffectiveWidth >= dimLineLength)
                bFit = false;
            }

        return  bFit;
        }
    else if (bAutoFit && pAdimProcess->flags.tightFitTextAbove)
        {
        /* text (above or aside) does not cross dimension line: fit the bigger one, text or terms: */
        checkSize = 2 * pAdimProcess->GetDimElementCP()->geom.termWidth;

        if (checkSize < pTextSize->x)
            checkSize = pTextSize->x;

        bFit = checkSize + 2 * pAdimProcess->GetDimElementCP()->geom.textMargin < dimLineLength;

        return  bFit;
        }
    else if (bAutoFit && pAdimProcess->flags.ignoreMinLeader)
        {
        /* the passed in minLeader is the effective min leader */
        if (pAdimProcess->GetDimElementCP()->flag.horizontal)
            *pEffectiveWidth = BentleyApi::adim_getIntersectLength (pCheck, pTextSize, 2 * pAdimProcess->GetDimElementCP()->geom.textMargin);

        checkSize = *pEffectiveWidth + 2 * minLeader;
        }
    else
        {
        /* preserve legacy behavior in getHorizontalTextPosition vs getAlignedTextPosition */
        if (pAdimProcess->GetDimElementCP()->flag.horizontal)
            {
            checkSize = BentleyApi::adim_getIntersectLength(pCheck, pTextSize, 2 * pAdimProcess->GetDimElementCP()->geom.textMargin) + 2 * minLeader;

            *pEffectiveWidth = checkSize;
            }
        else
            {
            checkSize = pTextSize->x + 2 * (pAdimProcess->GetDimElementCP()->geom.textMargin + minLeader);
            }
        }
    /* all other cases fall through to default check method */

    bFit = pAdimProcess->GetDimElementCP()->flag.termMode == 1 || pAdimProcess->GetDimElementCP()->flag.termMode == 2 ||
           (pAdimProcess->GetDimElementCP()->flag.termMode == 0 && checkSize < dimLineLength);

    return  bFit;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool         BentleyApi::adim_areTerminatorsBetweenExtensionLines
(
AdimProcess         *pAdimProcess,
double              effectiveWidth,
double              fitMargin,
double              dimLineLength,
double              insideMinLeader,
DPoint2dCR          offset,
int                 textJust
)
    {
    /*-----------------------------------------------------------------------------------
    At this stage we know that all components don't fit.  Now we are to determine
    whether terminators will be the next fit choice.

    When dim line runs through both terminators, treat it as if there are no terminators
    to effectively make terminators to fit into a much smaller space.  A typical example
    is DWG's oblique terminator (dim line thru) vs. blank arrowhead (no line thru).
    -----------------------------------------------------------------------------------*/
    double          termsWidth = 2 * insideMinLeader;
    bool            bTermsFit = termsWidth <= dimLineLength;
    bool            bTextFits = effectiveWidth + fitMargin < dimLineLength;

    /*-----------------------------------------------------------------------------------
    Keep terminators in between the witness lines on following conditions:

    1) for options "move text first", "move either", and "keep text inside", and when text
        is manually moved outside. This rule makes dimension to behave effectively the
        same as if the text fits inside witness lines.
    2) for fit option "move text first", and when text is at its default position.
    3) for fit option "move either", and when text does not fit;
    4) for fit option "move either", and when both fit but terminators make up the bigger
        size comparing to text width.  That is to apply the best fit rule: keep the bigger
        and move the smaller.
    -----------------------------------------------------------------------------------*/
    if (pAdimProcess->flags.allDontFit && bTermsFit)
        {
        bool    bTextPlacedOutside = isTextPlacedOutside (pAdimProcess, effectiveWidth, dimLineLength, offset, textJust);

        if (bTextPlacedOutside &&
            (DIMSTYLE_VALUE_FitOption_MoveTextFirst == pAdimProcess->flags.fitOption ||
             DIMSTYLE_VALUE_FitOption_KeepTextInside == pAdimProcess->flags.fitOption ||
             DIMSTYLE_VALUE_FitOption_MoveEither == pAdimProcess->flags.fitOption))
            {
            pAdimProcess->flags.allDontFit = 0;

            return  true;
            }

        if (DIMSTYLE_VALUE_FitOption_MoveTextFirst == pAdimProcess->flags.fitOption)
            return  DIMTEXT_OFFSET != textJust;

        if (DIMSTYLE_VALUE_FitOption_MoveEither == pAdimProcess->flags.fitOption)
            {
            /*---------------------------------------------------------------------------
            For best fit, when text does not fit, terminators are the natural choice to
            fit and kept inside.  When text also fits, apply the best fitting rule here:
            only if terminator is the bigger of the two we keep term inside.
            ---------------------------------------------------------------------------*/
            if (!bTextFits || termsWidth > effectiveWidth)
                return  true;
            }
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/06
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    areTerminatorsSuppressable
(
double              lineLength,
AdimProcess const   *pAdimProcess,
double const        offsetx,
int                 textJust
)
    {
    // terminators should not be suppressed with these fit options:
    if (DIMSTYLE_VALUE_FitOption_MoveTextFirst == pAdimProcess->flags.fitOption ||
        DIMSTYLE_VALUE_FitOption_MoveBoth == pAdimProcess->flags.fitOption ||
        DIMSTYLE_VALUE_FitOption_KeepTermsOutside == pAdimProcess->flags.fitOption)
        return  false;

    // terminator suppression is not allowed under these conditions:
    if (DIMTEXT_OFFSET == textJust || pAdimProcess->flags.fitTermsInside)
        return  false;

    // now, check for scenarios that do allow suppression to take place:
    if (DIMSTYLE_VALUE_FitOption_KeepTextInside == pAdimProcess->flags.fitOption ||
        (offsetx > 0.0 && offsetx < lineLength))
        return  true;

    return  false;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name      adim_generateDimension                                      |
|                                                                       |
| author    JVB                                         1/90            |
|                                                                       |
+----------------------------------------------------------------------*/
int BentleyApi::adim_generateDimension
(
double         *textLift,
AdimProcess    *ep,          /* => Function used to process elements   */
DPoint3d       *start,       /* => Start point of dimension line       */
DPoint3d       *end,         /* => End point of dimension line         */
DPoint3d       *currpoint,   /* => Current data point                  */
double          dimLength,   /* => Length of dimension line            */
DimText const  *dimTextCP,     /* => Dimension text location etc.        */
int            leftTerm,     /* => Left terminator index               */
int            rightTerm     /* => Right terminator index              */
)
    {
    DPoint2d        textSize;
    DVec3d          check, direction, txtdir;
    DPoint3d        origin, endpt, witBase, midpt, tmp1, tmp2, rtxtorg;
    double          charWidth, line_len, textWidth, effectiveWidth;
    double          margin, textMargin, textMar2, insideMinLeader, outsideMinLeader;
    UInt16          chainType = DIMSTYLE_VALUE_BallAndChain_ChainType_None;
    int             status;
    bool            ballAndChain = false;
    bool            bLeftSuppress = false, bRightSuppress = false;
    bool            bDimlineThruTerm = adim_isDimlineThruEitherTerm(ep);
    double          dOffsetAtLeftEdge;
    DimStyleProp_Text_Location textLocation = DIMSTYLE_VALUE_Text_Location_Inline;
    DimStyleProp_Text_Location prevTextLocation = DIMSTYLE_VALUE_Text_Location_Inline;
    DimStyleProp_FitOptions fitOption = DIMSTYLE_VALUE_FitOption_MoveTermsFirst;

    ElementHandleCR dimElement = ep->GetElemHandleCR();
    DimensionElm const* pDim = &dimElement.GetElementCP()->ToDimensionElm();
    mdlDim_overridesGetTextLocation (&prevTextLocation, ep->pOverrides, ep->flags.embed);
    
    /* get fit options */
    mdlDim_getFitOption (&fitOption, dimElement);
    ep->flags.fitOption = fitOption;

    witBase = *currpoint;

    endpt  = tmp1 = *end;
    origin = tmp2 = *start;

    /*-------------------------------------------------------------------
    Measure
    -------------------------------------------------------------------*/
    dimLength *= pDim->GetScale();
    line_len   = bsiDPoint3d_distance (start, end);
    adim_getLengthStrings (ep, dimLength, pDim->frmt.primaryAccuracy);

    /*-------------------------------------------------------------------
    Text size & related params
    -------------------------------------------------------------------*/
    adim_getTextSize (&textSize, ep, &ep->strDat, ADIM_TEXTSIZE_Exact);
    if (pDim->flag.dual)
        textSize.y += pDim->geom.textLift * 2.0;

    textWidth  = textSize.x;
    charWidth  = ep->strDat.charWidth;
    textMargin = pDim->geom.textMargin;
    textMar2   = textMargin * 2.0;

    ep->flags.ignoreMinLeader = mdlDim_isMinLeaderIgnored (ep->pOverrides, false);

    /* get effective inside minimum leader */
    BentleyApi::adim_getEffectiveMinLeaders (&insideMinLeader, NULL, ep);

    DPoint2d offset;
    offset.x = dimTextCP->offset;
    offset.y = dimTextCP->offsetY;
    
    int textJust = dimTextCP->flags.b.just;
    bool pushRight = dimTextCP->flags.b.pushTextRight;
    /*-------------------------------------------------------------------
    If vertical text is required - generate and return
    -------------------------------------------------------------------*/
    if (pDim->tmpl.vertical_text ||  (pDim->tmpl.nofit_vertical &&
        textSize.x + insideMinLeader > line_len))
        {
        margin = pDim->geom.witExtend + charWidth * 0.5;
        return  generateVerticalDimension (ep, start, end, &witBase, textWidth, margin, leftTerm, rightTerm, offset, textJust);
        }

    /*-------------------------------------------------------------------
    Text is not vertical
    -------------------------------------------------------------------*/
    if (LegacyMath::DEqual (direction.NormalizedDifference (*end, *start), 0.0))
        {
        ep->rMatrix.GetColumn(direction,  0);
#if defined (NEEDS_WORK)
        dimText->offset = leaderLen;
#endif
        }

    /* check is the dimension line direction in view coordinates */
    check = direction;
    ep->vuMatrix.Multiply(check);

    /*-----------------------------------------------------------------------------------
    Apply fit options:

    There are 3 calculated variables to tell whether a dimension fits:

    allDontFit      - Text and margins with or without min leaders do not fit.
    fitTermsInside  - Keep terminators inside even if allDontFit=true
    pushTextOutside - Push text outside if allDontFit=true

    These three variables are sufficient to tell how to place dimension lines, text, and
    terminators under varies conditions.

    The rule for "best fit" is to first try to fit all components (i.e. text + margins +
    terminators).  If all don't fit, then try to fit the bigger component (either text or
    terminators), then followed by fitting the smaller one.  Eventually as witness lines
    move nearer, none will fit and all get moved outside.
    -----------------------------------------------------------------------------------*/
    ep->flags.allDontFit = !areAllBetweenExtensionLines (&effectiveWidth, ep, &textSize, insideMinLeader, line_len, &check);
    ep->flags.fitTermsInside = adim_areTerminatorsBetweenExtensionLines (ep, effectiveWidth, 0.0, line_len, insideMinLeader, offset, textJust);

    /* get effective outside minimum leader which depends on terminator being inside vs outside */
    BentleyApi::adim_getEffectiveMinLeaders (NULL, &outsideMinLeader, ep);

    /*-------------------------------------------------------------------
    Compute text origin and direction
    -------------------------------------------------------------------*/
    if (pDim->flag.horizontal)
        {
        getHorizontalTextPosition (ep, &rtxtorg, &ballAndChain, &chainType, &txtdir, &dOffsetAtLeftEdge, &textSize,
                                   &origin, &direction, &check, line_len, outsideMinLeader, charWidth, offset, textJust, pushRight);

        textWidth *= fabs (bsiDVec3d_dotProduct (&direction, &txtdir));
        }
    else  /* Above or In-line text */
        {
        getAlignedTextPosition (ep, &rtxtorg, &ballAndChain, &chainType, &txtdir, &dOffsetAtLeftEdge, &textSize,
                                &origin, &direction, &check, line_len, outsideMinLeader, charWidth, offset, textJust, pushRight);
        }

    /*-------------------------------------------------------------------
    Dimension with leader aka ball and chain
    -------------------------------------------------------------------*/
    if (ballAndChain)
        {
        // Compute the midpoint of the dimline which will be used as the leader origin
        bsiDPoint3d_addScaledDVec3d (&midpt, &origin, &direction, line_len / 2.0);

        // When ball and chain is used AND there is a physical leader existing,
        // autocad gives text->offset corresponding to the appropriate left-center
        // or right-center point. There is no extra text-justification shift needed,
        // so clear the override.
        // Note that for multiline text, we will not clear the text-justification
        // in the multiline linkage.
        if (chainType != DIMSTYLE_VALUE_BallAndChain_ChainType_None)
            mdlDim_overridesClearSegmentPropertyBit (ep->pOverrides, ADIM_GETSEG (ep->partName), SEGMENT_Override_TextJustification);
        }
    else
        {
        offset.y = 0.0;
        }

    //  If text is rotated relative to the dimension, then don't do any of the "upward"
    //  offsetting that is normally associated with non-embedded text. For now, do this
    //  flattening business only for Above location so the existing dimensions remain
    //  unaffected. Don't think we should be doing this for Outside and Top-Left,
    //  especially since DWG dimensions don't do this.
    mdlDim_overridesGetTextLocation (&textLocation, ep->pOverrides, ep->flags.embed);
    
    if (DIMSTYLE_VALUE_Text_Location_Above == textLocation)
        {
        double zRotation;
        if (!LegacyMath::Vec::AreParallel (&txtdir, &direction)
            || mdlDim_overridesGetSegmentTextRotation (&zRotation, ep->pOverrides, ADIM_GETSEG (ep->partName), 0.0)
                && fabs (fmod (zRotation, msGeomConst_2pi)) != 0)
            {
            // Note : Changing Above to Inline only requires the embed flag to change since uTextLocation is 0
            // in both cases (refer midimstyle.h).
            ep->flags.embed = true;
            }
        }

    /*-------------------------------------------------------------------
    Generate text and dimension line(s)
    -------------------------------------------------------------------*/
    clearTextBox (ep);

    bLeftSuppress  = mdlDim_overridesGetSegmentFlagSuppressLeftDimLine  (NULL, ep->pOverrides, ADIM_GETSEG (ep->partName), false);
    bRightSuppress = mdlDim_overridesGetSegmentFlagSuppressRightDimLine (NULL, ep->pOverrides, ADIM_GETSEG (ep->partName), false);

    if (!ep->flags.allDontFit)
        {
        /*---------------------------------------------------------------
        Dimension text will fit within the dimension line
        ---------------------------------------------------------------*/
        if (status = adim_generateLinearDimensionWithOffset (ep, ballAndChain ? &midpt : NULL, &rtxtorg, &txtdir, offset.y))
            {
            return  status;
            }

        if (ballAndChain)
            {
            ADIM_SETNAME (ep->partName, ADTYPE_DIMLINE, ADSUB_NONE);
            if (status = adim_generateDimLine (ep, start, end, DIM_MATERIAL_DimLine, leftTerm, rightTerm,
                                               TRIM_BOTH, true, termsAreInside (true, ep), true))
                return (status);
            }
        else
            {
            if (isOnLeftSide (ep, textWidth, 0.0, offset.x, textJust))
                {
                int         trimCode         = TRIM_BOTH;
                bool        generateLeftLine = true;

                /*-------------------------------------------------------
                Text was dragged outside left witness line
                -------------------------------------------------------*/
                tmp2 = origin;

                mdlDim_overridesGetTextLocation (&textLocation, ep->pOverrides, ep->flags.embed);
                if (DIMSTYLE_VALUE_Text_Location_Inline == textLocation)
                    {
                    double          trimDistance, rightEdge;
                    DimTermBlock*   pTermBlock = (DimTermBlock*) mdlDim_getOptionBlock (dimElement, ADBLK_TERMINATOR, NULL);

                    bsiDPoint3d_addScaledDVec3d (&tmp1, &origin, &direction, dOffsetAtLeftEdge + textSize.x);

                    // If text is not completely on left side, so that some portions are left between witness lines
                    // don't generate left portion of dimension line because it would run thru terminator and text.
                    // Select the greater width (text or terminator trim) for trimming
                    trimDistance = adim_getTrimDistance (ep, leftTerm, pTermBlock, termsAreInside (true, ep));
                    rightEdge    = dOffsetAtLeftEdge + textSize.x;

                    if (rightEdge > 0.0)
                        generateLeftLine = false;

                    if (rightEdge > trimDistance)
                        {
                        tmp2 = tmp1;
                        trimCode = TRIM_RIGHT;
                        }
                    }
                else
                    bsiDPoint3d_addScaledDVec3d (&tmp1, &origin, &direction, dOffsetAtLeftEdge + textMargin);

                ADIM_SETNAME (ep->partName, ADTYPE_DIMLINE, ADSUB_NONE);

                if (generateLeftLine)
                    {
                    if (SUCCESS != (status = adim_generateDimLine (ep, &tmp1, &tmp2, DIM_MATERIAL_DimLine, leftTerm, rightTerm, TRIM_RIGHT, false,
                                                                   termsAreInside (true, ep), true)))
                        {
                        return  status;
                        }
                    }

                if (SUCCESS != (status = adim_generateDimLine (ep, &tmp2, &endpt, DIM_MATERIAL_DimLine, leftTerm, rightTerm, trimCode, true,
                                                               termsAreInside (true, ep), true)))
                    {
                    return  status;
                    }

                tmp1 = endpt;
                tmp2 = origin;
                }
            else if (isOnRightSide (ep, textWidth, line_len, offset.x, textJust))
                {
                bool        generateRightLine = true;
                int         trimCode          = TRIM_BOTH;

                /*-------------------------------------------------------
                Text was dragged outside right witness line
                -------------------------------------------------------*/
                tmp2 = endpt;
                mdlDim_overridesGetTextLocation (&textLocation, ep->pOverrides, ep->flags.embed);
                if (DIMSTYLE_VALUE_Text_Location_Inline == textLocation)
                    {
                    double          trimDistance;
                    DimTermBlock*   pTermBlock        = (DimTermBlock*) mdlDim_getOptionBlock (dimElement, ADBLK_TERMINATOR, NULL);

                    bsiDPoint3d_addScaledDVec3d (&tmp1, &origin, &direction, dOffsetAtLeftEdge);

                    trimDistance = adim_getTrimDistance (ep, rightTerm, pTermBlock, termsAreInside (true, ep));
                    if (dOffsetAtLeftEdge < line_len)
                        generateRightLine = false;

                    if (dOffsetAtLeftEdge < (line_len - trimDistance))
                        {
                        tmp2     = tmp1;
                        trimCode = TRIM_LEFT;
                        }
                    }
                else
                    bsiDPoint3d_addScaledDVec3d (&tmp1, &origin, &direction, dOffsetAtLeftEdge + textWidth + textMargin);

                ADIM_SETNAME (ep->partName, ADTYPE_DIMLINE, ADSUB_NONE);
                if (generateRightLine)
                    {
                    if (SUCCESS != (status = adim_generateDimLine (ep, &tmp1, &tmp2, DIM_MATERIAL_DimLine, leftTerm, rightTerm, TRIM_LEFT, false,
                                                                   termsAreInside (true, ep), false)))
                        {
                        return  status;
                        }
                    }

                if (SUCCESS != (status = adim_generateDimLine (ep, &tmp2, &origin, DIM_MATERIAL_DimLine, leftTerm, rightTerm, trimCode, true,
                                                               termsAreInside (true, ep), false)))
                    {
                    return  status;
                    }

                tmp1 = endpt;
                tmp2 = origin;
                }
            else
                {
                /*-------------------------------------------------------
                Standard text between witness lines
                -------------------------------------------------------*/
                if ((SUCCESS == intersectDimensionLineWithText (&tmp1, &tmp2, &origin, &endpt, ep)))
                    {
                    double          trimDistance;
                    DimTermBlock*   pTermBlock = (DimTermBlock*) mdlDim_getOptionBlock (dimElement, ADBLK_TERMINATOR, NULL);

                    /*-------------------------------------------------------
                    An intersection was found
                    -------------------------------------------------------*/
                    // check interference with terminators
                    trimDistance = adim_getTrimDistance (ep, leftTerm, pTermBlock, termsAreInside (true, ep));
                    if (dOffsetAtLeftEdge < trimDistance || bsiDPoint3d_distance(&tmp1, &origin) < 1.0)
                        bLeftSuppress = true;

                    if (dOffsetAtLeftEdge + textSize.x > line_len - trimDistance || bsiDPoint3d_distance(&tmp2, &endpt) < 1.0)
                        bRightSuppress = true;

                    ADIM_SETNAME (ep->partName, ADTYPE_DIMLINE, ADSUB_NONE);
                    if (!bLeftSuppress)
                        if (SUCCESS != (status = adim_generateDimLine (ep, &tmp1, &origin, DIM_MATERIAL_DimLine, leftTerm, rightTerm, TRIM_LEFT, true,
                                                                       termsAreInside (true, ep), false)))
                            {
                            return  status;
                            }

                    if (!bRightSuppress)
                        if (SUCCESS != (status = adim_generateDimLine (ep, &tmp2, &endpt, DIM_MATERIAL_DimLine, leftTerm, rightTerm, TRIM_RIGHT, true,
                                                                       termsAreInside (true, ep), true)))
                            {
                            return  status;
                            }
                    }
                else
                    {
                    /*-------------------------------------------------------
                    No intersection
                    -------------------------------------------------------*/
                    DPoint3d dStart, dEnd;

                    dStart = tmp2 = *start;
                    dEnd   = tmp1 = *end;

                    if (!bLeftSuppress && !bRightSuppress)
                        {
                        /* Neither side is suppressed */
                        ADIM_SETNAME (ep->partName, ADTYPE_DIMLINE, ADSUB_NONE);
                        if (status = adim_generateDimLine (ep, &dStart, &dEnd, DIM_MATERIAL_DimLine,
                                                           leftTerm, rightTerm, TRIM_BOTH, true,
                                                           termsAreInside (true, ep), true))
                            return (status);
                        }
                    else
                    if (!bLeftSuppress || !bRightSuppress)
                        {
                        /* Only one side is suppressed */
                        ADIM_SETNAME (ep->partName, ADTYPE_DIMLINE, ADSUB_NONE);
                        if (bLeftSuppress)
                            {
                            bsiDPoint3d_addScaledDVec3d (&dStart, &dStart, &direction, dOffsetAtLeftEdge);
                            if (status = adim_generateDimLine (ep, &dStart, &dEnd, DIM_MATERIAL_DimLine,
                                                               leftTerm, rightTerm, TRIM_RIGHT, true,
                                                               termsAreInside (true, ep), true))
                                return (status);
                            }
                        else
                        if (bRightSuppress)
                            {
                            bsiDPoint3d_addScaledDVec3d (&dEnd, &dStart, &direction, dOffsetAtLeftEdge + textWidth);
                            if (status = adim_generateDimLine (ep, &dStart, &dEnd, DIM_MATERIAL_DimLine,
                                                               leftTerm, rightTerm, TRIM_LEFT, true,
                                                               termsAreInside (true, ep), true))
                                return (status);
                            }
                        }
                    else
                        {
                        /* Both sides are suppressed -- do nothing */
                        }
                    }
                }
            }
        }
    else
        {
        /*---------------------------------------------------------------
        Dimension Text will NOT fit within dimension line.
        ---------------------------------------------------------------*/
        bool        suppressTermsOutside = false;

        /*---------------------------------------------------------------------------
        Suppress terminators & dim lines if they flip outside due to fitting options
        yet text still stays inside.
        ---------------------------------------------------------------------------*/
        if (areTerminatorsSuppressable(line_len, ep, offset.x, textJust))
            mdlDim_extensionsGetSuppressUnfitTerminatorsFlag (&suppressTermsOutside, ep->pOverrides, false);

        if (ballAndChain)
            {
            if (SUCCESS != (status = adim_generateLinearDimensionWithOffset (ep, &midpt, &rtxtorg, &txtdir, offset.y)))
                {
                return  status;
                }

            if (pDim->flag.termMode == 2 || ep->flags.fitTermsInside)
                {
                /* force terminators between witness lines */
                tmp1 = endpt;
                tmp2 = origin;

                ADIM_SETNAME (ep->partName, ADTYPE_DIMLINE, ADSUB_NONE);
                if (SUCCESS != (status = adim_generateDimLine (ep, &endpt, &origin, DIM_MATERIAL_DimLine, leftTerm, rightTerm, TRIM_BOTH, true,
                                                               termsAreInside (true, ep), false)))
                    {
                    return  status;
                    }
                }
            else
                {
                /* reverse points order to move terminators outside of witness lines */
                bsiDPoint3d_addScaledDVec3d (&tmp1, &origin, &direction, -outsideMinLeader);
                bsiDPoint3d_addScaledDVec3d (&tmp2, &origin, &direction, line_len + outsideMinLeader);

                ADIM_SETNAME (ep->partName, ADTYPE_DIMLINE, ADSUB_NONE);
                if (!suppressTermsOutside)
                    {
                    if (SUCCESS != (status = adim_generateDimLine (ep, &tmp1, &origin, DIM_MATERIAL_DimLine, leftTerm, rightTerm, TRIM_RIGHT, false,
                                                                   termsAreInside (false, ep), true)))
                        {
                        return  status;
                        }
                    }

                if (needJoinerDimLineForBnc(chainType, ep))
                    {
                    if (SUCCESS != (status = adim_generateDimLine (ep, &origin, &endpt, DIM_MATERIAL_DimLine, leftTerm, rightTerm, TRIM_BOTH, true,
                                                                   termsAreInside (false, ep), true)))
                        {
                        return  status;
                        }
                    }

                if (!suppressTermsOutside)
                    {
                    if (SUCCESS != (status = adim_generateDimLine (ep, &endpt, &tmp2, DIM_MATERIAL_DimLine, leftTerm, rightTerm, TRIM_LEFT, false,
                                                                   termsAreInside (false, ep), true)))
                        {
                        return  status;
                        }
                    }
                }
            }
        else
            {
            bool        autoLift = !pDim->flag.noAutoTextLift;
            bool        needJoinerDimLine = false;
            DPoint3d    endPtForJoiner, originForJoiner;

            if (textLift && autoLift && (offset.x < 0 || dOffsetAtLeftEdge + textSize.x > line_len))
                {
                DVec3d yDir;

                *textLift  = ep->stackHeight;
                ep->rMatrix.GetColumn(yDir,  1);
                bsiDPoint3d_addScaledDVec3d (&rtxtorg, &rtxtorg, &yDir, *textLift);
                bsiDPoint3d_addScaledDVec3d (&origin,  &origin,  &yDir, *textLift);
                bsiDPoint3d_addScaledDVec3d (&endpt,   &endpt,   &yDir, *textLift);

                if (ep->ep.segment == ep->stack.segNo)
                    ep->stack.height = *textLift;
                }

            if (SUCCESS != (status = adim_generateLinearDimensionWithOffset (ep, NULL, &rtxtorg, &txtdir, offset.y)))
                {
                return  status;
                }

            originForJoiner = origin;
            endPtForJoiner = endpt;

            if (ep->flags.fitTermsInside && 0 == pDim->flag.termMode)
                {
                /* terminators stay inside while text pushed outside */
                double  offset = ep->flags.embed ? outsideMinLeader : textWidth + textMargin + outsideMinLeader;
                bool    noLineThruLeft = adim_checkNoLineFlag(ep, leftTerm);
                bool    noLineThruRight = adim_checkNoLineFlag(ep, rightTerm);

                bLeftSuppress = bRightSuppress = true;

                needJoinerDimLine = true;

                BentleyApi::adim_setCurrentSegmentTextIsOutside (ep, textJust);

                // terminator start points
                tmp1 = endPtForJoiner;
                tmp2 = originForJoiner;

                if (noLineThruLeft || noLineThruRight)
                    {
                    DPoint3d    offsetPoint;

                    if (DIMTEXT_RIGHT == textJust || pushRight)
                        {
                        // create a line extended out towards right
                        bsiDPoint3d_addScaledDVec3d (&offsetPoint, &endPtForJoiner, &direction, offset);
                        status = adim_generateDimLine (ep, &offsetPoint, &endPtForJoiner, DIM_MATERIAL_DimLine, leftTerm, rightTerm, false, false, true, true);
                        }
                    else
                        {
                        // create a line extended out towards left
                        bsiDPoint3d_addScaledDVec3d (&offsetPoint, &originForJoiner, &direction, -offset);
                        status = adim_generateDimLine (ep, &offsetPoint, &originForJoiner, DIM_MATERIAL_DimLine, leftTerm, rightTerm, false, false, true, true);
                        }
                    // the line between the witness lines will be created later
                    }
                else
                    {
                    // extend joiner points to cover text offset
                    if (DIMTEXT_RIGHT == textJust || pushRight)
                        bsiDPoint3d_addScaledDVec3d (&endPtForJoiner, &endPtForJoiner, &direction, offset);
                    else
                        bsiDPoint3d_addScaledDVec3d (&originForJoiner, &originForJoiner, &direction, -offset);
                    }
                }
            else if (dOffsetAtLeftEdge < -textSize.x)       /* Text outside on left */
                {
                double offset;

                needJoinerDimLine = true;

                BentleyApi::adim_setCurrentSegmentTextIsOutside (ep, textJust);

                mdlDim_overridesGetTextLocation (&textLocation, ep->pOverrides, ep->flags.embed);
                offset = (DIMSTYLE_VALUE_Text_Location_Inline == textLocation) ?
                            dOffsetAtLeftEdge + textSize.x :
                            dOffsetAtLeftEdge + textMargin;

                if (DIMSTYLE_VALUE_Text_Location_Inline == textLocation)
                    {
                    double          trimDistance, rightEdge;
                    DimTermBlock*   pTermBlock = (DimTermBlock*) mdlDim_getOptionBlock (dimElement, ADBLK_TERMINATOR, NULL);

                    trimDistance = adim_getTrimDistance (ep, leftTerm, pTermBlock, termsAreInside (true, ep));
                    rightEdge    = offset;

                    if (rightEdge > -trimDistance)
                        bLeftSuppress = true;
                    }

                if (ep->flags.ignoreMinLeader && bDimlineThruTerm)
                    outsideMinLeader *= 0.5;

                bsiDPoint3d_addScaledDVec3d (&tmp1, &origin, &direction, offset);
                bsiDPoint3d_addScaledDVec3d (&tmp2, &origin, &direction, line_len + outsideMinLeader);
                }
            else if (dOffsetAtLeftEdge > line_len) /* Text outside on right */
                {
                double offset;

                needJoinerDimLine = true;

                BentleyApi::adim_setCurrentSegmentTextIsOutside (ep, textJust);

                mdlDim_overridesGetTextLocation (&textLocation, ep->pOverrides, ep->flags.embed);
                offset = (DIMSTYLE_VALUE_Text_Location_Inline == textLocation) ?
                            dOffsetAtLeftEdge :
                            dOffsetAtLeftEdge + textWidth + textMargin;

                if (DIMSTYLE_VALUE_Text_Location_Inline == textLocation)
                    {
                    double          trimDistance;
                    DimTermBlock*   pTermBlock = (DimTermBlock*) mdlDim_getOptionBlock (dimElement, ADBLK_TERMINATOR, NULL);

                    trimDistance = adim_getTrimDistance (ep, rightTerm, pTermBlock, termsAreInside (true, ep));

                    if (offset < line_len + trimDistance)
                        bRightSuppress = true;
                    }

                if (ep->flags.ignoreMinLeader && bDimlineThruTerm)
                    outsideMinLeader *= 0.5;

                bsiDPoint3d_addScaledDVec3d (&tmp1, &origin, &direction, -outsideMinLeader);
                bsiDPoint3d_addScaledDVec3d (&tmp2, &origin, &direction, offset);
                }
            else
                {
                if (ep->flags.ignoreMinLeader && bDimlineThruTerm)
                    outsideMinLeader *= 0.5;

                bsiDPoint3d_addScaledDVec3d (&tmp1, &origin, &direction, -outsideMinLeader);
                bsiDPoint3d_addScaledDVec3d (&tmp2, &origin, &direction, line_len + outsideMinLeader);
                }

            ADIM_SETNAME (ep->partName, ADTYPE_DIMLINE, ADSUB_NONE);

            // ---->|...
            if (!bLeftSuppress && !suppressTermsOutside)
                if (SUCCESS != (status = adim_generateDimLine (ep, &tmp1, &origin, DIM_MATERIAL_DimLine, leftTerm, rightTerm, TRIM_RIGHT, false,
                                                               termsAreInside (false, ep), true)))
                    {
                    return  status;
                    }

            // ...|---|...
            mdlDim_overridesGetTextLocation (&textLocation, ep->pOverrides, ep->flags.embed);
            if ((pDim->flag.joiner && (needJoinerDimLine || !(pDim->flag.horizontal || (DIMSTYLE_VALUE_Text_Location_Inline == textLocation)))) ||
                0 == pDim->flag.termMode && ep->flags.fitTermsInside)
                {
                if (SUCCESS != (status = adim_generateDimLine (ep, &originForJoiner, &endPtForJoiner, DIM_MATERIAL_DimLine, leftTerm, rightTerm, TRIM_BOTH, true,
                                                               termsAreInside (false, ep), true)))
                    {
                    return  status;
                    }
                }

            // ...|<----
            if (!bRightSuppress && !suppressTermsOutside)
                if (SUCCESS != (status = adim_generateDimLine (ep, &tmp2, &endpt, DIM_MATERIAL_DimLine, leftTerm, rightTerm, TRIM_LEFT, false,
                                                               termsAreInside (false, ep), false)))
                    {
                    return  status;
                    }

            if  (suppressTermsOutside)
                return  status;
            }
        }

    /*-----------------------------------------------------------------------------------
    Terminators
    -----------------------------------------------------------------------------------*/
    ADIM_SETNAME (ep->partName, ADTYPE_TERM_LEFT, ADSUB_NONE);
    if (SUCCESS != (status = adim_generateLineTerminator (ep, &tmp1, &origin, leftTerm, false)))
        return  status;

        // Restore the embed flag incase it was changed in this function
    if (DIMSTYLE_VALUE_Text_Location_Above == prevTextLocation && true == ep->flags.embed)
        ep->flags.embed = false;

    ADIM_SETNAME (ep->partName, ADTYPE_TERM_RIGHT, ADSUB_NONE);
    return (adim_generateLineTerminator (ep, &tmp2, &endpt, rightTerm, false));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int      adim_generateSizeDim
(
DPoint3d        *currpoint,   /* => current data point                 */
AdimProcess     *ep,          /* => process function                   */
DimText const   *dimTextCP      /* => Dimension text information         */
)
    {
    DSegment3d witline, wit1, wit2;
    DVec3d          xvec, yvec, lengthVec;
    double          dimLength;
    int             leftTerm, rightTerm;
    int             status=0;

    /*------------------------------------------------------------------
    We removed these checks to support DWG where 2 definition points can
    be identical.  This removal should not hurt any dimension behavior,
    if not improve it.
    ------------------------------------------------------------------*/
#ifdef DONT_ALLOW_COINCIDENT
    if (LegacyMath::RpntEqual (currpoint, ep->points) ||
        LegacyMath::RpntEqual (currpoint, ep->points+1))
        return (ILLEGAL_DEFINITION);
#endif

    /*--------------------------------------------------------------
      ap.points[0] - contains base of 1st witness line
      ap.points[1] - contains end of 1st witness line
      ap.points[2] - end of 2nd witness line will be calculated
      ap.points[3] - contains base of second witness line
    --------------------------------------------------------------*/
    ep->rMatrix.GetColumn(xvec,  0);
    ep->rMatrix.GetColumn(yvec,  1);

    wit1.point[0] = ep->points[0];
    wit1.point[1] = ep->points[1];
    wit2.point[0] = *currpoint;

    getRightWitness (&wit2, &wit1.point[1], &wit2.point[0], &xvec, &yvec);

    if (ep->GetDimElementCP()->flag.alignment == 3) /* Arbitrary axis dimension */
        bsiDVec3d_subtractDPoint3dDPoint3d (&lengthVec, &wit2.point[1], &wit1.point[1]);
    else
        bsiDVec3d_subtractDPoint3dDPoint3d (&lengthVec, &wit2.point[0], &wit1.point[0]);
    dimLength = fabs (bsiDVec3d_dotProduct (&lengthVec, &xvec));
    if (ep->pdDimLengths != NULL)
        ep->pdDimLengths[ADIM_GETSEG(ep->partName)] = dimLength;

#if defined (DONT_ALLOW_ZERO_LENGTH_DIMENSIONS)
    if (LegacyMath::DEqual (dimLength, 0.0))
        return (ERROR);
#endif

    adim_getEffectiveTerminators (&leftTerm, &rightTerm, ep);

    if (status = adim_generateDimension (NULL, ep, &wit1.point[1], &wit2.point[1], currpoint, dimLength, dimTextCP, leftTerm, rightTerm))
        return (status);

    if (!ep->GetDimElementCP()->GetDimTextCP(0)->flags.b.noWitness)
        {
        witline = wit1;
        offsetWitnessLine (&witline, &yvec, ep, 0.0, 0);

        ADIM_SETNAME (ep->partName, ADTYPE_EXT_LEFT, ADSUB_NONE);
        if (SUCCESS != (status = adim_generateExtensionLine (ep, &witline.point[0], &witline.point[1],
                                                             ep->GetDimElementCP()->GetDimTextCP(0)->flags.b.altSymb)))
            return (status);
        }

    if (!dimTextCP->flags.b.noWitness)
        {
        witline = wit2;
        offsetWitnessLine (&witline, &yvec, ep, 0.0, ep->ep.segment);

        ADIM_SETNAME (ep->partName, ADTYPE_EXT_RIGHT, ADSUB_NONE);
        if (SUCCESS != (status = adim_generateExtensionLine (ep, &witline.point[0], &witline.point[1],
                                                             dimTextCP->flags.b.altSymb)))
            return (status);
        }

    ep->points[2] = wit1.point[1];
    *currpoint = wit2.point[1];

    return (status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int      adim_generateSizeCont
(
DPoint3d        *currpoint,
AdimProcess     *ep,
DimText const   *dimText
)
    {
    DSegment3d wit2, witline;
    DVec3d      xvec, yvec, lengthVec;
    DPoint3d    wit1End, wit2Org, lengthOrg;
    double      dimLength, textLift;
    int         leftTerm, rightTerm;
    int         status=0;

    ep->rMatrix.GetColumn(xvec,  0);
    ep->rMatrix.GetColumn(yvec,  1);
    wit1End = ep->points[ep->GetDimElementCP()->tmpl.stacked ? 1:2];
    wit2Org = *currpoint;

    getRightWitness (&wit2, &wit1End, &wit2Org, &xvec, &yvec);

    if (ep->GetDimElementCP()->dimcmd == static_cast<byte>(DimensionType::LocateSingle) || ep->GetDimElementCP()->dimcmd == static_cast<byte>(DimensionType::LocateStacked))
        {
        DSegment3d line;
        double          dOffset = 0.0;

        // Locate dimensions with arbitrary alignment measure the distance
        // along the dimension line. Therefore we need to compute
        // the start point of dimension line. In stacked case, include
        // the height offset too. Oct 12, 2001 pn
        dOffset = ep->GetDimElementCP()->GetDimTextCP(0)->offset;

        if (ep->GetDimElementCP()->tmpl.stacked)
            {
            static double   s_dStackHeight = 0.0;

            if (1 == ADIM_GETSEG (ep->partName))
                s_dStackHeight = ep->stackHeight;
            else
                s_dStackHeight += ep->stackHeight;

            dOffset += s_dStackHeight;
            }

        line.point[0] = ep->points[0];
        bsiDPoint3d_addScaledDVec3d (&line.point[1], &line.point[0], &yvec, dOffset);
        lengthOrg = line.point[1];
        }
    else
        lengthOrg = wit1End;

    if (ep->GetDimElementCP()->flag.alignment == 3) /* Arbitrary axis dimension */
        bsiDVec3d_subtractDPoint3dDPoint3d (&lengthVec, &wit2.point[1], &lengthOrg);
    else
        bsiDVec3d_subtractDPoint3dDPoint3d (&lengthVec, &wit2.point[0], &lengthOrg);

    dimLength = fabs (bsiDVec3d_dotProduct (&lengthVec, &xvec));
    if (ep->pdDimLengths != NULL)
        ep->pdDimLengths[ADIM_GETSEG(ep->partName)] = dimLength;

#ifdef DONT_ALLOW_COINCIDENT
    if (LegacyMath::DEqual (dimLength, 0.0))
        return (ERROR);
#endif

    if (!ep->GetDimElementCP()->tmpl.stacked)
        {
        leftTerm  = ep->GetDimElementCP()->tmpl.bowtie_symbol ?
            ep->GetDimElementCP()->tmpl.bowtie_symbol : ep->GetDimElementCP()->tmpl.left_term;
        rightTerm = !ep->flags.lastSeg && ep->GetDimElementCP()->tmpl.bowtie_symbol ?
            0 : ep->GetDimElementCP()->tmpl.right_term;
        }
    else
        {
        leftTerm  = ep->GetDimElementCP()->tmpl.left_term;
        rightTerm = ep->GetDimElementCP()->tmpl.right_term;
        }

    textLift = 0.0;
    if (status = adim_generateDimension (&textLift, ep, &wit1End, &wit2.point[1],
        currpoint, dimLength, dimText, leftTerm, rightTerm))
        return (status);

    if (!dimText->flags.b.noWitness)
        {
        witline = wit2;
        offsetWitnessLine (&witline, &yvec, ep, textLift, ep->ep.segment);

        ADIM_SETNAME (ep->partName, ADTYPE_EXT_RIGHT, ADSUB_NONE);
        if (SUCCESS != (status = adim_generateExtensionLine (ep, &witline.point[0], &witline.point[1],
                                                             dimText->flags.b.altSymb)))
            return (status);
        }

    if (!ep->GetDimElementCP()->GetDimTextCP(0)->flags.b.noWitness)
        {
        if (ep->GetDimElementCP()->tmpl.stacked)
            {
            witline.point[0] = ep->points[0];
            witline.point[1] = wit1End;
            offsetWitnessLine (&witline, &yvec, ep, 0.0, 0);

            ADIM_SETNAME (ep->partName, ADTYPE_EXT_LEFT, ADSUB_NONE);
            if (SUCCESS != (status = adim_generateExtensionLine (ep, &witline.point[0], &witline.point[1],
                                                                 ep->GetDimElementCP()->GetDimTextCP(0)->flags.b.altSymb)))
                return (status);
            }
        else if (textLift != 0.0)
            {
            witline.point[0] = witline.point[1] = wit1End;
            offsetWitnessLine (&witline, &yvec, ep, textLift, 0);

            ADIM_SETNAME (ep->partName, ADTYPE_EXT_RIGHT, ADSUB_NONE);
            if (SUCCESS != (status = adim_generateExtensionLine (ep, &witline.point[0], &witline.point[1],
                                                                 ep->GetDimElementCP()->GetDimTextCP(0)->flags.b.altSymb)))
                return (status);
            }
        }
    *currpoint = wit2.point[1];

    return (SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LinearDimensionHelper::StrokeDimension (AdimProcess& ep) const
    {
    DimensionElm const*  dim = ep.GetDimElementCP();
    if (dim->nPoints  < 2)
        return ERROR;

    ep.Init ();
    
    for (int index = 0; index < dim->nPoints; ++index)
        ep.points[index] = dim->GetPoint (index);

    DPoint3d orgPoint = ep.points[0];
    DVec3d yvec;
    (ep.rMatrix).GetColumn(yvec,  1);
    
    DSegment3d line;
    line.point[0] = ep.points[0];
    bsiDPoint3d_addScaledDVec3d (&line.point[1], &line.point[0], &yvec, dim->GetDimTextCP(0)->offset);
    ep.points[1] = line.point[1];
    ep.points[3] = dim->GetPoint (1);
    
    m_hdlr.GetStrings (ep.GetElemHandleCR(), ep.strDat.m_strings, 1, NULL);
    ep.ep.segment = 1;

    /*-------------------------------------------------------------------
    This flags crap is needed to handle joint terminators in
    generateDimension. I hope to find a better way some day.
    -------------------------------------------------------------------*/
    ep.flags.firstSeg = true;
    ep.flags.lastSeg  = dim->nPoints < 3 ? true : false;
    ADIM_SETSEG (ep.partName, 0);

    StatusInt status = SUCCESS;
    if (status = adim_generateSizeDim (ep.points+3, &ep, dim->GetDimTextCP(1)))
        return status;

    for (int i=2; i < dim->nPoints; i++)
        {
        ep.points[0] = orgPoint;
        ep.ep.segment = i;

        dimTextBlock_setPopulated (&ep, false);
        ep.m_textBlock = TextBlock::Create (*ep.GetDgnModelP ());

        if (dim->tmpl.stacked)
            adim_offsetStackPoint (&ep.points[1], ep.stackHeight, &ep.rMatrix);
        else
            ep.points[1] = ep.points[2];

        ep.points[2] = ep.points[3];
        ep.points[3] = dim->GetPoint (i);
        
        m_hdlr.GetStrings (ep.GetElemHandleCR(), ep.strDat.m_strings, i, NULL);

        ep.flags.firstSeg = false;
        ep.flags.lastSeg  = i == dim->nPoints-1 ? true : false;
        ADIM_SETSEG (ep.partName, (i-1));

        if (status = adim_generateSizeCont (ep.points+3, &ep, dim->GetDimTextCP(i)))
            return status;

        if (dim->tmpl.stacked && ep.ep.segment <= ep.stack.segNo)
            ep.stack.height += ep.stackHeight;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int     generateOrdinateDimension
(
AdimProcess     *ep,         /* => Function used to process elements   */
DSegment3dP      line,       /* => Ordinate dimension line             */
double          dimLength,   /* => Length of dimension (text value)    */
double          userOffset,  /* => user defined offset from default    */
DimText const   *dimText     /* => Text placement info/flags           */
)
    {
    DPoint2d        tile;
    DPoint3d        txtorg, dimOrg, dPoints[4];
    DVec3d          txtdir, direction, yDir, check;
    DPoint2d        textSize;
    bool            bFreeLocation;
    double          textDist, lineDist, textMargin, leader = 0.0;
    double          dotLen, halfText, overlap;
    int             status;

    clearTextBox (ep);

    mdlDim_extensionsGetOrdinateFreeLocationFlag (&bFreeLocation, ep->pOverrides, false);

    /*-----------------------------------------------------------------------------------
        For autocad compatibility we need to be able to suppress segment.
        Look suppress status from noWitness field of dimText.
    -----------------------------------------------------------------------------------*/
    if (dimText->flags.b.noWitness)
        return  SUCCESS;

    ElementHandleCR dimElement =  ep->GetElemHandleCR();
    DimensionElm const* dim = &dimElement.GetElementCP()->ToDimensionElm();

    adim_getTileSize (&tile, NULL, dimElement);
    adim_getLengthStrings (ep, dimLength + userOffset, dim->frmt.primaryAccuracy);
    adim_getTextSize (&textSize, ep, &ep->strDat, ADIM_TEXTSIZE_Exact);

    /*-------------------------------------------------------------------
    Now we need to take the combined string size and adjust the height
    so we can use it in overlap calculations for dog legs.
        dual        - add space between primary and secondary
        underline   - add space below
        box         - add space all the way around
    Copy the values from generate functions - a kludge for 5.5 but it works.
    -------------------------------------------------------------------*/
    if (dim->flag.dual)
        textSize.y    += dim->geom.textLift * 2.0;

    if (dim->flag.boxText || dim->flag.capsuleText)
        textSize.y    += tile.x;
    else if (dim->flag.underlineText)
        textSize.y    += tile.x / 2.0;

    textMargin = dim->geom.textMargin;

    direction.NormalizedDifference (*( &line->point[1]), *( &line->point[0]));

    txtorg = line->point[1];
    check  = direction;

    ep->vuMatrix.Multiply(*((DPoint3dP)&check));

    /*----------------------------------------------------------------------
      If x is negative, the text direction needs to be reversed. If the
      direction is vertical, the text must read "up hill". (-y to +y)
    ----------------------------------------------------------------------*/
    if (check.x < -mgds_fc_epsilon || fabs (check.y + 1.0) < 0.0001)
        bsiDVec3d_scale (&txtdir, &direction, -1.0);
    else
        txtdir = direction;

    if (ep->flags.textBlockPopulated)
        {
        /*-------------------------------------------------------------------------------
        Move text origin to its justification point - only do this for multiline text.
        Single line text justification is handled in adim_generateText down the road.
        -------------------------------------------------------------------------------*/
        switch  (adim_getDimTextJustificationHorizontal (ep))
            {
            case TextElementJustification::CenterMiddle:
                bsiDPoint3d_addScaledDVec3d (&txtorg, &txtorg, &txtdir, textSize.x * 0.5);
                break;

            case TextElementJustification::RightMiddle:
                bsiDPoint3d_addScaledDVec3d (&txtorg, &txtorg, &txtdir, textSize.x);
                break;
            }
        }

    /* Offset extension line from start point */
    bsiDPoint3d_addScaledDVec3d (&line->point[0], &line->point[0], &direction, dim->geom.witOffset);
    textDist = bsiDVec3d_dotProduct (&direction, &txtdir);

    if (textDist < 0.0)
        {
        if (dimText->flags.b.just == DIMTEXT_START)
            bsiDPoint3d_addScaledDVec3d (&line->point[1], &line->point[1], &txtdir, textSize.x + textMargin);
        else
            bsiDPoint3d_addScaledDVec3d (&txtorg, &line->point[1], &txtdir, -(textSize.x + textMargin));
        }
    else
        {
        if (dimText->flags.b.just == DIMTEXT_START)
            {
            bsiDPoint3d_addScaledDVec3d (&txtorg, &line->point[1], &txtdir, -textSize.x);
            bsiDPoint3d_addScaledDVec3d (&line->point[1], &line->point[1], &txtdir,
                        -(textSize.x + textMargin));
            }
        else
            {
            bsiDPoint3d_addScaledDVec3d (&line->point[1], &line->point[1], &txtdir, -textMargin);
            }
        }

    ep->rMatrix.GetColumn(yDir,  1);
    dimOrg = ep->points[0];
    bsiDVec3d_subtractDPoint3dDPoint3d (&check, &txtorg, &dimOrg);

    dotLen      = fabs (bsiDVec3d_dotProduct (&check, &yDir));
    halfText    = (textSize.y + textMargin) / 2.0;

    overlap     = (ep->stackHeight + halfText) - /* top of previous */
                  (dotLen - halfText);           /* bottom of current */

    leader = dim->geom.margin ? dim->geom.margin : 3.0 * tile.y;

    /*-------------------------------------------------------------------
    Rules for y-positioning of text, in the order of preference.
    1. Check if freely locating along the y. If so, apply it.
    2. V5.5 - if stacked bit is set and text overlaps previous dim, create
       a "dog leg" in the dimension line.
    3. Place the text without any y-offset.
    -------------------------------------------------------------------*/
    if (bFreeLocation && !LegacyMath::DEqual(dimText->offsetY, 0.0))
        {
        bsiDPoint3d_addScaledDVec3d (&txtorg, &txtorg, &yDir, dimText->offsetY);

        dPoints[0] = line->point[0];
        dPoints[3] = line->point[1];

        bsiDPoint3d_addScaledDVec3d (&dPoints[3], &dPoints[3], &yDir, dimText->offsetY);
        bsiDPoint3d_addScaledDVec3d (&dPoints[2], &dPoints[3], &direction, -leader);
        bsiDPoint3d_addScaledDVec3d (&dPoints[1], &dPoints[2], &direction, -leader);
        bsiDPoint3d_addScaledDVec3d (&dPoints[1], &dPoints[1], &yDir, -dimText->offsetY);

        ADIM_SETNAME (ep->partName, ADTYPE_DIMLINE, ADSUB_NONE);
        if (status = adim_generateLineString (ep, dPoints, 4, false, DIM_MATERIAL_DimLine))
            return  status;
        }
    else if (dim->tmpl.stacked && dimLength > 0.0 && overlap > 0.0)
        {
        bsiDPoint3d_addScaledDVec3d (&txtorg, &txtorg, &yDir, overlap);
        ep->stackHeight = dotLen + overlap;
        if  (ep->ep.segment == ep->stack.segNo)
            ep->stack.height = overlap;

        dPoints[0]  = line->point[0];
        dPoints[3]  = line->point[1];

        /*-----------------------------------------------------------
        Make dog leg leader go "leader" to left
        -----------------------------------------------------------*/
        bsiDPoint3d_addScaledDVec3d (dPoints+3, dPoints+3, &yDir, overlap);
        bsiDPoint3d_addScaledDVec3d (dPoints+2, dPoints+3, &direction, -leader);

        /*-----------------------------------------------------------
        Make leader kink "leader" long
        -----------------------------------------------------------*/
        bsiDPoint3d_addScaledDVec3d (dPoints+1, dPoints+2, &direction, -leader);
        bsiDPoint3d_addScaledDVec3d (dPoints+1, dPoints+1, &yDir, -overlap);

        ADIM_SETNAME (ep->partName, ADTYPE_DIMLINE, ADSUB_NONE);
        if (status = adim_generateLineString (ep, dPoints, 4, false, DIM_MATERIAL_DimLine))
            return  status;
        }
    else
        {
        ep->stackHeight = dotLen;

        bsiDVec3d_subtractDPoint3dDPoint3d (&direction, &line->point[1], &line->point[0]);
        lineDist  = bsiDVec3d_dotProduct (&txtdir, &direction);

        ADIM_SETNAME (ep->partName, ADTYPE_DIMLINE, ADSUB_NONE);
        if (textDist * lineDist > 0.0)
            if (status = adim_generateLine (ep, &line->point[0], &line->point[1], DIM_MATERIAL_DimLine))
                return (status);
        }

    return (adim_generateLinearDimension (ep, &txtorg, &txtdir, dimText->offsetY));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int      adim_generateOrdinate1
(
DPoint3d        *origin,     /* => Current data point                  */
AdimProcess     *ep,         /* => Function used to process elements   */
DimText const   *dimText     /* => Dimension text information          */
)
    {
    bool            bUseDatumValue = false;
    DVec3d          xvec;
    DSegment3d line;
    double          startValue, datumValue;

    mdlDim_overridesGetOrdinateStartValueX (&startValue, ep->pOverrides, 0.0);

    mdlDim_extensionsGetOrdinateUseDatumValueFlag (&bUseDatumValue, ep->pOverrides, 0);

    if (bUseDatumValue)
        mdlDim_extensionsGetOrdinateDatumValue (&datumValue, ep->pOverrides, 0.0);
    else
        datumValue = 0.0;

    ep->rMatrix.GetColumn(xvec,  0);

    line.point[0] = *origin;
    bsiDPoint3d_addScaledDVec3d (&line.point[1], &line.point[0], &xvec, dimText->offset);

    if (ep->pdDimLengths != NULL)
        ep->pdDimLengths[0] = startValue;

    /* NEEDSWORK : This start value (dimOverride app) stuff needs to be removed. When it
     * is removed, the datum value will be set using datumValue only */
    return (generateOrdinateDimension (ep, &line, 0.0, startValue + datumValue, dimText));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int      adim_generateOrdinate2
(
DPoint3d        *origin,     /* => Data point used to locate text      */
AdimProcess     *ep,         /* => Function used to process elements   */
DimText const   *dimText     /* => Dimension text information          */
)
    {
    double     dimLength;
    bool       bUseDatumValue = false, bReverseDecrement = false;
    DVec3d     xvec, yvec, rtmp;
    DPoint3d   startPoint;
    DSegment3d line;
    double     startValue, datumValue;

    mdlDim_overridesGetOrdinateStartValueX (&startValue, ep->pOverrides, 0.0);

    mdlDim_extensionsGetOrdinateUseDatumValueFlag (&bUseDatumValue, ep->pOverrides, 0);

    if (bUseDatumValue)
        mdlDim_extensionsGetOrdinateDatumValue (&datumValue, ep->pOverrides, 0.0);
    else
        datumValue = 0.0;

    ep->rMatrix.GetColumn(xvec,  0);
    ep->rMatrix.GetColumn(yvec,  1);

    line.point[0] = *origin;
    bsiDPoint3d_addScaledDVec3d (&line.point[1], &line.point[0], &xvec, (double)dimText->offset);

    /* calculate dimension length */
    startPoint = ep->points[0];
    bsiDVec3d_subtractDPoint3dDPoint3d (&rtmp, &line.point[0], &startPoint);

    /* Check if the user wants to decrement along the reverse direction */
    mdlDim_extensionsGetOrdinateReverseDecrementFlag (&bReverseDecrement, ep->pOverrides, 0);
    
    DimensionElm const* dim = ep->GetDimElementCP();
    /* Compute the dimension length based on the user preference */
    if (bReverseDecrement)
        dimLength = bsiDVec3d_dotProduct (&rtmp, &yvec) * dim->GetScale();
    else
        dimLength = fabs (bsiDVec3d_dotProduct (&rtmp, &yvec) * dim->GetScale());

    if (ep->pdDimLengths != NULL)
        ep->pdDimLengths[ADIM_GETSEG(ep->partName)] = dimLength;

    /* NEEDSWORK : This start value (dimOverride app) stuff needs to be removed. When it
     * is removed, the datum value will be set using datumValue only */
    return (generateOrdinateDimension (ep, &line, dimLength, startValue + datumValue, dimText));
    }

/*-----------------------------------------------------------------------------------
|  Structure used by adim_sortVertices
*----------------------------------------------------------------------------------*/
typedef struct
    {
    double      distance;
    DPoint3d    dimPoint;
    int         rootIndex;

    } PointSortingRecord;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/03
+---------------+---------------+---------------+---------------+---------------+------*/
static int     compareRecords
(
const void           *pArg1,
const void           *pArg2
)
    {
    const PointSortingRecord  *pRec1 = (const PointSortingRecord *) pArg1;
    const PointSortingRecord  *pRec2 = (const PointSortingRecord *) pArg2;

    if (pRec1->distance == pRec2->distance)
        return 0;

    return pRec1->distance < pRec2->distance ? -1 : 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt   adim_sortVertices (EditElementHandleR dimElement, RotMatrixCR rMatrix)
    {
    int const  bufferSize = offsetof (DependencyLinkage, root) + 7 + (sizeof (DependencyRootAssocPoint_I) * dimElement.GetElementP()->ToDimensionElm().nPoints);
    /*-------------------------------------------------------------------
    This function should be called to sort the points in an ordinate
    dimension ONLY if the stacked bit is set in the dimension.
    -------------------------------------------------------------------*/
    DPoint3d            *pVertices;
    PointSortingRecord  *pRecords;
    DependencyLinkage   *depLinkageP;
    if (NULL == (pRecords = (PointSortingRecord *) _alloca (sizeof (PointSortingRecord) * dimElement.GetElementP()->ToDimensionElm().nPoints)) ||
        NULL == (pVertices = (DPoint3d *) _alloca (sizeof (DPoint3d) * dimElement.GetElementP()->ToDimensionElm().nPoints)) ||
        NULL == (depLinkageP = (DependencyLinkage *) _alloca (bufferSize)))
        return ERROR;

    for (int index = 0; index < dimElement.GetElementP()->ToDimensionElm().nPoints; ++index)
        pVertices[index] = dimElement.GetElementP()->ToDimensionElm().GetPoint (index);
    
    DPoint3d    basePoint = pVertices[0];
    DVec3d      yDir;
    rMatrix.GetColumn(yDir,  1);
    
    bool    inOrder = true;
    double  previousDistance = 1.0 - DBL_MAX;
    /*-------------------------------------------------------------------
    Extract the dimension points into the sortable record array.  If the
    points are already in order, we can skip sorting.
    -------------------------------------------------------------------*/
    for (int iPoint = 0; iPoint < dimElement.GetElementP()->ToDimensionElm().nPoints; iPoint++)
        {
        pRecords[iPoint].dimPoint = dimElement.GetElementP()->ToDimensionElm().GetPoint(iPoint);
        pRecords[iPoint].rootIndex = -1;

        bsiDVec3d_subtractDPoint3dDPoint3d ((DVec3dP)&pVertices[iPoint], &pVertices[iPoint], &basePoint);
        pRecords[iPoint].distance = bsiDVec3d_dotProduct ((DVec3dP)&pVertices[iPoint], &yDir);

        if (pRecords[iPoint].distance < previousDistance)
            inOrder = false;

        previousDistance = pRecords[iPoint].distance;
        }

    if (inOrder)
        return SUCCESS;

    /*-------------------------------------------------------------------
    Extract the dependency roots into the sortable record array.
    -------------------------------------------------------------------*/
    DependencyLinkageAccessor linkageCP;
    bool hasDepLinkage = false;
    if (SUCCESS == DependencyManagerLinkage::GetLinkageFromMSElement (&linkageCP, dimElement.GetElementP(), DEPENDENCYAPPID_MicroStation, DEPENDENCYAPPVALUE_AssocPoint) &&
        linkageCP->u.f.rootDataType == DEPENDENCY_DATA_TYPE_ASSOC_POINT_I)
        {
        hasDepLinkage = true;
        
        size_t sz = DependencyManagerLinkage::GetSizeofLinkage (*linkageCP, 0);
        memcpy (depLinkageP, linkageCP, std::min<size_t> (sz, bufferSize));

        for (int iRoot=0; iRoot < depLinkageP->nRoots; iRoot++)
            {
            int iPoint = depLinkageP->root.a_i[iRoot].i;

            pRecords[iPoint].rootIndex = iRoot;
            }
        }

    /*-------------------------------------------------------------------
    Sort the records
    -------------------------------------------------------------------*/
    qsort (pRecords, dimElement.GetElementP()->ToDimensionElm().nPoints, sizeof (*pRecords), compareRecords);

    /*-------------------------------------------------------------------
    Put the sorted points back into the dimension, and place the
    correct point numbers into the dependency roots.
    -------------------------------------------------------------------*/
    for (int iPoint = 0; iPoint < dimElement.GetElementP()->ToDimensionElm().nPoints; iPoint++)
        {
        dimElement.GetElementP()->ToDimensionElmR().SetPoint(pRecords[iPoint].dimPoint, iPoint);
        
        int iRoot;
        if (hasDepLinkage && (0 <= (iRoot = pRecords[iPoint].rootIndex)))
            {
            depLinkageP->root.a_i[iRoot].i = iPoint;
            }
        }

    /*-------------------------------------------------------------------
    Replace the dependency linkage
    -------------------------------------------------------------------*/
    if (hasDepLinkage)
        return DependencyManagerLinkage::UpdateLinkage (dimElement, *depLinkageP, 0);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       OrdinateDimensionHelper::ReEvaluateElement (EditElementHandleR dimElement) const
    {
    DimensionElm const* dim = &dimElement.GetElementCP()->ToDimensionElm();
    if (dim->nPoints < 1)
        return ERROR;
    
    RotMatrix rMatrix;
    m_hdlr.GetRotationMatrix (dimElement, rMatrix);

    if (dim->tmpl.stacked)
        adim_sortVertices (dimElement, rMatrix);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       OrdinateDimensionHelper::StrokeDimension (AdimProcess& ap) const
    {
    DimensionElm const* dim = ap.GetDimElementCP();
    if (dim->nPoints < 1)
        return ERROR;

    ap.Init ();

    m_hdlr.GetStrings (ap.GetElemHandleCR(), ap.strDat.m_strings, 0, NULL);

    ap.points[0] = dim->GetPoint(0);

    ap.ep.segment = 0;

    ADIM_SETSEG (ap.partName, ap.ep.segment);

    StatusInt status = SUCCESS;
    if (status = adim_generateOrdinate1 (ap.points, &ap, dim->GetDimTextCP(0)))
        return  status;

    for (int i=1; i < dim->nPoints; i++)
        {
        ap.ep.segment = i;

        dimTextBlock_setPopulated (&ap, false);
        ap.m_textBlock = TextBlock::Create (*ap.GetElemHandleCR().GetDgnModelP ());
        ap.points[2] = dim->GetPoint (i);
        
        m_hdlr.GetStrings (ap.GetElemHandleCR(), ap.strDat.m_strings, i, NULL);
        ADIM_SETSEG (ap.partName, ap.ep.segment);

        if (status = adim_generateOrdinate2 (ap.points+2, &ap, dim->GetDimTextCP(i)))
            return  status;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/08
+---------------+---------------+---------------+---------------+---------------+------*/
static WString adim_generateLabelLineDirectionString
(
DVec3d&         lineDir,
AdimProcess*    ep
)
    {
    DirectionMode   directionMode;
    double          baseDir;
    bool            clockwise;

    mdlDim_extensionsGetDirectionMode (&directionMode, ep->pOverrides);
    mdlDim_extensionsGetDirectionBaseDir (&baseDir, ep->pOverrides);
    mdlDim_extensionsGetDirectionClockwise (&clockwise, ep->pOverrides);

    AngleFormatVals angleFormat     = adim_getAngleFormat (ep->GetElemHandleCR());
    int             angleAccuracy   = ep->GetDimElementCP()->frmt.secondaryAccuracy;

    if (DIMSTYLE_VALUE_Value_DMSPrecisionMode_Floating == ep->GetDimElementCP()->frmt.dmsAccuracyMode)
        adim_getFloatingAngularFormat (&angleFormat, &angleAccuracy);

    DirectionFormatterPtr  formatter = DirectionFormatter::Create();

    formatter->GetAngleFormatter().SetAngleModeFromLegacy (angleFormat);
    formatter->GetAngleFormatter().SetAnglePrecisionFromLegacy (angleAccuracy);

    formatter->SetDirectionMode (directionMode);
    formatter->SetBaseDirection (baseDir);
    formatter->SetClockwise      (TO_BOOL (clockwise));

    formatter->GetAngleFormatter().SetLeadingZero   (ep->GetDimElementCP()->flag.leadingZero2);
    formatter->GetAngleFormatter().SetTrailingZeros (ep->GetDimElementCP()->flag.trailingZeros2);
    formatter->GetAngleFormatter().SetAllowNegative (false);

    //Fix:TR#287864 We were not respecting true north value from a model
    //This might cause difference in behavior for refernce files
    double trueNorthValue = ep->GetDgnModelP()->GetDgnProject().Units().GetAzimuth();
    formatter->SetTrueNorthValue (trueNorthValue);
    formatter->SetAddTrueNorth  (true);

    double  directionValue = Angle::Atan2 (lineDir.y, lineDir.x) * msGeomConst_degreesPerRadian;

    return  formatter->ToString (directionValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int      adim_generateLabelLine
(
DPoint3d        *inPoint,
AdimProcess     *ep,
DimText const   *dimText
)
    {
    DPoint2d       tile;
    DSegment3d line;
    DPoint3d       baseTextOrigin;
    DVec3d         lineDir, textDir, textInViewDir, perpvec, currDir;
    double         length, textOffset, textMargin;
    int            status = SUCCESS;

    bool           bSuppressAngle = false, bSuppressLength = false, bInvert = false, bAdjacent = true;
    DPoint2d       textSize;

    mdlDim_extensionsGetLabelLineSuppressAngleFlag (&bSuppressAngle, ep->pOverrides, false);
    mdlDim_extensionsGetLabelLineSuppressLengthFlag (&bSuppressLength, ep->pOverrides, false);
    mdlDim_extensionsGetLabelLineInvertLabelsFlag (&bInvert, ep->pOverrides, false);
    mdlDim_extensionsGetLabelLineAdjacentLabelsFlag (&bAdjacent, ep->pOverrides, false);
    
    ElementHandleCR dimElement =  ep->GetElemHandleCR();
    DimensionElm const* dim = &dimElement.GetElementCP()->ToDimensionElm();
    adim_getTileSize (&tile, NULL, dimElement);
    /*-------------------------------------------------------------------
    Label Line dimensions with a version less than 6 always used
    0.5 * text height as the lower margin. Changed to pay attention to
    geom.lowerMargin for 5.5, 8/95
    -------------------------------------------------------------------*/
    if (ep->GetDimElementCP()->version < 6)
        textMargin = 0.5 * tile.y;
    else
        textMargin = dim->geom.textLift;

    line.point[0] = ep->points[1];
    line.point[1] = ep->points[2];
    length = lineDir.NormalizedDifference (*( &line.point[1]), *( &line.point[0]));

    bsiDVec3d_subtractDPoint3dDPoint3d (&currDir, inPoint, &line.point[0]);
    textOffset = bsiDVec3d_dotProduct (&lineDir, &currDir);
    bsiDPoint3d_addScaledDVec3d (&baseTextOrigin, &line.point[0], &lineDir, textOffset);

    textInViewDir = textDir = lineDir;

    ep->vuMatrix.Multiply(textInViewDir);

    if (textInViewDir.x < -mgds_fc_epsilon || fabs (textInViewDir.y + 1.0) < 0.0001)
        bsiDVec3d_scale (&textDir, &textDir, -1.0);

    adim_getYVec (&perpvec, &textDir, ep);

    if ( ! bSuppressAngle)
        {
        TextElementJustification    angleJust;
        DPoint3d                    angleTextOrigin;
        WString                     dirString = adim_generateLabelLineDirectionString (lineDir, ep);
        WStringR                    epDirString = *(ep->strDat.m_strings.GetPrimaryStrings() + 1);

        adim_insertLengthString (epDirString, dirString.c_str());

        if ((bInvert && bAdjacent) || (!bInvert && !bAdjacent))
            {
            //angle is below the line
            angleJust = static_cast <TextElementJustification>(6 * dim->GetDimTextCP(0)->flags.b.just - 6);
            bsiDPoint3d_addScaledDVec3d (&angleTextOrigin, &baseTextOrigin, &perpvec, -textMargin);
            }
        else
            {
            //angle is above the line
            angleJust = static_cast <TextElementJustification>(6 * dim->GetDimTextCP(0)->flags.b.just - 4);
            bsiDPoint3d_addScaledDVec3d (&angleTextOrigin, &baseTextOrigin, &perpvec, textMargin);
            }

        if (bAdjacent && !bSuppressLength)
            {
            adim_getStringSize (&textSize, epDirString.c_str(), &tile, dim->text.font, ep);
            bsiDPoint3d_addScaledDVec3d (&angleTextOrigin, &angleTextOrigin, &textDir, 0.5 * textSize.x + tile.x);
            }

        /*---------------------------------------------------------------
         Indicate the part name as Upper_Tol so that the angle text gets
         stored in the second position in the heap. This is done
         specifically so V7 labellines look right.
         ---------------------------------------------------------------*/
        ADIM_SETNAME (ep->partName, ADTYPE_TEXT_UPPER, ADSUB_TOL_UPPER);
        if (status = adim_generateText (ep, epDirString.c_str(), &angleTextOrigin, &textDir, angleJust, &tile))
            return (status);

        adim_harvestTextBoxForDerivedData (ep);
        }

    if ( ! bSuppressLength)
        {
        TextElementJustification    lengthJust;
        double                      dRoundOff;
        WChar                       masterLbl[MAX_DIMSTR], subLbl[MAX_DIMSTR];
        DPoint3d                    lengthTextOrigin;
        DimUnitBlock                dimUnits;
        WStringR                    epLenString = *ep->strDat.m_strings.GetPrimaryStrings();

        adim_getDimensionUnits (&dimUnits, masterLbl, subLbl, dimElement, true);
        adim_convertUnits (&length, length, &dimUnits, ep->GetDgnModelP());

        mdlDim_extensionsGetRoundOff (&dRoundOff, ep->pOverrides, 0.0);
        adim_uorToDimString (epLenString, &dimUnits, masterLbl, subLbl,
                             length * dim->GetScale(), true, 0, 0, 0, &dRoundOff, false, ep, dim->frmt.primaryAccuracy);

        if (!bInvert)
            {
            // length is above the line
            lengthJust = static_cast <TextElementJustification>(6 * dim->GetDimTextCP(0)->flags.b.just - 4);
            bsiDPoint3d_addScaledDVec3d (&lengthTextOrigin, &baseTextOrigin, &perpvec, textMargin);
            }
        else
            {
            // length is below the line
            lengthJust = static_cast <TextElementJustification>(6 * dim->GetDimTextCP(0)->flags.b.just - 6);
            bsiDPoint3d_addScaledDVec3d (&lengthTextOrigin, &baseTextOrigin, &perpvec, -textMargin);
            }

        if (bAdjacent && !bSuppressAngle)
            {
            adim_getStringSize (&textSize, epLenString.c_str(), &tile, dim->text.font, ep);
            bsiDPoint3d_addScaledDVec3d (&lengthTextOrigin, &lengthTextOrigin, &textDir, - ( 0.5 * textSize.x + tile.x));
            }

        /*---------------------------------------------------------------
         Indicate the part name as Upper so that the angle text gets
         stored in the first position in the heap
         ---------------------------------------------------------------*/
        ADIM_SETNAME (ep->partName, ADTYPE_TEXT_UPPER, ADSUB_NONE);
        if (status = adim_generateText (ep, epLenString.c_str(), &lengthTextOrigin, &textDir, lengthJust, &tile))
             return (status);

        adim_harvestTextBoxForDerivedData (ep);
        }

    return (status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LabelLineHelper::StrokeDimension (AdimProcess& ap) const
    {
    DimensionElm const* dim = ap.GetDimElementCP();
    if (dim->nPoints < 1)
        return ERROR;

    ap.Init();
    m_hdlr.GetStrings (ap.GetElemHandleCR(), ap.strDat.m_strings, 0, NULL);
    
    ap.points[0] = dim->GetPoint (0);
    
    if (dim->nPoints == 1)
        {
        /*---------------------------------------------------------------
        degenerate label line - one point and no associations could be
        caused by drop association on 4.0 beta single point label line.
        ---------------------------------------------------------------*/
        return (SUCCESS);
        }
    else if (dim->nPoints == 3)
        {
        /*---------------------------------------------------------------
        4.0 beta non associattive label line had 3 points ->
            (1)text origin, (2)line origin, (3)line end.
        ---------------------------------------------------------------*/
        ap.points[1] = ap.GetDimElementCP()->GetPoint(1);
        ap.points[2] = ap.GetDimElementCP()->GetPoint(2);
        }
    else
        {
        ap.points[1] = ap.points[0];
        ap.points[2] = ap.GetDimElementCP()->GetPoint(1);
        
        DSegment3d lineSeg;
        lineSeg.point[0] = ap.points[1];
        lineSeg.point[1] = ap.points[2];
        
        DVec3d lineDir;
        lineDir.NormalizedDifference (*( &lineSeg.point[1]), *( &lineSeg.point[0]));
        bsiDPoint3d_addScaledDVec3d (&ap.points[0], &lineSeg.point[0], &lineDir, ap.GetDimElementCP()->GetDimTextCP(0)->offset);
        }

    return adim_generateLabelLine (ap.points, &ap, NULL);
    }

/*---------------------------------------------------------------------------------------
Notes:

    (1) intersect_len is computed as follows:

      o Picture the dimension line:

             \
              \
               \

      o The bvector called check (called "chk" in this note) is the dimension line's direction bvector.

            chk.y :\  1         So, chk defines a right triangle,
                  ..            whose hypoteneuse is length 1.
                  chk.x

        chk is a UNIT bvector.

      o The text is assumed to fit in a h X w box:

            | w  |
            +----+ --
            |text|  h
            +----+ --

        The intersection between the dimension line can be solved as follows:

    case 1: chk.y*w <= chk.x*h  tall, narrow text box

            \ |w |
             \+--+              The intersection defines a right triangle:
              |  |
              |\ |              |\
              | \|              | \ len   <->   chk.y :\  1
              |  |              ....                  ..
              +--+\               w                   chk.x
                   \

        The chk triangle and the intersection triangle are similar triangles, so
            len/1 = w/chk.x

        This also includes the case where:

          |      w        |
          +---------------+      --     chk.x = 1; chk.y = 0
        --|---------------|---   h      len = w/chk.x
          +---------------+      --

    case 2: chk.y*w > chk.x*h   wide text box

            \
        +----------------+ --   The intersection defines a right triangle:
        |     \          |  h     h :\  len <->  chk.y :\  1
        +----------------+ --       ---                ..
                \                                      chk.x

        The chk triangle and the intersection triangle are similar triangles, so
            len/1 = h/chk.y

    3D: If the dimension line and text box are in different planes (as can happen with
        "horizontal" text), we solve the apparent intersection by restating chk in the
        local coordinate system of the view.

-------------------------------------------------------------------------------------------
(2) Computing actual intersection of dimension line w/ dimension text

    ep->textBox defines the rotated/optimized range box of the text as actually drawn.
    If we rotated the text, wrapped lines, etc., the range box will enclose the results.

    Suppose the text is rotated -270 degrees -- that is, vertical (from top to bottom)

pStart                              pStart  = start of dimension line
  \    |     h   | m|               pEnd    = end           "
   \
    \ [0]           [3]
     \ +------------+   ---         h       = ep->textBox.height: full height of text as drawn
      \| o-->UP     |    m          w       = ep->textBox.width:   "   width    "
       | |+------+  |   ---         m       = ep->GetDimElementCP()->geom.textMargin: space around text
       | V| --|  |  |
       |  | |:|  |  |
       |  |  X   |  |    w
       |  | --|  |  |               o       = ep->textBox.baseFirstChar: lower left corner of first char
       |  | .    |  |               V       = baseDir: direction along the base  of the text
       |  +------+  |   ---         >UP     = upDir:      "                height   "
       |            |
       +------------+
      [1]     \     [2]             [0..3] are the corners of the text box
               \
               pEnd



---------------------------------------------------------------------------------------*/
