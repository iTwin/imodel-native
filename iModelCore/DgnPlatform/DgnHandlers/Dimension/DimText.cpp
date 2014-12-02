/*----------------------------------------------------------------------+
|
|   $Source: DgnHandlers/Dimension/DimText.cpp $
|     $Date$
|   $Author$
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/DgnRscFont.h>

/*---------------------------------------------------------------------------------------
    Accuracy bitmask fields

    0 0 0 0  0 0 0 0
    | | | |  | | | + 0x01: on bit expresses the precision decimal position, see mdldim.h for definitions
    | | | |  | | +-- 0x02: "
    | | | |  | +---- 0x04: "
    | | | |  +------ 0x08: "
    | | | |
    | | | +--------- 0x10: "
    | | +----------- 0x20: "
    | +------------- 0x40: " (only when used with decimal values, other meanings see below)
    +--------------- 0x80: on decimal, off fraction value

    0 1 x x  x x x x    : alternative display for value
    ===============================================================
    0 1 0 0  0 x x x    : ext'd accuracy display, scientific format (0x40 - 0x47)
               +++++      number of decimals (power of 10)

---------------------------------------------------------------------------------------*/

USING_NAMESPACE_BENTLEY_DGNPLATFORM

#define FMTw_dash                   (L"-")
#define FMTw_space                  (L" ")
#define FMTw_lu                     (L"%lu")
#define FMTw_ld                     (L"%ld")

#define round(x)                    floor(x+0.5)

#define DIMACC_IsScientific(a)      (((a >> 3) ^ 0x8) ? false : true)

#define DIMACC_SciPrecisionPwr(a)   (((a >> 3) ^ 0x8) ? 0 : (a & 0x07)+1)

/*----------------------------------------------------------------------+
|                                                                       |
|   External Variables                                                  |
|                                                                       |
+----------------------------------------------------------------------*/
static int  generateULText (AdimProcess*, WString* strings, DPoint3d*, DVec3d*);
static int  generatePMText (AdimProcess*, WString* strings, DPoint3d*, DVec3d*);

static void getSingleLength (AdimProcess*, bool, double, UInt16, UInt16,
                double, DimUnitBlock*, WCharCP, WCharCP, UShort primaryAccuracy);
static void getPMLength (AdimProcess*, DPoint2dP, WString*, double,
                bool, UInt16, UInt16, double, DimUnitBlock*, WCharCP, WCharCP, UShort primaryAccuracy);
static void getULLength (AdimProcess*, DPoint2dP, WString*, double,
                bool, double, DimUnitBlock*, WCharCP, WCharCP, UShort primaryAccuracy);

static void superscriptLSD (WCharP, DimensionElm const*, double, int, AdimProcess*);

static int  generateTemplateSymbol (AdimProcess*, DPoint3d*, DVec3d*, double, int);
static int  generateCustomSymbol (AdimProcess*, DPoint3d*, DVec3d*, double, int);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
static void    addPrefix
(
WString&        string,
UInt16          prefix
)
    {
    if (prefix)
        {
        wchar_t buf[64];
        BeStringUtilities::Snwprintf (buf, L"%d", prefix);
        string.insert (0, buf);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
static void    addSuffix
(
WString&        string,
UInt16          suffix
)
    {
    if (suffix)
        {
        string.push_back(suffix);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BentleyApi::adim_loadTextBlockFromMarkedUpString
(
TextBlockR      textBlock,
WCharCP       pwString,
TextParamWide*  pTextParamWide,
DPoint2dCP      pTileSize,
AdimProcess*    pAdimProcess
)
    {
    textBlock.Clear ();

    TextBlockProperties textblockProperties (*pTextParamWide, *pTileSize, 0, textBlock.GetDgnModelR ());
    textBlock.SetProperties (textblockProperties);

    ParagraphProperties paragraphProperties (*pTextParamWide, textBlock.GetDgnModelR ());

    ParagraphRange paragraphRange (textBlock);
    textBlock.SetParagraphPropertiesForRange (paragraphProperties, paragraphRange);
    
    /*-----------------------------------------------------------------------------------
     A bug in line arranger produces incorrect text node origin when linespacing=0.
     This is a hack to ensure correct text node before line arranger gets fixed.
     -----------------------------------------------------------------------------------*/
    textBlock.SetLineSpacingValue (1.0, paragraphRange);
    textBlock.SetLineSpacingValueOverrideFlag (true, paragraphRange);

    return dimTextBlock_appendMarkedUpString (textBlock, pwString, pTextParamWide, pTileSize, pAdimProcess);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name      adim_getCharWidth                                           |
|                                                                       |
| author    JVB                                         1/91            |
|                                                                       |
+----------------------------------------------------------------------*/
static void adim_getCharWidth
(
double         *pWidth,
int             dimChar,
double          height,
int             fontNo,
ElementHandleCR    dimElement
)
    {
    if (0 == dimChar)
        {
        *pWidth = 0.0;
        return;
        }

    TextStringProperties    props;

    DPoint2d    textSize;
    textSize.x = textSize.y = height;
    props.SetFontSize (textSize);

    DgnFontCR       font = *DgnFontManager::ResolveFont (fontNo, *dimElement.GetDgnProject(), DGNFONTVARIANT_DontCare);
    props.SetFont (font);

    WChar     chars[] = {(WChar)dimChar, L'\0'};
    TextString  textString (chars, NULL, NULL, props);

    *pWidth = textString.GetWidth();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::adim_getStringSize
(
DPoint2dP       stringSize,
WCharCP       string,
DPoint2dCP      tileSize,
int             fontNo,
AdimProcess*    pAdimProcess
)
    {
    TextParamWide       textParamWide;

    memset (&textParamWide, 0, sizeof (TextParamWide));

    DgnModelP modelRef = pAdimProcess->GetDgnModelP();
    if (pAdimProcess->pTextStyle)
        textParamWide = DgnTextStylePersistence::Legacy::ToTextParamWide(*pAdimProcess->pTextStyle, modelRef->GetDgnProject(), NULL, NULL);
    else
        textParamWide.font = fontNo;

    if (dimTextBlock_stringContainsMarkup (string))
        {
        TextBlock textBlock (*modelRef);

        if (SUCCESS ==  adim_loadTextBlockFromMarkedUpString (textBlock, string, &textParamWide, tileSize, pAdimProcess))
            {
            textBlock.PerformLayout ();

            // Do *NOT* use the exact range
            DRange3d range = textBlock.GetNominalRange ();

            if (bsiDRange3d_isNull (&range))
                stringSize->x = stringSize->y = 0.0;
            else
                {
                stringSize->x = range.high.x - range.low.x;
                stringSize->y = range.high.y - range.low.y;
                }
            }
        }
    else
        {
        /* This code originally called mdlText_getSizeUsingTileWide2 which contains some extra logic for dealing with
           RTL and vertical text.  I removed it for now, but if we see problems, look there first */
        TextString txString (string, NULL, NULL, TextStringProperties (textParamWide, *tileSize, modelRef->Is3d (), modelRef->GetDgnProject ()));

        stringSize->x = txString.GetWidth();
        stringSize->y = txString.GetHeight();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/04
+---------------+---------------+---------------+---------------+---------------+------*/
void    BentleyApi::adim_harvestTextBoxForDerivedData
(
AdimProcess    *ap
)
    {
    if (ap->pDerivedData && ap->pDerivedData->pTextBoxes)
        {
        AdimSegmentTextBoxes  *pTextBox = &ap->pDerivedData->pTextBoxes[ADIM_GETSEG(ap->partName)];

        if (ADTYPE_TEXT_LOWER == ADIM_GETTYPE (ap->partName))
            {
            pTextBox->secondary = ap->textBox[1];
            pTextBox->flags.hasSecondary = true;
            }
        else
            {
            pTextBox->primary = ap->textBox[0];
            pTextBox->flags.hasPrimary = true;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
int BentleyApi::adim_generateSingleDimension
(
AdimProcess  *ep,
bool          upper,
DPoint3d     *origin,
DVec3d       *direction
)
    {
    int             iStatus = -1;

    // This function gets called for both primary and secondary text.
    // Only the primary text can have mltext.
    if (upper)
        iStatus = adim_generateTextUsingDescr (ep, origin, direction);

    if (-1 == iStatus)
        {
        if (ep->GetDimElementCP()->flag.tolerance)
            {
            if (ep->GetDimElementCP()->flag.tolmode)
                iStatus = generateULText (ep, upper ? ep->strDat.GetPrimaryStrings() : ep->strDat.GetSecondaryStrings(), origin, direction);
            else
                iStatus = generatePMText (ep, upper ? ep->strDat.GetPrimaryStrings() : ep->strDat.GetSecondaryStrings(), origin, direction);
            }
        else                /* straight non-toleranced single dimensioning */
            {
            WStringP    str = ep->strDat.m_strings.GetString (upper ? DIMTEXTPART_Primary : DIMTEXTPART_Secondary, DIMTEXTSUBPART_Main);

            iStatus = adim_generateText (ep, str->c_str(), origin, direction, TextElementJustification::LeftMiddle, NULL);
            }
        }

    adim_harvestTextBoxForDerivedData (ep);

    return iStatus;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int generatePMText
(
AdimProcess*    ep,
WString*        strings,
DPoint3d*       origin,
DVec3d*         direction
)
    {
    DimTolrBlock*               tolBlock;
    DPoint2d                    adTile, tolTile, preTile;
    DPoint2d                    dimSize, minusSize, plusSize;
    DPoint3d                    rtolorg, rloworg, ruporg;
    double                      tolOff;
    WString                     tmpString;
    WChar                       str[2];
    int                         status = 0;
    int                         fontNo;
    TextElementJustification    mainJust = TextElementJustification::LeftMiddle;

    if (NULL == (tolBlock = (DimTolrBlock*) mdlDim_getOptionBlock (ep->GetElemHandleCR(), ADBLK_TOLERANCE, NULL)))
        return (ERROR);

    fontNo = ep->GetDimElementCP()->text.font;
    adim_getTileSize (&adTile, &tolTile, ep->GetElemHandleCR());
    preTile.x = preTile.y = tolTile.y * 2.0 + tolBlock->tolVertSep;

    adim_getStringSize (&dimSize, strings[0].c_str(), &adTile, fontNo, ep);
    dimSize.x += tolBlock->tolHorizSep;

    /*-----------------------------------------------------------------------------------
    Angular dimension uses its original justification while linear dimension always uses
    LC, which is passed in adim_generateText.  This function should also do the same thing
    so when tolerance is active the angular dimension text can be adjusted correctly based
    on its justification.  We only need to do this for main text here, not the tolerance
    symbols which should still use LC.
    -----------------------------------------------------------------------------------*/
    if (DIM_ANGULAR(static_cast<DimensionType>(ep->GetDimElementCP()->dimcmd)))
        mainJust = adim_getDimTextJustificationHorizontal(ep);

    if (tolBlock->lowerTol || tolBlock->upperTol)
        {
        switch (mainJust)
            {
            case TextElementJustification::CenterMiddle:
                bsiDPoint3d_addScaledDVec3d (&rtolorg, origin, direction, 0.5 * dimSize.x);
                break;
            case TextElementJustification::RightMiddle:
                rtolorg = *origin;
                break;
            default:
                bsiDPoint3d_addScaledDVec3d (&rtolorg, origin, direction, dimSize.x);
            }

        if (!tolBlock->flags.stackEqual && tolBlock->lowerTol == tolBlock->upperTol)
            {
            tmpString = strings[2];
            if (tolBlock->pmChar)
                tmpString.push_back(tolBlock->pmChar);
            else
                {
                DgnFontCP font = ep->GetDgnModelP()->GetDgnProject().Fonts().FindFont (fontNo);
                if (NULL != font)
                    tmpString.push_back(font->RemapFontCharToUnicodeChar (font->GetPlusMinusCharCode ()));
                }

            addPrefix (tmpString, tolBlock->prefix);
            addSuffix (tmpString, tolBlock->suffix);

            ADIM_SETNAME (ep->partName, ADTYPE_INHERIT, ADSUB_TOL_SINGLE);
            if (SUCCESS != (status = adim_generateText (ep, tmpString.c_str(), &rtolorg, direction, TextElementJustification::LeftMiddle, &tolTile)))
                return  status;
            }
       else
            {
            /* Put here so that prefix is at least listed as a tolerance */
            ADIM_SETNAME (ep->partName, ADTYPE_INHERIT, ADSUB_TOL_UPPER);

            if (tolBlock->prefix)
                {
                str[0] = tolBlock->prefix;
                str[1] = '\0';

                if (SUCCESS != (status = adim_generateText (ep, str, &rtolorg, direction, TextElementJustification::LeftMiddle, &preTile)))
                    return  status;

                adim_getCharWidth (&dimSize.x, tolBlock->prefix, preTile.y, ep->GetDimElementCP()->text.font, ep->GetElemHandleCR());
                bsiDPoint3d_addScaledDVec3d (&rtolorg, &rtolorg, direction, dimSize.x);
                }

           tolOff = (double)(tolTile.y + tolBlock->tolVertSep) / 2.0;

           adim_offsetText (&rloworg, &rtolorg, direction, -tolOff, ep);
           adim_offsetText (&ruporg,  &rtolorg, direction,  tolOff, ep);

           if (SUCCESS != (status = adim_generateText (ep, strings[1].c_str(), &ruporg, direction, TextElementJustification::LeftMiddle, &tolTile)))
                return  status;

           ADIM_SETNAME (ep->partName, ADTYPE_INHERIT, ADSUB_TOL_LOWER);
           if (SUCCESS != (status = adim_generateText (ep, strings[2].c_str(), &rloworg, direction, TextElementJustification::LeftMiddle, &tolTile)))
                return  status;

           if (tolBlock->suffix)
                {
                adim_getStringSize (&plusSize,  strings[1].c_str(), &tolTile, fontNo, ep);
                adim_getStringSize (&minusSize, strings[2].c_str(), &tolTile, fontNo, ep);

                dimSize.x = (minusSize.x > plusSize.x) ? minusSize.x : plusSize.x;
                bsiDPoint3d_addScaledDVec3d (&rtolorg, &rtolorg, direction, dimSize.x);
                str[0] = tolBlock->suffix;
                str[1] = '\0';

                if (SUCCESS != (status = adim_generateText (ep, str, &rtolorg, direction, TextElementJustification::LeftMiddle, &preTile)))
                    return  status;
                }
           }
        }

    ADIM_SETNAME (ep->partName, ADTYPE_INHERIT, ADSUB_NONE);
    return (status = adim_generateText (ep, strings[0].c_str(), origin, direction, mainJust, &adTile));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int generateULText
(
AdimProcess*    ep,
WString*        strings,
DPoint3d*       origin,
DVec3d*         direction
)
    {
    DimTolrBlock   *tolBlock;
    DPoint2d       adTile;
    DPoint3d       rloworg, ruporg;
    double         tolSep;
    int            status = 0;

    if (NULL == (tolBlock = (DimTolrBlock*) mdlDim_getOptionBlock (ep->GetElemHandleCR(), ADBLK_TOLERANCE, NULL)))
        return  ERROR;

    adim_getTileSize (&adTile, NULL, ep->GetElemHandleCR());
    if ((tolBlock->upperTol == 0) && (tolBlock->lowerTol == 0))
        {
        ADIM_SETNAME (ep->partName, ADTYPE_INHERIT, ADSUB_LIM_SINGLE);
        if (SUCCESS != (status = adim_generateText (ep, strings[0].c_str(), origin, direction, TextElementJustification::LeftMiddle, &adTile)))
            return  status;
        }
    else
        {
        tolSep = (double)(adTile.y + tolBlock->tolVertSep) / 2.0;

        adim_offsetText (&rloworg, origin, direction,  tolSep, ep);
        adim_offsetText (&ruporg,  origin, direction, -tolSep, ep);

        ADIM_SETNAME (ep->partName, ADTYPE_INHERIT, ADSUB_LIM_UPPER);
        if (SUCCESS != (status = adim_generateText (ep, strings[0].c_str(),  &rloworg, direction, TextElementJustification::LeftMiddle, &adTile)))
            return  status;

        ADIM_SETNAME (ep->partName, ADTYPE_INHERIT, ADSUB_LIM_LOWER);
        if (SUCCESS != (status = adim_generateText (ep, strings[1].c_str(), &ruporg, direction, TextElementJustification::LeftMiddle, &adTile)))
            return  status;
        }

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    adim_getEffectiveUnderline
(
AdimProcess     *ep          /* => Function used to process elements   */
)
    {
    bool        bUnderline = false;

    /* If we are using the line arranger, or if the dimension is using a text style
       let the text system handle the underlining */
    if (false == dimTextBlock_getRequiresTextBlock (ep, NULL) &&
        NULL == ep->pTextStyle)
        {
        mdlDim_overridesGetSegmentFlagUnderlineText (&bUnderline, ep->pOverrides,
                                                     ADIM_GETSEG (ep->partName),
                                                     ep->GetDimElementCP()->flag.underlineText);
        }

    return  bUnderline;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          adim_rotateText                                         |
|                                                                       |
| author        DonFu                                   07/02           |
|                                                                       |
+----------------------------------------------------------------------*/
void     BentleyApi::adim_rotateText
(
DVec3dP         pDirection,    /* <=> Dimension text direction        */
DPoint3dP       pOrigin,       /* <=> Dimension text origin           */
DPoint2dP       pBoxSize,      /* => text frame box size              */
AdimProcess*    pAdimProcess   /* => Function used to process elements*/
)
    {
    double      textAngle = 0;

    /*-------------------------------------------------------------------
    If text has a rotation angle, it must be rotated about the center
    origin, not justification point.  Thus we must transform the input
    (always) LC origin to counter rotate the text to get correct overal
    result:
    -------------------------------------------------------------------*/
      if (mdlDim_overridesGetSegmentTextRotation(&textAngle, pAdimProcess->pOverrides, ADIM_GETSEG (pAdimProcess->partName), 0.0) &&
          fabs(textAngle) > 0.01)
        {
        DVec3d      delta, xAxis, yAxis, zAxis;
        DPoint3d    center;
        RotMatrix   rMatrix, dimMatrix, invMatrix;

        xAxis = *pDirection;

        /* update text x-direction bvector with the angle rotation */
        rMatrix.InitIdentity ();
        rMatrix.InitFromPrincipleAxisRotations(rMatrix,  0.0,  0.0,  textAngle);
        /* rotation takes place on dimension plane. i.e. M = D x R x invD */
        rMatrix.InitProduct(pAdimProcess->rMatrix, rMatrix);
        invMatrix.InverseOf(pAdimProcess->rMatrix);
        rMatrix.InitProduct(rMatrix, invMatrix);
        rMatrix.Multiply(*pDirection);

        /* get dimension rotation matrix */
        pAdimProcess->rMatrix.GetColumn(zAxis,  2);
        bsiDVec3d_crossProduct (&yAxis, &zAxis, &xAxis);
        dimMatrix.InitFromColumnVectors(xAxis, yAxis, zAxis);

        /* find dimension text center-center point in world: */
        delta.x = pBoxSize->x / 2.0;
        delta.y =
        delta.z = 0.0;
        dimMatrix.Multiply(delta);
        bsiDPoint3d_addDPoint3dDPoint3d (&center, pOrigin, &delta);

        /*-------------------------------------------------------------------------------
        On text's plane, rotate text's left-center origin about text's center-center point,
        by dimension and text angles.  Then translate it to world.
        -------------------------------------------------------------------------------*/
        delta.x = -pBoxSize->x / 2.0;
        delta.y =
        delta.z = 0.0;
        dimMatrix.Multiply(delta);
        rMatrix.Multiply(delta);
        bsiDPoint3d_addDPoint3dDPoint3d (pOrigin, &delta, &center);
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name      adim_generateTextSymbols                                    |
|                                                                       |
| author    JVB                                         3/90            |
|                                                                       |
+----------------------------------------------------------------------*/
int BentleyApi::adim_generateTextSymbols
(
AdimProcess*    ep,         /* => Function used to process elements   */
double          charwdth,    /* => Character width                     */
DPoint2dP       textSize,   /* => Size of dimension text string       */
DPoint3dP       origin,     /* => Dimension text origin corresponding to the text justification */
DVec3dP         direction   /* => Dimension text direction            */
)
    {
    RotMatrix    rMatrix;
    DSegment3d uline;
    DPoint3d     rpnts[5], tmp_origin, saveOrigin;
    double       xoffset, yoffset, width;
    int          status, prevType, prevSub;
    bool         multiline;

#if 0
    //          ***TRICKY: we care about the overall dimension text justification, not the "current segment"
    //                  That's because we want a box around all of the text, whether it is composed of one
    //                  segment or many.
    //              In the long run, we should draw the text box/capsule
    //              AFTER drawing the text and then use the actual text box coordinates.
#endif

    yoffset = textSize->y + charwdth;
    xoffset = textSize->x + charwdth;

    saveOrigin = *origin;
    multiline  = ep->flags.textBlockPopulated;

    /* Shift the text box origin by text justification override */
    /* Only angular dimension and multiline need the shift.  All others assume input origin already LC justified */
    if (multiline || mdlDim_isAngularDimension(ep->GetElemHandleCR()))
        bsiDPoint3d_addScaledDVec3d (origin, origin, direction, BentleyApi::adim_computeLeftEdgeOffset (ep, 0.0, textSize->x));

    /*-----------------------------------------------------------------------------------
        BOUNDING BOX

           1 +-----+ 2
             |     |
        0,4  +-----+ 3

    -----------------------------------------------------------------------------------*/
    /* compute lower left corner (NB: This assumes origin is TextElementJustification::LeftMiddle or TextElementJustification::CenterMiddle or TextElementJustification::RightMiddle!) */
// TODO: vertical alignment !!!
    adim_offsetText (&rpnts[0], origin, direction, - yoffset / 2.0, ep);

    bsiDPoint3d_addScaledDVec3d (&rpnts[0], &rpnts[0], direction, -charwdth / 2.0);

    rpnts[4] = rpnts[0];

    adim_offsetText (&rpnts[1], &rpnts[0], direction, yoffset, ep);

    bsiDPoint3d_addScaledDVec3d (&rpnts[2], &rpnts[1], direction, xoffset);
    bsiDPoint3d_addScaledDVec3d (&rpnts[3], &rpnts[0], direction, xoffset);

    BentleyApi::adim_getRMatrixFromDir (&rMatrix, direction, &ep->rMatrix, &ep->vuMatrix);

    prevType = ADIM_GETTYPE (ep->partName);
    prevSub  = ADIM_GETSUB  (ep->partName);

    ADIM_SETNAME (ep->partName, ADTYPE_TEXT_SYMBOLS, ADSUB_NONE);

    /*-----------------------------------------------------------------------------------
        BOX
    -----------------------------------------------------------------------------------*/
    if (ep->GetDimElementCP()->flag.boxText)
        if (SUCCESS != (status = adim_generateLineString (ep, rpnts, 5, 1,
                                                          DIM_MATERIAL_DimLine)))
            return  status;

    /*-----------------------------------------------------------------------------------
        UNDERLINE
    -----------------------------------------------------------------------------------*/
    if (adim_getEffectiveUnderline (ep))
        {
        adim_offsetText (&uline.point[0], origin, direction, - ((textSize->y + charwdth / 2.0) / 2.0), ep);

        bsiDPoint3d_addScaledDVec3d (&uline.point[1], &uline.point[0], direction, textSize->x);

        if (SUCCESS != (status = adim_generateLine (ep, &uline.point[0], &uline.point[1], DIM_MATERIAL_DimLine)))
            return  status;
        }

    /*-----------------------------------------------------------------------------------
        CAPSULE
    -----------------------------------------------------------------------------------*/
    if (ep->GetDimElementCP()->flag.capsuleText)
        {
        double  rad = yoffset / 2.0;

        bsiDPoint3d_addScaledDVec3d (&rpnts[1], &rpnts[1], direction, charwdth);
        bsiDPoint3d_addScaledDVec3d (&rpnts[2], &rpnts[2], direction, -charwdth);
        bsiDPoint3d_addScaledDVec3d (&rpnts[3], &rpnts[3], direction, -charwdth);
        bsiDPoint3d_addScaledDVec3d (&rpnts[4], &rpnts[4], direction, charwdth);

        if (SUCCESS != (status = adim_generateLine (ep, rpnts+1, rpnts+2, DIM_MATERIAL_DimLine)))
            return  status;

        if (SUCCESS != (status = adim_generateLine (ep, rpnts+3, rpnts+4, DIM_MATERIAL_DimLine)))
            return  status;

        adim_offsetText (&tmp_origin, &rpnts[4], direction, yoffset/2.0, ep);

        if (SUCCESS != (status = adim_generateArc (ep, &tmp_origin, charwdth, rad, &rMatrix, msGeomConst_piOver2, msGeomConst_pi)))
            return  status;

        bsiDPoint3d_addScaledDVec3d (&tmp_origin,  &tmp_origin, direction, xoffset - 2.0*charwdth);

        if (SUCCESS != (status = adim_generateArc (ep, &tmp_origin, charwdth, rad, &rMatrix, - msGeomConst_piOver2, msGeomConst_pi)))
            return  status;
        }

    /*-----------------------------------------------------------------------------------
        ABOVE
    -----------------------------------------------------------------------------------*/
    if (ep->GetDimElementCP()->tmpl.above_symbol == 1)
        {
        DPoint3d    aPnts[3];

        /*-------------------------------------------------------------------------------
            ANSI style above symbol is defined to be:
            - 1.5 * H in width and
            - 0.3 * H in height
            It is not relative to text width.
        -------------------------------------------------------------------------------*/
        bsiDPoint3d_addScaledDVec3d (&tmp_origin, origin, direction, textSize->x / 2.0);

        adim_offsetText (&aPnts[1], &tmp_origin, direction, 1.2 * textSize->y, ep);
        bsiDPoint3d_addScaledDVec3d (&aPnts[0], &aPnts[1], direction,  0.75 * textSize->y);
        bsiDPoint3d_addScaledDVec3d (&aPnts[2], &aPnts[1], direction, -0.75 * textSize->y);

        adim_offsetText (&aPnts[0], &aPnts[0], direction, -0.3 * textSize->y, ep);
        adim_offsetText (&aPnts[2], &aPnts[2], direction, -0.3 * textSize->y, ep);

        if (SUCCESS != (status = adim_generateArcByPoints (ep, aPnts, DIM_MATERIAL_Text)))
            return  status;
        }

    *origin = saveOrigin;

    if (multiline)
        {
        // Legacy prefices and suffices are not supported with multilines - time to exit
        ADIM_SETNAME (ep->partName, prevType, prevSub);
        return  SUCCESS;
        }

    /*-----------------------------------------------------------------------------------
        PREFIX / SUFFIX
    -----------------------------------------------------------------------------------*/
    width = textSize->x;

    ADIM_SETNAME (ep->partName, ADTYPE_INHERIT, ADSUB_SUFFIX);
    if (SUCCESS != (status = generateCustomSymbol (ep, origin, direction,
                                                   width, false)))
        return  status;

    if (SUCCESS != (status = generateTemplateSymbol (ep, origin, direction,
                                                     width, false)))
        return  status;

    ADIM_SETNAME (ep->partName, ADTYPE_INHERIT, ADSUB_PREFIX);
    if (SUCCESS != (status = generateCustomSymbol (ep, origin, direction,
                                                   width, true)))
        return  status;

    if (SUCCESS != (status = generateTemplateSymbol (ep, origin, direction,
                                                     width, true)))
        return  status;

    if (ep->strDat.preSymMar)
        bsiDPoint3d_addScaledDVec3d (origin, origin, direction, ep->strDat.preSymMar);

    /*-----------------------------------------------------------------------------------
        Restore
    -----------------------------------------------------------------------------------*/
    ADIM_SETNAME (ep->partName, prevType, prevSub);

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    isDimensionOrientedForReadableText
(
AdimProcess const   *pAdimProcess
)
    {
    DVec3d          witnessVec, viewHoriVec, viewNormalVec, xAxis, zAxis;
    double          dimHeight = 0.0;

    /*-----------------------------------------------------------------------------------
    A readable text orientation is such a dimension whose witness bvector must point to
    positive y-axis of the view, i.e. condition 180 <= angle > 0 must be satisfied.
    When 180 < angle <= 0, text must be flipped up to a readable position.
    -----------------------------------------------------------------------------------*/
    pAdimProcess->vuMatrix.GetRow(viewHoriVec,  0);
    pAdimProcess->vuMatrix.GetRow(viewNormalVec,  2);
    viewHoriVec.Normalize ();
    viewNormalVec.Normalize ();

    /* get the bvector of witness lines extruding */
    pAdimProcess->rMatrix.GetColumn(xAxis,  0);
    pAdimProcess->rMatrix.GetColumn(zAxis,  2);
    xAxis.Normalize ();
    zAxis.Normalize ();
    bsiDVec3d_normalizedCrossProduct (&witnessVec, &zAxis, &xAxis);

    mdlDim_getHeightDirect (&dimHeight, pAdimProcess->GetElemHandleCR(), 0);
    if (dimHeight < 0.0 && DIM_LINEAR(pAdimProcess->GetDimElementCP()->dimcmd))
        bsiDVec3d_negate (&witnessVec, &witnessVec);

    /* a readable angle is between 0 ~ 180 and unreadable between 180~ 360*/
    if (bsiDVec3d_normalizedCrossProduct (&zAxis, &viewHoriVec, &witnessVec) > mgds_fc_epsilon)
        return  bsiDVec3d_dotProduct(&zAxis, &viewNormalVec) > 0.0;

    /*-----------------------------------------------------------------------------------
    handle boundary conditions - a vertical dimension on right (0 being unreadable or
    on left at 180 being readable.
    -----------------------------------------------------------------------------------*/
    return  bsiDVec3d_dotProduct(&witnessVec, &viewHoriVec) < 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/06
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isTextToBePushedRight
(
int     textJust,
bool    pushRight,
AdimProcess const   *pAdimProcess
)
    {
    if (DIM_ANGULAR(static_cast<DimensionType>(pAdimProcess->GetDimElementCP()->dimcmd)))
        return DIMTEXT_END == textJust;
    else
        return DIMTEXT_END == textJust || pushRight;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/06
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     isTextToBePushedLeft
(
int                 textJust,
bool                pushRight,
AdimProcess const   *pAdimProcess
)
    {
    if (DIM_ANGULAR(static_cast<DimensionType>(pAdimProcess->GetDimElementCP()->dimcmd)))
        return DIMTEXT_START == textJust;
    else
        return DIMTEXT_START == textJust || !pushRight;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/05
+---------------+---------------+---------------+---------------+---------------+------*/
static void    calcBallNChainTextOffset
(
DPoint2d&           offset,
int                 textJust,
bool                pushRight,
double              lineLength,     /*  => Length of dimension line          */
DPoint2dCP          pTextSize,     /*  => total text size                   */
DVec3dCP            pTextDir,      /*  => text running direction            */
DVec3dCP            pLineDir,      /*  => Dim line running direction        */
DimOffsetBlock*     pBncBlock,     /*  => ball & chain offset block         */
AdimProcess const*  pAdimProcess  /*  => Function used to process elements */
)
    {
    double      liftMargin = 0.5 * (pAdimProcess->GetDimElementCP()->text.height + pAdimProcess->GetDimElementCP()->geom.textLift);
    double      liftHeight = pAdimProcess->GetDimElementCP()->geom.witExtend + liftMargin + 0.5 * pAdimProcess->GetDimElementCP()->text.height;
    double      elbowLength = pAdimProcess->GetDimElementCP()->geom.margin ? pAdimProcess->GetDimElementCP()->geom.margin : pAdimProcess->strDat.charWidth;
    double      textMargin = pAdimProcess->GetDimElementCP()->geom.textMargin;
    double      dimHeight = 0, dotProduct = 0;
    int         chainType = NULL==pBncBlock ? DIMSTYLE_VALUE_BallAndChain_ChainType_None : pBncBlock->flags.chainType;
    bool        hasChain = DIMSTYLE_VALUE_BallAndChain_ChainType_None != chainType;
    bool        isTextPerp = false, isTextOutward = true, isTextParallel = false, isTextReadable = true;
    DVec3d      lineVec, liftVec, elbowVec, textVec;

    /*-----------------------------------------------------------------------------------
    Calculate default ball & chain offset when text must move under fit options:

    1) Imagine to create such a chain bvector that it starts from middle of dimension line
        and ends at the intersection with text def line and that it is alway perpendicular
        to the dimension line.
    2) Finding the offset is a simple bvector sum on the dimension plane:

        offsetVector = halfDimLineVector + chainLiftVector + elbowVector(optional)
                        (bvector 2.1)        (bvector 2.2)        (bvector 2.3)

    3) Alway move text "outside" of the dimension, i.e. the farthermost side along the
        dimension height direction.
    4) The origin for a linear dimension is the start point with input bvector pLineDir;
        while the origin for an angular dimension is at arc length center.
    5) When dim line and text are perpendicular, add an extra text margin.
    6) Offset does not include text margin
    -----------------------------------------------------------------------------------*/

    if (NULL == pTextDir || NULL == pLineDir)
        return;

    mdlDim_getHeightDirect (&dimHeight, pAdimProcess->GetElemHandleCR(), 0);

    if (DIM_ANGULAR(static_cast<DimensionType>(pAdimProcess->GetDimElementCP()->dimcmd)))
        {
        liftHeight = 2 * pAdimProcess->GetDimElementCP()->geom.termWidth;

        // keep text height always above dimension height:
        if (dimHeight < 0.0)
            liftHeight -= dimHeight;

        // for angular dimension with aligned text, the offset is constant:
        if (!pAdimProcess->GetDimElementCP()->flag.horizontal)
            {
            offset.x = 0.0;
            offset.y = liftHeight + pAdimProcess->GetDimElementCP()->geom.textLift + 0.5 * pAdimProcess->GetDimElementCP()->text.height;
            return;
            }
        }

    // check if dim line is perpendicular or parallel to text
    lineVec = *pLineDir;
    textVec = *pTextDir;
    lineVec.Normalize ();
    textVec.Normalize ();
    dotProduct = bsiDVec3d_dotProduct (&lineVec, &textVec);
    if (fabs(dotProduct) < mgds_fc_epsilon)
        isTextPerp = true;
    else if (fabs(1 - fabs(dotProduct)) < mgds_fc_epsilon)
        isTextParallel = true;

    // check if text is placed outward from dimension line
    if (DIM_LINEAR(pAdimProcess->GetDimElementCP()->dimcmd) && dimHeight < 0.0)
        isTextOutward = false;

    // lift height is different when text has to flip to a readable orientation
    isTextReadable = isDimensionOrientedForReadableText (pAdimProcess);
    if (!isTextReadable)
        liftHeight += pAdimProcess->GetDimElementCP()->text.height;

    // calculate elbow/inline length
    if (hasChain)
        {
        // there is a chain and may be also an elbow
        if (pBncBlock->flags.elbow)
            mdlDim_extensionsGetBncElbowLength(&elbowLength, pAdimProcess->pOverrides, elbowLength);
        else
            elbowLength = 0.0;

        // slant the chain by 0.3*lift height (~17by angle) for aligned text
        if (!pAdimProcess->GetDimElementCP()->flag.horizontal && DIM_LINEAR(pAdimProcess->GetDimElementCP()->dimcmd))
            elbowLength += 0.3 * liftHeight;

        // apply user pref to place text left/right, when text aligned with dim line
        if (isTextToBePushedLeft(textJust, pushRight, pAdimProcess) && isTextParallel)
            elbowLength = -elbowLength;
        }
    else
        {
        // there is no chain nor elbow - place text in the middle of dim line
        elbowLength = -pTextSize->x * 0.5;

        if (pAdimProcess->GetDimElementCP()->flag.horizontal)
            elbowLength -= textMargin;

        // when text running in reversed direction of dimension line, revert shift direction
        // do the same for vertical dim with inward text orientation
        if ((!pAdimProcess->GetDimElementCP()->flag.horizontal && 0.0 != elbowLength && fabs(dotProduct+1) < mgds_fc_epsilon) ||
            (isTextPerp && !isTextOutward))
            elbowLength = -elbowLength;
        }

    /*-------------------------------------------------------------------------------
    Get the elbow bvector which is along text direction. (bvector 2.3)
    -------------------------------------------------------------------------------*/
    elbowVec = *pTextDir;
    if (DIM_LINEAR(pAdimProcess->GetDimElementCP()->dimcmd))
        pAdimProcess->rMatrix.MultiplyTranspose(elbowVec);

    // flip text from right to left side of the chain line if text runs nose down toward the dimension line.
    if (hasChain && (elbowVec.y + mgds_fc_epsilon < 0.0))
        elbowLength = -elbowLength;
    // flip text again if the text would otherwise be placed "inside" of the dimension.
    // but do not flip for aligned text which has to run in its original readable direction.
    if (!isTextOutward && !isTextParallel)
        elbowLength = -elbowLength;

    bsiDVec3d_scaleInPlace (&elbowVec, elbowLength);

    /*-----------------------------------------------------------------------------------
    Get the chain bvector which is vertical to and runs outward from dimension line.
    (bvector 2.2)
    -----------------------------------------------------------------------------------*/
    liftVec.x = liftVec.z = 0.0;
    liftVec.y = liftHeight;
    if (!isTextOutward)
        liftVec.y = -liftVec.y;

    // sum of liftVec & textVec gives the bvector from dimension line middle point to text origin:
    bsiDPoint3d_addDPoint3dDPoint3d (&textVec, &liftVec, &elbowVec);

    // origin for linear dimension is at start def point and for angular dimension it is at arc center.
    if (DIM_LINEAR(pAdimProcess->GetDimElementCP()->dimcmd))
        {
        /*-------------------------------------------------------------------------------
        Get the half-dimension line bvector, which originates from the start dimension point
        to the center of the dimension line. (bvector 2.1)
        -------------------------------------------------------------------------------*/
        pAdimProcess->rMatrix.MultiplyTranspose(lineVec);
        bsiDVec3d_scaleInPlace (&lineVec, 0.5 * lineLength);

        // sum of this bvector & previous bvector results in bvector of dim origin to text origin:
        bsiDVec3d_addInPlace (&textVec, &lineVec);
        }

    offset.x = textVec.x;
    offset.y = textVec.y;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     BentleyApi::adim_calcTextOffset
(
DPoint2d&       offset,
int             textJust,
bool            pushRight,
double          lineLength,     /*  => Length of dimension line        */
double          stringLength,   /*  => Length of dimension string      */
double          charWidth,      /*  => Char width used in dim text     */
double          leader,         /*  => Leader length                   */
int             termMode,       /*  => Terminator mode                 */
DPoint2dCP      pTextSize,   /*  => total text size                 */
DVec3dCP        pTextDir,    /*  => Text running direction          */
DVec3dCP        pDimlineDir, /*  => Dim line running direction      */
AdimProcess*    pAdimProcess  /* <=> Function used to process elements */
)
    {
    DimStyleProp_FitOptions         fitOption = (DimStyleProp_FitOptions)pAdimProcess->flags.fitOption;
    bool                            bMoveTextNotFit = false, bMoveTextPerRequest = false, bMoveTextForInsideTerms = false, bAllowTextMove = false;
    DimStyleProp_BallAndChain_Mode  bncMode = DIMSTYLE_VALUE_BallAndChain_Mode_None;

    /*----------------------------------------------------------------
     If the dimension text location is not dictated by justification,
     it means the user has manually dragged the text by a distance
     that is stored in dimText->offset. In that case, we directly
     use that value.
    ----------------------------------------------------------------*/
    if (DIMTEXT_OFFSET == textJust)
        return;

    /* Otherwise compute dimText->offset purely based on the justification */

    /*-----------------------------------------------------------------------------------
    Allow text to move for fitting purpose for all auto-fitting options as well as fixed
    fitting options with auto ball & chain turned on:
    -----------------------------------------------------------------------------------*/
    BentleyApi::mdlDim_getBallNChainMode (&bncMode, pAdimProcess->GetElemHandleCR());
    bAllowTextMove = 0 == termMode || DIMSTYLE_VALUE_BallAndChain_Mode_Auto == bncMode;

    /* push text outside because it simply does not fit */
    bMoveTextNotFit = pAdimProcess->flags.textNotFit && bAllowTextMove && DIMSTYLE_VALUE_FitOption_KeepTextInside != fitOption;

    if (pAdimProcess->flags.allDontFit && bAllowTextMove)
        {
        /* or move text outside per fit option */
        bMoveTextPerRequest = DIMSTYLE_VALUE_FitOption_MoveTextFirst == fitOption || DIMSTYLE_VALUE_FitOption_MoveBoth == fitOption;

        /* or move text outside in order to keep terminators inside */
        bMoveTextForInsideTerms = pAdimProcess->flags.fitTermsInside && (DIMSTYLE_VALUE_FitOption_MoveTextFirst == fitOption || DIMSTYLE_VALUE_FitOption_MoveEither == fitOption);
        }

    if (bMoveTextNotFit || bMoveTextPerRequest || bMoveTextForInsideTerms)
        {
        DimOffsetBlock  *pBncBlock = (DimOffsetBlock*)mdlDim_getOptionBlock (pAdimProcess->GetElemHandleCR(), ADBLK_EXTOFFSET, NULL);

        /*-------------------------------------------------------------------------------
        Text won't fit between the extension lines:

        In a general case if there is a fit option we move text towards right; otherwise
        we move it towards left.

        In case of ball&chain toggle turned on, although no actual offset is set, we
        apply fit option to show ball&chain effect.
        -------------------------------------------------------------------------------*/
        if (DIMSTYLE_VALUE_BallAndChain_Mode_Auto == bncMode && NULL != pBncBlock)
            calcBallNChainTextOffset (offset, textJust, pushRight, lineLength, pTextSize, pTextDir, pDimlineDir, pBncBlock, pAdimProcess);
        else if (isTextToBePushedRight(textJust, pushRight, pAdimProcess))
            offset.x = lineLength + leader;
        else
            offset.x = -(stringLength + leader);

        /* save off the state that text has been push outside of witness lines */
        pAdimProcess->flags.pushTextOutside = true;
        }
    else if (DIM_ANGULAR(static_cast<DimensionType>(pAdimProcess->GetDimElementCP()->dimcmd)) && DIMSTYLE_VALUE_FitOption_KeepTextInside == fitOption && pAdimProcess->flags.textNotFit)
        {
        // force text inside and at the center
        offset.x = 0.5 * (stringLength - lineLength);

        pAdimProcess->flags.pushTextOutside = true;
        }
    else
        {
        switch (textJust)
            {
            case DIMTEXT_START:
                offset.x = 3.0 * charWidth;
                break;

            case DIMTEXT_CENTER:
                offset.x = 0.5 * (lineLength - stringLength);
                break;

            case DIMTEXT_END:
                offset.x = lineLength - 3.0 * charWidth - stringLength;
                break;
            }

        pAdimProcess->flags.pushTextOutside = false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::adim_getTileSize
(
DPoint2dP    textSize,
DPoint2dP    tolSize,
ElementHandleCR dimElement
)
    {
    DimTolrBlock    *tolBlock;
    DimensionElm const* dim = &dimElement.GetElementCP()->ToDimensionElm();
    if (textSize)
        {
        textSize->x = dim->text.width;
        textSize->y = dim->text.height;
        }

    if (tolSize)
        {
        if (NULL != (tolBlock = (DimTolrBlock*) mdlDim_getOptionBlock (dimElement, ADBLK_TOLERANCE, NULL)))
            {
            tolSize->x = tolBlock->tolWidth;
            tolSize->y = tolBlock->tolHeight;
            }
        else
            *tolSize = *textSize;
        }


#ifdef uncomp
    /*-------------------------------------------------------------------
    would be a nice display option - but if the dimension view is
    not open it will blow up in your face.
    -------------------------------------------------------------------*/
    UpdateInfo      *uiP;
    /*----------------------------------------------------------------
      The tile size will be divided by the reference file scale factor
      so that text and arrowheads in a dimension element in a scaled
      reference file will be the same size as those in the main file.
    ----------------------------------------------------------------*/
    uiP = (UpdateInfo *) ((GuiWindowP  ) guiWindow_viewWindowGet(dim->view))->userData2;

    if (textSize)
        {
        textSize->w = (double)dim->text.width  / uiP->refScale;
        textSize->h = (double)dim->text.height / uiP->refScale;
        }

    if (tolSize)
        {
        if (tolBlock = mdlDim_getOptionBlock (dim, ADBLK_TOLERANCE, NULL))
            {
            tolSize->w = tolBlock->tolWidth  / uiP->refScale;
            tolSize->h = tolBlock->tolHeight / uiP->refScale;
            }
        else
            *tolSize = *textSize;
        }
#endif

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  03/01
+---------------+---------------+---------------+---------------+---------------+------*/
int      BentleyApi::adim_convertUnits
(
double          *pDistanceUorOut,
double          uorsIn,
DimUnitBlock    *pDimUnitsIn,
DgnModelP    modelRef
)
    {
    StatusInt   status = SUCCESS;

    if  (! pDimUnitsIn->controlFlags.independentUnits)
        {
        // scale input uors from modelRef master units to dimension master units
        UnitDefinition      masterUnit = modelRef->GetModelInfo().GetMasterUnit();
        UnitDefinition      dimMasterUnit;
        
        BentleyApi::adim_extractUnitBlock (NULL, &dimMasterUnit, NULL, pDimUnitsIn);

        dimMasterUnit.ConvertDistanceFrom (*pDistanceUorOut, uorsIn, masterUnit);
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::adim_getDimensionUnits
(
DimUnitBlock*       pDimUnits,
WCharP              pMasterLabel,
WCharP              pSubLabel,
ElementHandleCR     dimElement,
bool                isPrimary
)
    {
    DimUnitBlock    *pTmpUnits;
    bool            bUseSafeDefaults = false;
    
    DimensionElm const* pDim = &dimElement.GetElementCP()->ToDimensionElm();
    if (pMasterLabel != NULL)
        pMasterLabel[0] = 0;
    if (pSubLabel != NULL)
        pSubLabel[0] = 0;

    DgnModelP dgnmodel = dimElement.GetDgnModelP ();
    
    BeAssert (NULL != dgnmodel);

    if (pDim->frmt.angleMeasure && mdlDim_isAngularDimension (dimElement))
        {
        bUseSafeDefaults = true;
        }
    else
    if (NULL != (pTmpUnits = (DimUnitBlock*) mdlDim_getOptionBlock (dimElement, isPrimary ? ADBLK_PRIMARY : ADBLK_SECONDARY, NULL)))
        {
        memcpy((void*)pDimUnits, (void*)pTmpUnits, sizeof(pDimUnits));
        }
    else
        {
        UnitDefinition      masterUnit = dgnmodel->GetModelInfo().GetMasterUnit();
        UnitDefinition      subUnit    = dgnmodel->GetModelInfo().GetSubUnit();

        if (0.0 != masterUnit.GetNumerator() && 0.0 != masterUnit.GetDenominator() &&
            0.0 != subUnit.GetNumerator() && 0.0 != subUnit.GetDenominator())
            {
            BentleyApi::adim_createUnitBlock (pDimUnits, true, NULL, &masterUnit, &subUnit);

            if (pMasterLabel)
                wcscpy (pMasterLabel, masterUnit.GetLabel().c_str());

            if (pSubLabel)
                wcscpy (pSubLabel, subUnit.GetLabel().c_str());
            }
        else
            {
            bUseSafeDefaults = true;
            }
        }

    if (bUseSafeDefaults)
        {
        memset (&pDimUnits->masterFlags, 0, sizeof (pDimUnits->masterFlags));
        pDimUnits->masterNumerator   = 1.0;
        pDimUnits->masterDenominator = 1.0;
        if (pMasterLabel)
            pMasterLabel[0] = 0;

        memset (&pDimUnits->subFlags, 0, sizeof (pDimUnits->subFlags));
        pDimUnits->subNumerator      = 1.0;
        pDimUnits->subDenominator    = 1.0;
        if (pSubLabel)
            pSubLabel[0] = 0;
        }

    /* The uor scale is never stored.  It is always derived from the model that
       contains the dimension */
    if (0.0 >= (pDimUnits->uorPerMast = dgnmodel->GetMillimetersPerMaster()))
        pDimUnits->uorPerMast = 1.0;

    /* reset master unit label if override is on */
    if (pMasterLabel)
        mdlDim_getUnitLabel (pMasterLabel, dimElement, isPrimary ? DIMLABEL_MASTUNIT : DIMLABEL_SECONDARY_MASTUNIT);
    /* reset sub unit label if override is on */
    if (pSubLabel)
        mdlDim_getUnitLabel (pSubLabel, dimElement, isPrimary ? DIMLABEL_SUBUNIT : DIMLABEL_SECONDARY_SUBUNIT);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
static void    getSymbolInfo
(
DimStringData   *dstr,
ElementHandleCR    dimElement
)
    {
    DimSymBlock  *symBlock;
    double       charWidth, symWidth, cellWidth, cellHeight;
    int          index, symNo;

    adim_getTileSize (&dstr->textTile, &dstr->tolTile, dimElement);
    dstr->charWidth = charWidth = (double)dstr->textTile.x;

    memset (dstr->auxSym, 0, sizeof (dstr->auxSym));

    /*-------------------------------------------------------------------
    Aux symbol 0 is the custom prefix symbol
    -------------------------------------------------------------------*/
    if (NULL != (symBlock = (DimSymBlock*) mdlDim_getOptionBlock (dimElement, ADBLK_PRESYMBOL, NULL)))
        {
        dstr->auxSym[0].type   = AUXSYM_SYMBOL;
        dstr->auxSym[0].symbol = symBlock->symChar;
        dstr->auxSym[0].font   = symBlock->symFont;
        adim_getCharWidth (&symWidth, symBlock->symChar, dstr->textTile.y, symBlock->symFont, dimElement);
        dstr->auxSym[0].width  = symWidth;
        }
    else if (SUCCESS == adim_getCellId (&dstr->auxSym[0].cellId, dimElement, DEPENDENCYAPPVALUE_PrefixCell))
        {
        dstr->auxSym[0].type   = AUXSYM_CELL;

        if (adim_findSharedCellDef (&cellWidth, &cellHeight, NULL, dstr->auxSym[0].cellId, dimElement.GetDgnModelP()))
            dstr->auxSym[0].width = 0.0;
        else
            dstr->auxSym[0].width = ((double) dstr->textTile.y / cellHeight) * cellWidth;
        }
    
    DimensionElm const* dim = &dimElement.GetElementCP()->ToDimensionElm();
    /*-------------------------------------------------------------------
    Aux symbol 1 is the built-in prefix symbol from the template
    -------------------------------------------------------------------*/
    dstr->auxSym[1].type   = AUXSYM_BUILTIN;
    dstr->auxSym[1].index  = static_cast <byte> (index = dim->tmpl.pre_symbol);

    if (index)
        dstr->auxSym[1].width = (index > 3 ? 2.0 : 1.0) * charWidth;

    /*-------------------------------------------------------------------
    Aux symbol 2 is the built in suffix symbol from the template
    -------------------------------------------------------------------*/
    dstr->auxSym[2].type   = AUXSYM_BUILTIN;
    dstr->auxSym[2].index  = static_cast <byte> (index = dim->tmpl.post_symbol);

    if (index)
        dstr->auxSym[2].width = (index > 3 ? 2.0 : 1.0) * charWidth;

    /*-------------------------------------------------------------------
    Aux symbol 3 is the custom suffix symbol
    -------------------------------------------------------------------*/
    if (NULL != (symBlock = (DimSymBlock*) mdlDim_getOptionBlock (dimElement, ADBLK_SUFSYMBOL, NULL)))
        {
        dstr->auxSym[3].type   = AUXSYM_SYMBOL;
        dstr->auxSym[3].symbol = symBlock->symChar;
        dstr->auxSym[3].font   = symBlock->symFont;
        adim_getCharWidth (&symWidth, symBlock->symChar, dstr->textTile.y, dstr->auxSym[3].font, dimElement);
        dstr->auxSym[3].width  = symWidth;
        }
    else if (SUCCESS == adim_getCellId (&dstr->auxSym[3].cellId, dimElement, DEPENDENCYAPPVALUE_SuffixCell))
        {
        dstr->auxSym[3].type   = AUXSYM_CELL;

        if (adim_findSharedCellDef (&cellWidth, &cellHeight, NULL, dstr->auxSym[3].cellId, dimElement.GetDgnModelP()))
            dstr->auxSym[3].width = 0.0;
        else
            dstr->auxSym[3].width = ((double) dstr->textTile.y / cellHeight) * cellWidth;
        }

    /*-------------------------------------------------------------------
    Determine margins and widths based on symbols
    -------------------------------------------------------------------*/
    dstr->preSymMar = dstr->auxSym[0].width || dstr->auxSym[1].width ? charWidth / 2.0 : 0.0;

    dstr->sufSymMar = dstr->auxSym[2].width || dstr->auxSym[3].width ? charWidth / 2.0 : 0.0;

    dstr->symWidth = 0.0;

    for (symNo=0; symNo < 4; symNo++)
        dstr->symWidth += dstr->auxSym[symNo].width;

    dstr->symWidth += dstr->preSymMar + dstr->sufSymMar;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::adim_getLengthStrings
(
AdimProcess    *pAdimProcess,   /* <=> */
double          dimLength,      /*  => Length of dimension line        */
UShort          primaryAccuracy
)
    {
    DimUnitBlock    dimUnits;
    DimPreSufBlock  *charBlock;
    UInt16          mainPrefix, mainSuffix, upperPrefix, upperSuffix, lowerPrefix, lowerSuffix;
    WChar         masterLabel[MAX_DIMSTR], subLabel[MAX_DIMSTR];

    DimensionElm const *dim         = pAdimProcess->GetDimElementCP();
    DgnModelP    modelRef    = pAdimProcess->GetDgnModelP();
    ElementHandleCR    dimElement = pAdimProcess->GetElemHandleCR();

    memset (&dimUnits, 0, sizeof(dimUnits));

    if (NULL != (charBlock = (DimPreSufBlock*) mdlDim_getOptionBlock (dimElement, ADBLK_PRESUF, NULL)))
        {
        mainPrefix  = charBlock->mainPrefix;
        mainSuffix  = charBlock->mainSuffix;
        upperPrefix = charBlock->upperPrefix;
        upperSuffix = charBlock->upperSuffix;
        lowerPrefix = charBlock->lowerPrefix;
        lowerSuffix = charBlock->lowerSuffix;
        }
    else
        {
        mainPrefix = mainSuffix = upperPrefix = upperSuffix = lowerPrefix = lowerSuffix = 0;
        }

    getSymbolInfo (&pAdimProcess->strDat, dimElement);
    adim_getDimensionUnits (&dimUnits, masterLabel, subLabel, dimElement, true);

    if (dim->flag.dual)
        {
        double      secConv = 1.0;

        /* get primary dim strings */
        getSingleLength (pAdimProcess, true, dimLength, upperPrefix, upperSuffix, 1.0, &dimUnits, masterLabel, subLabel, primaryAccuracy);

        /* get secondary units and labels */
        masterLabel[0] = subLabel[0] = 0;
        adim_getDimensionUnits (&dimUnits, masterLabel, subLabel, dimElement, false);

        adim_convertUnits (&secConv, 1.0, &dimUnits, modelRef);

        /* get secondary strings */
        getSingleLength (pAdimProcess, false, dimLength, lowerPrefix, lowerSuffix, secConv, &dimUnits, masterLabel, subLabel, primaryAccuracy);
        }
    else
        {
        getSingleLength (pAdimProcess, true, dimLength, mainPrefix, mainSuffix, 1.0, &dimUnits, masterLabel, subLabel, primaryAccuracy);

        memset (&pAdimProcess->strDat.lowerSize, 0, sizeof(pAdimProcess)->strDat.lowerSize);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
static void    getSingleLength
(
AdimProcess*    pAdimProcess,         /* <=> */
bool            primary,              /*  => true if primary (upper) dim   */
double          dimLength,            /* =>  Default dimension length      */
UInt16          prefix,               /* =>  One character prefix          */
UInt16          suffix,               /* =>  One character suffix          */
double          tolunits,
DimUnitBlock*   pDimUnits,
WCharCP         pMasterLabel,
WCharCP         pSubLabel,
UShort          primaryAccuracy
)
    {
    double      dRoundOff;
    DPoint2dP   pTextSize;
    WStringP    strings;

    DimOverrides  *pOverrides = pAdimProcess->pOverrides;
    DimStringData *dstr       = &pAdimProcess->strDat;

    if (primary)
        {
        pTextSize = &dstr->upperSize;
        strings   = dstr->GetPrimaryStrings();
        mdlDim_extensionsGetRoundOff (&dRoundOff, pOverrides, 0.0);
        }
    else
        {
        pTextSize = &dstr->lowerSize;
        strings   = dstr->GetSecondaryStrings();
        mdlDim_extensionsGetSecondaryRoundOff (&dRoundOff, pOverrides, 0.0);
        }
    
    ElementHandleCR    dimElement = pAdimProcess->GetElemHandleCR();
    DimensionElm const* dim = &dimElement.GetElementCP()->ToDimensionElm();
    if (dim->flag.tolerance)
        {
        if (dim->flag.tolmode)
            {
            getULLength (pAdimProcess, pTextSize, strings, dimLength, primary,
                         tolunits, pDimUnits, pMasterLabel, pSubLabel, primaryAccuracy);
            }
        else
            {
            getPMLength (pAdimProcess, pTextSize, strings, dimLength, primary,
                         prefix, suffix, tolunits, pDimUnits, pMasterLabel, pSubLabel, primaryAccuracy);
            }
        }
    else                /* straight non-toleranced single dimensioning */
        {
        adim_convertUnits (&dimLength, dimLength, pDimUnits, pAdimProcess->GetDgnModelP());

        adim_uorToDimString (strings[0], pDimUnits, pMasterLabel, pSubLabel, dimLength,
                             primary, prefix, suffix, 0, &dRoundOff, false, pAdimProcess, primaryAccuracy);

        adim_getStringSize (pTextSize,strings[0].c_str(), &dstr->textTile, dim->text.font, pAdimProcess);
        }

    pTextSize->x += dstr->symWidth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
int      BentleyApi::adim_insertLengthString
(
WString&      dimStg,     /* <=> string to be used in dimension       */
WCharCP       lenStr      /*  => length string for dimension string   */
)
    {
    if (dimStg.empty())                   /* Dimension string is blank - use  */
        {                               /* length string alone              */
        dimStg = lenStr;
        return  SUCCESS;
        }
    
    WString::size_type offset;
    if (WString::npos == (offset = dimStg.find(DIMMLTEXT_ValuePlaceHolder)))
        return SUCCESS;

    dimStg.replace(offset, 1, lenStr);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
static void    getPMLength
(
AdimProcess*        pAdimProcess,               /* <=> */
DPoint2dP           textSize,                   /* <=  Size of combined strings */
WString*            wStrings,                   /* <=> Strings to create or use */
double              dimLength,                  /* =>  Default dimension length */
bool                primary,                    /* =>  true if primary (upper) dim  */
UInt16              prefix,                     /* =>  One character prefix     */
UInt16              suffix,                     /* =>  One character suffix     */
double              tolunits,
DimUnitBlock*       dimUnits,
WCharCP           pMasterLabel,
WCharCP           pSubLabel,
UShort              primaryAccuracy
)
    {
    DimTolrBlock*   tolBlock;
    DPoint2d        adTile, tolTile;
    DPoint2d        plusSize, minusSize;
    double          rtmp, dRoundOff;
    short           prec;
    int             upperSign, lowerSign;
    double          upperTol, lowerTol;

    DgnModelP    modelRef   = pAdimProcess->GetDgnModelP();
    DimOverrides*   pOverrides = pAdimProcess->pOverrides;
    ElementHandleCR    dimElement = pAdimProcess->GetElemHandleCR();
    if (NULL == (tolBlock = (DimTolrBlock*) mdlDim_getOptionBlock (dimElement, ADBLK_TOLERANCE, NULL)))
        return;

    adim_getTileSize (&adTile, &tolTile, dimElement);
    adim_convertUnits (&dimLength, dimLength, dimUnits, modelRef);

    if (primary)
        mdlDim_extensionsGetRoundOff (&dRoundOff, pOverrides, 0.0);
    else
        mdlDim_extensionsGetSecondaryRoundOff (&dRoundOff, pOverrides, 0.0);

    adim_uorToDimString (wStrings[0], dimUnits, pMasterLabel, pSubLabel, dimLength,
                         primary, prefix, suffix, 0, &dRoundOff, false, pAdimProcess, primaryAccuracy);

    DimensionElm const* dim = &dimElement.GetElementCP()->ToDimensionElm();
    adim_getStringSize (textSize, wStrings[0].c_str(), &adTile, dim->text.font, pAdimProcess);

    prec = primary ? primaryAccuracy : dim->frmt.secondaryAccuracy;

    upperTol  = tolBlock->upperTol;
    if (upperTol < 0.0)
        {
        upperTol  = 0.0 - upperTol;
        upperSign = '-';
        }
    else if (upperTol == 0.0)
        {
        if (tolBlock->flags.signForZero)
            upperSign = (0 > tolBlock->lowerTol) ? '-' : '+';  // opposite of lower
        else
            upperSign = ' ';
        }
    else
        {
        upperSign = '+';
        }

    /*-------------------------------------------------------------------
    This seems backward but - the tolerances are always stored as positive
    values and the lower ones are negated (this dates back to the way the
    TCB was set up - pre-V4). Some customers figured that out and put in
    a negative lower tolerance to get it to come out as a  positive value.
    -------------------------------------------------------------------*/
    lowerTol = tolBlock->lowerTol;
    if (lowerTol > 0)
        {
        lowerSign = '-';
        }
    else if (lowerTol == 0L)
        {
        if (tolBlock->flags.signForZero)
            lowerSign = (0 > tolBlock->upperTol) ? '+' : '-';  // opposite of upper
        else
            lowerSign = ' ';
        }
    else
        {
        lowerTol = 0L - lowerTol;
        lowerSign = '+';
        }

    // Don't apply unit conversion for tolerances
    adim_uorToDimString (wStrings[1], dimUnits, pMasterLabel, pSubLabel, (double)(upperTol * tolunits),
                         primary, 0, 0, upperSign, NULL, true, pAdimProcess, primaryAccuracy);
    adim_uorToDimString (wStrings[2], dimUnits, pMasterLabel, pSubLabel, (double)(lowerTol * tolunits),
                         primary, 0, 0, lowerSign, NULL, true, pAdimProcess, primaryAccuracy);

    if (tolBlock->lowerTol || tolBlock->upperTol) /* one or the other is nonzero */
        {
        bool    stackedTol = tolBlock->flags.stackEqual || tolBlock->upperTol != tolBlock->lowerTol;

        adim_getStringSize (&plusSize,  wStrings[1].c_str(), &tolTile, dim->text.font, pAdimProcess);
        adim_getStringSize (&minusSize, wStrings[2].c_str(), &tolTile, dim->text.font, pAdimProcess);

        if (minusSize.x > plusSize.x)
            plusSize.x = minusSize.x;

        textSize->x += tolBlock->tolHorizSep + plusSize.x;

        if (stackedTol)
            {
            textSize->y  = tolTile.y * 2.0 + tolBlock->tolVertSep;
            adim_getCharWidth (&rtmp, tolBlock->prefix, textSize->y, dim->text.font, dimElement);
            textSize->x += rtmp;
            adim_getCharWidth (&rtmp, tolBlock->suffix, textSize->y, dim->text.font, dimElement);
            textSize->x += rtmp;
            }
        else
            {
            if (tolBlock->prefix)
                textSize->x += tolTile.x;

            if (tolBlock->suffix)
                textSize->x += tolTile.x;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
static void getULLength
(
AdimProcess*        pAdimProcess,           /* <=> */
DPoint2dP           textSize,               /* <=  Combined size of strings     */
WString*            strings,                /* <=> Strings to create or use     */
double              dimLength,              /*  => Length of dimension line     */
bool                primary,                /*  => true for primary dim        */
double              tolunits,
DimUnitBlock*       dimUnits,
WCharCP             pMasterLabel,
WCharCP             pSubLabel,
UShort              primaryAccuracy
)
    {
    DimTolrBlock*   tolBlock;
    DPoint2d        adTile;
    DPoint2d        minusSize;
    
    ElementHandleCR dimElement = pAdimProcess->GetElemHandleCR();
    if (NULL == (tolBlock = (DimTolrBlock*) mdlDim_getOptionBlock (dimElement, ADBLK_TOLERANCE, NULL)))
        return;

    adim_getTileSize (&adTile, NULL, dimElement);

    // Don't apply unit conversion for tolerances
    adim_convertUnits (&dimLength, dimLength, dimUnits, pAdimProcess->GetDgnModelP());

    adim_uorToDimString (strings[0], dimUnits, pMasterLabel, pSubLabel,
                         dimLength + (double)tolBlock->upperTol * tolunits,
                         primary, 0, 0, 0, NULL, false, pAdimProcess, primaryAccuracy);

    adim_uorToDimString (strings[1], dimUnits, pMasterLabel, pSubLabel,
                         dimLength - (double)tolBlock->lowerTol * tolunits,
                         primary, 0, 0, 0, NULL, false, pAdimProcess, primaryAccuracy);
    
    DimensionElm const* dim = &dimElement.GetElementCP()->ToDimensionElm();
    adim_getStringSize (textSize,   strings[0].c_str(), &adTile, dim->text.font, pAdimProcess);
    adim_getStringSize (&minusSize, strings[1].c_str(), &adTile, dim->text.font, pAdimProcess);

    if (minusSize.x > textSize->x)
        textSize->x = minusSize.x;

    if ((tolBlock->upperTol != 0) || (tolBlock->lowerTol != 0))
        textSize->y = 2.0 * adTile.y + tolBlock->tolVertSep;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    JoshSchifter    02/04
+---------------+---------------+---------------+---------------+---------------+------*/
static void    adim_getValueSeparators
(
WChar        *pDecimalSep,
WChar        *pThousandsSep,
bool            bMetric,
DimensionElm const *pDim
)
    {
    WChar     thousandsSep = 0;
    WChar     decimalSep   = '.';

    if (bMetric && pDim->frmt.decimalComma)
        decimalSep = ',';

    if (bMetric && pDim->frmt.metricSpc)
        {
        thousandsSep = ' ';

        if (pDim->flag.thousandSep)
            thousandsSep = pDim->frmt.decimalComma ? '.' : ',';
        }

    if (pDecimalSep)
        *pDecimalSep = decimalSep;

    if (pThousandsSep)
        *pThousandsSep = thousandsSep;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    JoshSchifter    02/04
+---------------+---------------+---------------+---------------+---------------+------*/
static void    adim_separateValueSegments
(
WChar        *pwString,
bool            bMetric,
DimensionElm const *pDim
)
    {
    WChar     thousandsSep = 0;
    WChar     decimalSep   = '.';

    adim_getValueSeparators (&decimalSep, &thousandsSep, bMetric, pDim);

    if ('.' != decimalSep)
        {
        WChar    *pDecimalLocation = wcsrchr (pwString, '.');

        if (NULL != pDecimalLocation)
            *pDecimalLocation = decimalSep;
        }

    if (0 != thousandsSep)
        {
        WChar         *pSrc, *pDst, *pEnd = pwString;
        WChar         wbuff[MAX_DIMSTR];

        wcscpy (wbuff, pwString);

        pDst = pwString;
        pSrc = pEnd = wbuff;

        while (*pEnd /* && *pEnd != decimalSep */ && iswdigit (*pEnd))
            pEnd++;

        while (pSrc < pEnd && (*pDst++ = *pSrc++))
            {
            ptrdiff_t dist = pEnd - pSrc;

            if (dist > 0 && dist % 3 == 0)
                *pDst++ = thousandsSep;
            }

        wcscpy (pDst, pEnd);
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    JoshSchifter    05/02
+---------------+---------------+---------------+---------------+---------------+------*/
static int     getScientificExponent
(
double          value
)
    {
    int  iExponent = 0;

    value = fabs (value);
    if (value < mgds_fc_epsilon)
        {
        iExponent = 0;
        }
    else
    if (value < 1.0)
        {
        while (value < 1.0 && iExponent > -308)
            {
            value *= 10.0;
            iExponent--;
            }
        }
    else
        {
        while (value >= 10.0 && iExponent < 308)
            {
            value /= 10.0;
            iExponent++;
            }
        }

    return iExponent;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static double  getMultiplier
(
DimensionElm const *pDim,
double          *pdRoundOff
)
    {
    return  (NULL != pdRoundOff) ? *pdRoundOff : 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static double  getEffectiveValue
(
DimensionElm const *pDim,
double          dNominalValue,
double          *pdRoundOff,
double          dDelta
)
    {
    double      dRoundOff;

    if (0.0 == (dRoundOff = getMultiplier (pDim, pdRoundOff)))
        return  dNominalValue;

    return  (round (dNominalValue / dRoundOff) * dRoundOff) + dDelta;
    }

typedef enum
    {
    DIMACCURACY_Standard    = 0,
    DIMACCURACY_Alternate   = 1,
    DIMACCURACY_Tolerance   = 2
    } DimResolvedAccuracy;

/*----------------------------------------------------------------------+
|                                                                       |
| name          convertToScientificPrecision                            |
|                                                                       |
| author        Don.Fu                                  09/03           |
|                                                                       |
+----------------------------------------------------------------------*/
static int     convertToScientificPrecision
(
int             precision
)
    {
    int         sciPrecision = 0;
    byte        digits;

    if (DIMACC_IsScientific(precision))
        return  precision;

    if (0 == precision)
        return  DIMACC_SCI_1;

    /* turn on the scientific bit: */
    sciPrecision |= DIMACC_Alt;

    for (digits = 0; digits < 8; digits++)
        {
        if (precision & 0x0001)
            break;

        precision >>= 1;
        }

    sciPrecision |= digits;

    return  sciPrecision;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          appendPrecisionStringInternal                           |
|                                                                       |
| author        RBB                                     11/86           |
|                                                                       |
+----------------------------------------------------------------------*/
static StatusInt   appendPrecisionStringInternal
(
WString&             wstr,              /* <= */
bool                *pIsValZero,        /* <= */
double              value,              /* => */
int                 precision,          /* => */
int                 addplus,            /* => */
bool                primary,            /* => */
bool                bNoReduceFraction,  /* => */
DimUnitBlock        *pDimUnits,         /* => */
double              *pdRoundOff,        /* => */
AdimProcess*        pAdimProc           /* => */
)
    {
    byte    leadingZero, trailingZeros;
    ScopedArray <WChar> wbuff(MAX_DIMSTRING);
    bool    bMetric = UnitSystem::Metric == static_cast<UnitSystem>(pDimUnits->masterFlags.system);
    bool valIsZero = false;
    
    DimensionElm const* dim = pAdimProc->GetDimElementCP();
    /*-------------------------------------------------------------------
    If the dimension version is less than 6 there was only one flag
    for leading and trailing zeros that controlled both primary and
    secondary dimensions. Starting with 6, (MS 5.5) the leading and
    trailing zero options are controlled separately for primary and
    secondary dimensions.
    -------------------------------------------------------------------*/
    if (primary)                        /* primary dimension (top)     */
        {
        leadingZero   = (byte)dim->flag.leadingZero;
        trailingZeros = (byte)dim->flag.trailingZeros;
        }
    else                                /* secondary dimension (bottom)*/
        {
        leadingZero   = (byte)dim->flag.leadingZero2;
        trailingZeros = (byte)dim->flag.trailingZeros2;
        }

    if (value < 0.0)
        {
        wstr.append(FMTw_dash);
        value *= -1.0;
        }
    else
        {
        if (addplus)
            {
            if (value  > 0.0 || !leadingZero)
                wstr.append(L"+");
            else
                wstr.append(FMTw_space);
            }
        }

    if (! precision)            /* show no fraction */
        {
        /* bail out to use scientific format, when measured value is more than 1e16 */
        value = getEffectiveValue (dim, value, pdRoundOff, 0.0);
        if (fabs(value) >= 1.e16)
            return  ERROR;
        
        BeStringUtilities::Snwprintf (wbuff.GetData(), MAX_DIMSTRING, FMTw_ld, (long) value);
        valIsZero = (0 == (long) value);

        adim_separateValueSegments (wbuff.GetData(), bMetric, dim);
        wstr.append(wbuff.GetData());
        }
    else if (precision & 0xc0)
        {                                       /* decimal display or scientific format */
        int         numDigits = 0, iExponent = 0;
        double      delta, dinDelta;
        bool        bIsScientific = DIMACC_IsScientific (precision);
        WChar     *delim, *pwTmp;

        delta = 0.499999999999;
        dinDelta = 0.0;

        if (!bIsScientific && dim->frmt.roundLSD && dim->frmt.superscriptLSD)
            dinDelta = 0.249999999999;

        if (bIsScientific)
            {
            iExponent = getScientificExponent (value);
            numDigits = DIMACC_SciPrecisionPwr (precision);

            value    /= pow (10.0, iExponent);
            }
        else
            {
            for (numDigits = 1; numDigits < 8; numDigits++)
                {
                if (precision & 0x0001)
                    break;

                precision >>= 1;
                }
            }

        if (!bIsScientific && numDigits && dim->frmt.roundLSD && dim->frmt.superscriptLSD)
            numDigits--;

        delta    /= pow (10.0, numDigits);
        dinDelta /= pow (10.0, numDigits);
        ScopedArray <WChar> fmt_str(MAX_DIMSTRING);
        BeStringUtilities::Snwprintf (fmt_str.GetData(), MAX_DIMSTRING, L"%%.%dlf", numDigits);

        /**
        * Should this be the similar with adim_convertUnits. Conversion is limited
        * to dimension values only, tolerance values are skipped. 4-20-01 pn
        */
        value = getEffectiveValue (dim, value, pdRoundOff, fabs (delta + dinDelta));

        /* bail out to use scientific format, when measured value is more than 1e16 for decimal units */
        if (!bIsScientific && fabs(value) >= 1.e16)
            return  ERROR;

        BeStringUtilities::Snwprintf (wbuff.GetData(), MAX_DIMSTRING, fmt_str.GetData(), fabs (value-delta-dinDelta));

        valIsZero = (0 == (long) (0.5 + ((value-delta-dinDelta) * pow (10.0, numDigits))));

        /* if necessary, strip trailing zeros */
        if (trailingZeros)
            {
            if (!bIsScientific && dim->frmt.superscriptLSD)
                superscriptLSD (wbuff.GetData(), dim, fabs(value)-delta, numDigits, pAdimProc);
            }
        else
            {
            wstripTrailingZeros (wbuff.GetData());
            }

        /* if necessary, strip leading zero */
        pwTmp = wbuff.GetData();
        if (wbuff.GetData()[0] == '0' &&  !leadingZero)
            pwTmp = wbuff.GetData()+1;

        if (true) //This block must occur AFTER strip leading zeros
            {
            WChar     decimalSep;

            adim_separateValueSegments (pwTmp, bMetric, dim);

            adim_getValueSeparators (&decimalSep, NULL, bMetric, dim);
            delim = wcsrchr (pwTmp, decimalSep);

            if (dim->frmt.omitLeadDelim && delim == pwTmp)
                {
                wcscpy (pwTmp, *(pwTmp+1) == '0' ? pwTmp+2 : pwTmp+1);
                if (NULL != pAdimProc->pDerivedData && NULL != pAdimProc->pDerivedData->pDelimiterOmitted)
                    pAdimProc->pDerivedData->pDelimiterOmitted[ADIM_GETSEG(pAdimProc->partName)] = true;
                }
            }

        /*---------------------------------------------------------------
        Make sure we don't end up with an empty string - from stripping
        zeros from 0.0 after formatting. Usually only happens in the base
        line of an ordinate dimension.
        ---------------------------------------------------------------*/
        if (*pwTmp == '\0')
            {
            pwTmp[0]  = '0';
            pwTmp[1]  = '\0';
            }

        wstr.append(pwTmp);

        if (bIsScientific)
            {
            BeStringUtilities::Snwprintf (wbuff.GetData(), MAX_DIMSTRING, L"E%+03d", iExponent);
            wstr.append (wbuff.GetData());
            }
        }
    else
        {                                       /* fractional display */
        long    ivalue;
        int     inumerator, idenominator;

        idenominator = 2*precision;
        value        = getEffectiveValue (dim, value, pdRoundOff, 0.0);

        /* bail out to use scientific format, when measured value is more than 1e10 for fractional units */
        if (fabs(value) >= 1.e10)
            return  ERROR;

        ivalue       = (long)(value);
        value       -= (double)ivalue;
        inumerator   = (int)(value*(double) idenominator);

        valIsZero = (0 == ivalue) && (0 == inumerator);

        if (ivalue || !inumerator || leadingZero)
            {
            BeStringUtilities::Snwprintf (wbuff.GetData(), MAX_DIMSTRING, FMTw_ld, ivalue);
            wstr.append (wbuff.GetData());
            }

        if (inumerator > 0)
            {
            bool    fromFont = DIMSTYLE_VALUE_Text_StackedFractionType_FromFont == dim->text.b.stackedFractionType;

            if (!bNoReduceFraction)
                {
                while ((inumerator/2)*2 == inumerator)
                    {
                    inumerator   /= 2;
                    idenominator /= 2;
                    }
                }

            wbuff.GetData()[0] = '\0';

            if (! dim->flag.stackFract)
                {
                // try to generate a stacked fraction string
                if ( ! fromFont)
                    {
                    // The \S in the string is a flag to later code that the string
                    // should be processed by the line arranger to contain a stacked
                    // fraction.  The denominator ends at the semicolon ";".
                    BeStringUtilities::Snwprintf (wbuff.GetData(), MAX_DIMSTRING, L"\\S%d/%d;", inumerator, idenominator);
                    }
                else
                    {
                    DgnFontCP font = DgnFontManager::ResolveFont(dim->text.font, pAdimProc->GetDgnModelP ()->GetDgnProject (), DGNFONTVARIANT_DontCare);
                    WChar fractionChar = 0;
                    if (DgnFontType::Rsc == font->GetType ())
                        {
                        FontChar fraction = static_cast<DgnRscFontCP>(font)->FractionToFontChar (static_cast <UInt8> (inumerator), static_cast <UInt8> (idenominator), false);

                        // Technically should be calling GetFontForCodePage, but this fraction character stuff
                        // only works for RSC fonts anyway, so there can't be a big font.
                        fractionChar = font->RemapFontCharToUnicodeChar (fraction);
                        }

                    BeStringUtilities::Snwprintf (wbuff.GetData(), MAX_DIMSTRING, L"%lc", fractionChar);
                    }
                }

            if ('\0' == wbuff.GetData()[0])
                {
                // non-stacked fraction
                BeStringUtilities::Snwprintf(wbuff.GetData(), MAX_DIMSTRING, L" %d/%d", inumerator, idenominator);

                // CR#42784 : for reasons unknown, this extra space has always been
                // added after the fraction.  To fix the CR, the space is now optional.
                if ( ! dim->frmt.noNonStackedSpace)
                    wcscat (wbuff.GetData(), L" ");
                }

            wstr.append (wbuff.GetData());
            }
        }

    if (NULL != pIsValZero)
        *pIsValZero = valIsZero;

    return  SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          appendPrecisionString                                   |
|                                                                       |
| author        RBB                                     11/86           |
|                                                                       |
+----------------------------------------------------------------------*/
static void    appendPrecisionString
(
WString&             wstr,              /* <= */
bool               *pIsValZero,        /* <= */
double              value,              /* => */
int                 precision,          /* => */
int                 addplus,            /* => */
bool                primary,            /* => */
bool                bNoReduceFraction,  /* => */
DimUnitBlock        *pDimUnits,         /* => */
double              *pdRoundOff,        /* => */
AdimProcess*        pAdimProc           /* => */
)
    {
    if (SUCCESS != appendPrecisionStringInternal(wstr, pIsValZero, value, precision, addplus, primary, bNoReduceFraction, pDimUnits, pdRoundOff, pAdimProc))
        {
        /* measured value found too big, force the precision to be scientific: */
        int sciPrecision = convertToScientificPrecision (precision);

        appendPrecisionStringInternal(wstr, pIsValZero, value, sciPrecision, addplus, primary, bNoReduceFraction, pDimUnits, pdRoundOff, pAdimProc);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
static void    getDimPrecision
(
double          *delta,
int             precision,
DimensionElm const *dim,
double          value
)
    {
    if (!precision)
        {
        *delta = 0.5 - DBL_EPSILON;
        }
    else if (precision & 0x80)      /* decimal display */
        {
        int         numDigits = 0;

        *delta    = 0.5 - DBL_EPSILON;
        BeAssert (!DIMACC_IsScientific (precision));

        for (numDigits = 1; numDigits < 8 && ! (precision & 0x0001); numDigits++)
            {
            if (precision & 0x0001)
                break;

            precision >>= 1;
            }
        
        if (numDigits && dim->frmt.roundLSD && dim->frmt.superscriptLSD)
            numDigits--;

        *delta   /= pow (10.0, numDigits);
        }
    else if (DIMACC_IsScientific (precision))       /* scientific display */
        {
        int     iExponent = getScientificExponent (value);
        int     numDigits = DIMACC_SciPrecisionPwr (precision);

        *delta = 0.5 - DBL_EPSILON;
        BeAssert(!dim->frmt.roundLSD || !dim->frmt.superscriptLSD);
        
        *delta   /= pow (10.0, numDigits - iExponent);
        }
    else                            /* fractional display */
        {
        *delta = (1.0 - DBL_EPSILON)/((double) precision * 4.0);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
static void    getUnitFormat
(
int                 *accuracyP,     /* <= accuracy */
bool                *bSubUnitsP,    /* <= Show sub units */
bool                *bLabelP,       /* <= Show unit labels(s) */
bool                *bDelimiterP,   /* <= Show delimter between master and sub */
bool                *bMastUnitsP,   /* <= Show master units */
bool                *bZeroMastP,    /* <= Show master units if zero */
bool                *bZeroSubP,     /* <= Show sub units if zero */
DimResolvedAccuracy *eAccuracyType, /* <= Accuracy Type used */
ElementHandleCR        dimElement,     /* => Current dimension element */
double              uors,           /* => Dimension Length in UORs */
bool                primary,        /* => true for primary units */
bool                tolerance,      /* => true for pm tolerance */
DimOverrides        *pOverrides,    /* => Dimension Overrides */
UShort              primaryAccuracy
)
    {
    int             blockType;
    UShort          accOverride=0;
    bool            hasAltFormat, hasAccExtension;
    DimAltFmtBlock  *altFmtBlockP;
    DimensionElm const* dimP = &dimElement.GetElementCP()->ToDimensionElm();
    if (!tolerance)
        {
        hasAccExtension = false;
        }
    else
        {
        if (primary)
            hasAccExtension = mdlDim_extensionsGetPrimaryTolAccuracy (&accOverride, pOverrides, 0);
        else
            hasAccExtension = mdlDim_extensionsGetSecondaryTolAccuracy (&accOverride, pOverrides, 0);
        }

    if (dimP->frmt.angleMeasure && mdlDim_isAngularDimension (dimElement))
        {
        if (hasAccExtension)
            {
            *accuracyP     = accOverride;
            *eAccuracyType = DIMACCURACY_Tolerance;
            }
        else
            {
            *accuracyP     = primaryAccuracy;
            *eAccuracyType = DIMACCURACY_Standard;
            }

        *bSubUnitsP  = false;
        *bLabelP     = false;
        *bDelimiterP = false;
        *bMastUnitsP = true;
        *bZeroMastP  = false;
        *bZeroSubP   = true;

        return;
        }

    if (primary)
        {
        if (hasAccExtension)
            {
            *accuracyP     = accOverride;
            *eAccuracyType = DIMACCURACY_Tolerance;
            }
        else
            {
            *accuracyP     = primaryAccuracy;
            *eAccuracyType = DIMACCURACY_Standard;
            }

        *bSubUnitsP  = dimP->frmt.adp_subunits;
        *bLabelP     = dimP->frmt.adp_label;
        *bDelimiterP = dimP->frmt.adp_delimiter;
        *bMastUnitsP = !dimP->text.b.adp_nomastunits;
        *bZeroMastP  = dimP->frmt.adp_allowZeroMast;
        *bZeroSubP   = !dimP->frmt.adp_hideZeroSub;

        blockType    = ADBLK_ALTFORMAT;
        hasAltFormat = dimP->text.b.hasAltFormat;
        }
    else
        {
        if (hasAccExtension)
            {
            *accuracyP     = accOverride;
            *eAccuracyType = DIMACCURACY_Tolerance;
            }
        else
            {
            *accuracyP     = dimP->frmt.secondaryAccuracy;
            *eAccuracyType = DIMACCURACY_Standard;
            }

        *bSubUnitsP  = dimP->frmt.adp_subunits2;
        *bLabelP     = dimP->frmt.adp_label2;
        *bDelimiterP = dimP->frmt.adp_delimiter2;
        *bMastUnitsP = !dimP->text.b.adp_nomastunits2;
        *bZeroMastP  = dimP->frmt.adp_allowZeroMast2;
        *bZeroSubP   = !dimP->frmt.adp_hideZeroSub2;

        blockType    = ADBLK_SECALTFORMAT;
        hasAltFormat = dimP->text.b.hasSecAltFormat;
        }

    if (hasAltFormat &&
        NULL != (altFmtBlockP = (DimAltFmtBlock*) mdlDim_getOptionBlock (dimElement, blockType, NULL)))
        {
        bool        useAltFmt = false;

#if defined (DEBUG_ALT_FORMAT)
        printf ("got %hs block %d%d%d%d%d %d - %d - %d\n",
                                                     ADBLK_ALTFORMAT == blockType ? "Primary  " : "Secondary",
                                                     altFmtBlockP->flags.adp_nomastunits,
                                                     altFmtBlockP->flags.adp_delimiter,
                                                     altFmtBlockP->flags.adp_label,
                                                     altFmtBlockP->flags.adp_subunits,
                                                     altFmtBlockP->flags.adp_allowZeroMast,
                                                     altFmtBlockP->flags.accuracy,
                                                     altFmtBlockP->flags.equalToThreshold,
                                                     altFmtBlockP->thresholdValue);
#endif


        if (fabs (uors - altFmtBlockP->thresholdValue) < mgds_fc_epsilon)
            useAltFmt = altFmtBlockP->flags.equalToThreshold;
        else if (uors > altFmtBlockP->thresholdValue)
            useAltFmt = altFmtBlockP->flags.greaterThanThreshold;
        else
            useAltFmt = !altFmtBlockP->flags.greaterThanThreshold;


        if (useAltFmt)
            {
            if (!hasAccExtension)
                {
                *accuracyP     = altFmtBlockP->flags.accuracy;
                *eAccuracyType = DIMACCURACY_Alternate;
                }

            *bSubUnitsP  = altFmtBlockP->flags.adp_subunits;
            *bLabelP     = altFmtBlockP->flags.adp_label;
            *bDelimiterP = altFmtBlockP->flags.adp_delimiter;
            *bMastUnitsP = !altFmtBlockP->flags.adp_nomastunits;
            *bZeroMastP  = altFmtBlockP->flags.adp_allowZeroMast;
            *bZeroSubP   = !altFmtBlockP->flags.adp_hideZeroSub;
            }
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 04/03
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    getNoReduceFractionFlag
(
bool                bPrimary,       /* => */
DimResolvedAccuracy eAccuracyType,  /* => */
DimOverrides        *pOverrides     /* => */
)
    {
    bool    bNoReduceFraction = false;

    switch (eAccuracyType)
        {
        case DIMACCURACY_Alternate:
            {
            if (bPrimary)
                mdlDim_extensionsGetNoReduceAltFractionFlag (&bNoReduceFraction, pOverrides, false);
            else
                mdlDim_extensionsGetNoReduceAltSecFractionFlag (&bNoReduceFraction, pOverrides, false);

            break;
            }

        case DIMACCURACY_Tolerance:
            {
            if (bPrimary)
                mdlDim_extensionsGetNoReduceTolFractionFlag (&bNoReduceFraction, pOverrides, false);
            else
                mdlDim_extensionsGetNoReduceTolSecFractionFlag (&bNoReduceFraction, pOverrides, false);

            break;
            }

        case DIMACCURACY_Standard:
        default:
            {
            if (bPrimary)
                mdlDim_extensionsGetNoReduceFractionFlag (&bNoReduceFraction, pOverrides, false);
            else
                mdlDim_extensionsGetNoReduceSecFractionFlag (&bNoReduceFraction, pOverrides, false);

            break;
            }
        }

    return bNoReduceFraction;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          adim_uorToDimString - generate the MU:SU:PU with        |
|                                    (optionally) units.                |
|                                                                       |
| author        RBB                                     11/86           |
|                                                                       |
+----------------------------------------------------------------------*/
void     BentleyApi::adim_uorToDimString
(
WString&        dimString,     /* <=> Dimension string                */
DimUnitBlock    *dimUnits,      /*  => Dimension MU:SU:PU definition   */
WCharCP       pMasterLabel,   /*  => Master unit label               */
WCharCP       pSubLabel,      /*  => Sub unit label                  */
double          uors,           /*  => Dimension length in uor's       */
bool            primary,        /*  => true if primary (upper) dim     */
UInt16          prefix,         /*  => Unit string prefix              */
UInt16          suffix,         /*  => Unit string suffix              */
int             pmChar,         /*  => plus, minus or zero             */
double          *pdRoundOff,    /*  => round off to nearest multiple or NULL */
bool            tolerance,      /*  => true if pm tolerance            */
AdimProcess*    pAdimProc,      /*  => context                         */
UShort          primaryAccuracy
)
    {
    int                 prec, iMasterUnits;
    WString             strBuf;
    double              masterUnits, subUnits, delta, uorsPerSub, subsPerMaster;
    bool                bSubUnits, bDelimiter, bLabel, bMastUnits, uorsNegative= false;
    bool                bAllowZeroMast, bAllowZeroSub;
    bool                bMetric = UnitSystem::Metric == static_cast<UnitSystem>(dimUnits->masterFlags.system);
    bool                bNoReduceFraction = false;
    DimOverrides*       pOverrides = pAdimProc->pOverrides;
    DimResolvedAccuracy eAccuracyType;
    
    DimensionElm const* dim = pAdimProc->GetDimElementCP();
    /* Check if the uors is negative. All the logic below assumes that
     * uor is positive, so change the sign now and add a '-' sign in
     * the string if necessary */
    if (uors < 0.0)
        {
        uors = fabs (uors);
        uorsNegative = true;
        }

    /*-----------------------------------------------------------------
      If a dimension string exists and has no wildcard character in it,
      there is nothing to fill in. (string was stored in element)
    -----------------------------------------------------------------*/
    if (!dimString.empty() && ! wcschr (&dimString[0], '*'))
        return;

    /* get unit format info, checking alt format and overrides if necessary */
    getUnitFormat (&prec, &bSubUnits, &bLabel, &bDelimiter, &bMastUnits, &bAllowZeroMast, &bAllowZeroSub,
                   &eAccuracyType, pAdimProc->GetElemHandleCR(), uors, primary, tolerance, pOverrides, primaryAccuracy);
    bNoReduceFraction = getNoReduceFractionFlag (primary, eAccuracyType, pOverrides);

    subsPerMaster = (dimUnits->subNumerator * dimUnits->masterDenominator)/
                    (dimUnits->subDenominator * dimUnits->masterNumerator);
    uorsPerSub    = dimUnits->uorPerMast / subsPerMaster;

    /* If SubUnits == MasterUnits, don't show sub units */
    if (1.0 == subsPerMaster)
        bSubUnits = false;

    if (DIMACC_IsScientific (prec))
        {
        // Don't allow master/sub dimensioning with scientific notation
        // it just doesn't make sense
        if (bMastUnits && bSubUnits)
            bSubUnits = false;

        // Don't allow din rounding with scientific notation
        // should probably be allowed, but I have no time to get it right
        //dim->frmt.roundLSD = dim->frmt.superscriptLSD = false;
        }

    getDimPrecision (&delta, prec, dim, bSubUnits ? uors / uorsPerSub : uors / dimUnits->uorPerMast);

    delta *= uorsPerSub;

    /* Apply subsPerMaster if subunits are NOT being displayed */
    if (!bSubUnits)
        delta *= subsPerMaster;

    subUnits    = (uors+delta)/uorsPerSub;
    masterUnits = subUnits/subsPerMaster;

    if (pmChar)
        strBuf.push_back((WChar)pmChar);

    /* If the uor is negative, put a '-' sign */
    if (!pmChar && uorsNegative)
        strBuf.push_back('-');

    if (!bSubUnits)
        {
        appendPrecisionString (strBuf, NULL, masterUnits, prec, false, primary, bNoReduceFraction, dimUnits, pdRoundOff, pAdimProc);
        if (bLabel && pMasterLabel)
            strBuf.append(pMasterLabel);
        }
    else
        {
        /*---------------------------------------------------------------------------
            Make sure the values are positive and manually assign negative sign.
        ---------------------------------------------------------------------------*/
        masterUnits = fabs (masterUnits);
        subUnits    = fabs (subUnits);

        if (fabs(masterUnits) >= 0xffffffff)
            iMasterUnits = 0;
        else
            iMasterUnits = bMastUnits ? (int) masterUnits : 0;
        subUnits -= (double) iMasterUnits * subsPerMaster;

        bool addMaster = iMasterUnits || bAllowZeroMast;

        if (addMaster)
            {
            ScopedArray <WChar> tmp(MAX_DIMSTRING);
            BeStringUtilities::Snwprintf (tmp.GetData(), MAX_DIMSTRING, FMTw_lu, (long)iMasterUnits);
            adim_separateValueSegments (tmp.GetData(), bMetric, dim);
            strBuf.append (tmp.GetData());
            if (bLabel && pMasterLabel)
                strBuf.append (pMasterLabel);
            }
        
        WString subBuf;
        bool     valIsZero;
        appendPrecisionString (subBuf, &valIsZero, subUnits, prec, false, primary, bNoReduceFraction, dimUnits, pdRoundOff, pAdimProc);

        bool addSub = !addMaster || bAllowZeroSub || !valIsZero;

        if (addSub && addMaster)
            {
            if (bDelimiter)
                strBuf.append (FMTw_dash);
            else
                strBuf.append (FMTw_space);
            }

        if (addSub)
            {
            strBuf.append (subBuf);

            if (bLabel && pSubLabel)
                strBuf.append (pSubLabel);
            }
        }

    if (dim->dimcmd != static_cast<byte>(DimensionType::Note))
        adim_insertLengthString (dimString, strBuf.c_str());

    addPrefix (dimString, prefix);
    addSuffix (dimString, suffix);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
static void    superscriptLSD
(
WCharP              wstr,
DimensionElm const* dim,
double              value,
int                 decimalPlaces,
AdimProcess*        pAdimProc
)
    {
    /*--------------------------------------------------------------------------
      Superscript Least Significant Digit

      DIN (German) standard dimensions use superscripted digits to indicate
      centimeters or millimeters.  The current implementation relies on the
      use of a specially designed font that substitutes superscripted digits
      for the symbols ")!@#$%^&*(".

      The roundLSD flag turns on the special DIN rounding behavior such that
        a) values from XX.00 <  val <= XX.25 are rounded to XX
        b) values from XX.25 <  val <  XX.75 are rounded to XX.5
        c) values from XX.75 <= val <  XX+1  are rounded to XX+1

      The roundLSD flag has no effect unless the supersciptLSD flag is ON.

      If the supersciptLSD flag is ON but the roundLSD flag is OFF, this allows
      for superscripting along with standard rounding.

      If both roundLSD and superscriptLSD are ON, the value string is rounded
      to one decimal place less than requested, and the superscripted 5 (%)
      is concatenated to the end.  A virtue of this approach is that the
      decimal point will not be in the string if one decimal place is requested.
      The produces the desirable output 10.5 -> 10% (instead of 10.%).

      The above description is based on interpretation of the (very old)
      existing code and should be taken with a grain of salt.  JS 05/02
    --------------------------------------------------------------------------*/
    WChar    *lastPtr = wstr + wcslen(wstr) - 1;
    WCharCP  superscripts = L")!@#$%^&*(";
    bool        fromFont = ! dim->text.b.superscriptMode;

    if (dim->frmt.roundLSD)
        {
        int iLoop;

        for (iLoop = 0; iLoop < decimalPlaces; iLoop++)
            value *= 10.0;

        value -= floor (value);
        if (value > .25 && value < .75)
            {
            if (fromFont)
                {
                lastPtr++;
                *lastPtr++ = superscripts[5];
                *lastPtr   = '\0';
                }
            else
                {
                lastPtr++;
                BeStringUtilities::Snwprintf (lastPtr, MAX_DIMSTRING, L"\\U5;");
                }

            if (NULL != pAdimProc->pDerivedData && NULL != pAdimProc->pDerivedData->pSuperscripted)
                pAdimProc->pDerivedData->pSuperscripted[ADIM_GETSEG(pAdimProc->partName)] = TRUE;
            }
        }
    else
        {
        int         lsdValue = *lastPtr - '0';
        WChar     decimalSep = '.';

        if (lastPtr > wstr && decimalSep == *(lastPtr-1))
            lastPtr--;

        *lastPtr = '\0';

        if (0 != lsdValue)
            {
            if (fromFont)
                BeStringUtilities::Snwprintf (lastPtr, MAX_DIMSTRING, L"%lc", superscripts[lsdValue]);
            else
                BeStringUtilities::Snwprintf (lastPtr, MAX_DIMSTRING, L"\\U%d;", lsdValue);

            if (NULL != pAdimProc->pDerivedData && NULL != pAdimProc->pDerivedData->pSuperscripted)
                pAdimProc->pDerivedData->pSuperscripted[ADIM_GETSEG(pAdimProc->partName)] = TRUE;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int     generateTemplateSymbol
(
AdimProcess     *ep,         /* => Function used to process elements   */
DPoint3d        *origin,     /* => Text origin                         */
DVec3d          *direction,  /* => Text direction                      */
double          length,      /* => Text length (including symbols)     */
int             first        /* => Set for symbol before text          */
)
    {
    DPoint2dP       tileSize;
    DimAuxSymbol*   auxSym;
    DimSymBlock*    diamBlock;
    RotMatrix       rMatrix;
    DSegment3d slash;
    DPoint3d        symorg;
    double          rad, width, charWidth;
    int             status;
    WChar         s_string[]  = {'S', 0};
    WChar         sr_string[] = {'S', 'R', 0};
    WChar         r_string[]  = {'R', 0};

    auxSym    = &ep->strDat.auxSym[first ? 1 : 2];
    tileSize  = &ep->strDat.textTile;
    charWidth = ep->strDat.charWidth;

    if (!auxSym->index)
        return  SUCCESS;

    diamBlock = (DimSymBlock*) mdlDim_getOptionBlock (ep->GetElemHandleCR(), ADBLK_DIAMSYM, NULL);
    BentleyApi::adim_getRMatrixFromDir (&rMatrix, direction, &ep->rMatrix, &ep->vuMatrix);

    if (first)
        {                               /* put the symbol first */
        symorg = *origin;
        bsiDPoint3d_addScaledDVec3d (origin, origin, direction, auxSym->width);
        }
    else
        {                               /* put the symbol last */
        width  = length - (ep->strDat.auxSym[3].width + auxSym->width);
        bsiDPoint3d_addScaledDVec3d (&symorg, origin, direction, width);
        }

    rad  = 0.5 * tileSize->y;

    switch (auxSym->index)
        {
        case 5: /* spherical diameter */
            if (SUCCESS != (status = adim_generateText (ep, s_string, &symorg, direction, TextElementJustification::LeftMiddle, tileSize)))
                return  status;
            bsiDPoint3d_addScaledDVec3d (&symorg, &symorg, direction, charWidth);
            /* Fall through to diameter case */

        case 1:
            {

//TODO Test generateTemplateSymbol in dynamics (BEIJING_DGNPLATFORM_WIP_JS)

            if (diamBlock && diamBlock->symChar)
                {
                status = adim_generateSymbol (ep, diamBlock->symFont, diamBlock->symChar,
                                              &symorg, &rMatrix, tileSize,
                                              TextElementJustification::LeftMiddle, DIM_MATERIAL_Text);

                adim_updateTextBox (ep, &symorg, direction, &rMatrix, tileSize);

                return status;
                }
            else
                {
                DPoint3d      tBoxOrigin;

                slash.point[0].x = slash.point[0].y = -rad;
                slash.point[1].x = slash.point[1].y = rad;
                slash.point[0].z = slash.point[1].z = 0.0;

                bsiDPoint3d_addScaledDVec3d (&symorg, &symorg, direction, rad);
                rMatrix.Multiply(&slash.point[0], &slash.point[0],  2);
                bsiDPoint3d_addDPoint3dArray (&slash.point[0], &symorg, 2);

                if (SUCCESS != (status = adim_generateLine (ep, &slash.point[0], &slash.point[1], DIM_MATERIAL_Text)))
                    return  status;

                status = adim_generateCircle (ep, &symorg, rad, &rMatrix, false, DIM_MATERIAL_Text);

                tBoxOrigin = slash.point[0];
                adim_updateTextBox (ep, &tBoxOrigin, direction, &rMatrix, tileSize);

                return status;
                }

            break;
            }

        case 2: /* radius symbol (R) */
            return (adim_generateText (ep, r_string, &symorg, direction, TextElementJustification::LeftMiddle, tileSize));
            break;

        case 3: /* square */
            {
            DPoint3d      tBoxOrigin;
            DPoint3d      rpoint[5];

            memset (rpoint, 0, sizeof(rpoint));
            rpoint[0].y = rpoint[3].y = -rad;
            rpoint[1].y = rpoint[2].y =  rad;
            rpoint[2].x = rpoint[3].x =  rad * 2.0;
            rpoint[4] = rpoint[0];

            rMatrix.Multiply(rpoint, rpoint,  5);
            bsiDPoint3d_addDPoint3dArray (rpoint, &symorg, 5);
            status = adim_generateLineString (ep, rpoint, 5, 1, DIM_MATERIAL_Text);

            tBoxOrigin = rpoint[0];
            adim_updateTextBox (ep, &tBoxOrigin, direction, &rMatrix, tileSize);

            return  status;
            break;
            }

        case 4: /* spherical radius */
            return (adim_generateText (ep, sr_string, &symorg, direction, TextElementJustification::LeftMiddle, tileSize));
            break;

        }

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int     generateCustomSymbol
(
AdimProcess *ap,
DPoint3d    *origin,         /* => Text origin                         */
DVec3d      *direction,      /* => Text direction                      */
double      length,          /* => Text length (including symbols)     */
int         first            /* => Set for symbol before text          */
)
    {
    DimAuxSymbol*   auxSym;
    DPoint2dP       adTile;
    RotMatrix       rMatrix;
    DPoint3d        symorg;
    int             status;

    auxSym = &ap->strDat.auxSym[first ? 0 : 3];
    adTile = &ap->strDat.textTile;

    if (!auxSym->type)
        return  SUCCESS;

    BentleyApi::adim_getRMatrixFromDir (&rMatrix, direction, &ap->rMatrix, &ap->vuMatrix);

    if (first)
        {
        symorg = *origin;
        bsiDPoint3d_addScaledDVec3d (origin, origin, direction, auxSym->width);
        }
    else
        {
        bsiDPoint3d_addScaledDVec3d (&symorg, origin, direction, length - auxSym->width);
        }

    //TODO Test generateCustomSymbol in dynamics (BEIJING_DGNPLATFORM_WIP_JS)
    if (auxSym->type == AUXSYM_SYMBOL)
        {
        status = adim_generateSymbol (ap, auxSym->font, auxSym->symbol,
                                      &symorg, &rMatrix, adTile, TextElementJustification::LeftMiddle, DIM_MATERIAL_Text);

        adim_updateTextBox (ap, &symorg, direction, &rMatrix, adTile);
        return  status;
        }
    else if (auxSym->type == AUXSYM_CELL)
        {
        status = adim_generateCell (ap, auxSym->cellId, &symorg, &rMatrix,
                                    auxSym->width, adTile->y, DIM_MATERIAL_Text);

        adim_updateTextBox (ap, &symorg, direction, &rMatrix, adTile);
        return  status;
        }

    return  SUCCESS;
    }


