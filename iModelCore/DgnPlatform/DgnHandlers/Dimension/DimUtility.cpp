/*----------------------------------------------------------------------+
|
|     $Source: DgnHandlers/Dimension/DimUtility.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#define SAFE_DOUBLE(x) ((x <= DBL_EPSILON && x >= -DBL_EPSILON) ? 1.0 : x)

USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
WString         AdimStringUtil::WStringFromVariChar (char const* inVariCharString, DgnFontCR effectiveFont)
    {
    WString uniString;
    VariCharConverter::VariCharToUnicode (uniString, static_cast<VariCharCP>(inVariCharString), VariCharConverter::ComputeNumBytes (inVariCharString), effectiveFont.GetCodePage ());

    return uniString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     06/08
+---------------+---------------+---------------+---------------+---------------+------*/
WString         AdimStringUtil::WStringFromVariChar (char const * inVariCharString, UInt16 fontNumber, UInt16 shxBigFontNumber, DgnProjectR dgnFile)
    {
    DgnFontCR      effectiveFont       = DgnFontManager::GetFontForCodePage (fontNumber, shxBigFontNumber, dgnFile);
    return WStringFromVariChar (inVariCharString, effectiveFont);
    }

/*---------------------------------------------------------------------------------**//**
* Converts the given WChar string to a VariChar string, using the given fonts to convert
*   from Unicode to locale-encoding.
* @param    outVariCharString   IN/OUT  Resulting VariChar string buffer; unless outNumBytes is 0, must be pre-allocated
* @param    outNumBytes         IN      Size of the output buffer, in bytes, including terminating NULL
* @param    inUnicodeString     IN      Input NULL-terminated WChar string buffer
* @param    fontNumber          IN      Potential 'small' font to use to convert Unicode to locale-encoding
* @param    bigFontNumber       IN      Potential 'big' font to use to convert locale-encoding to Unicode
* @return If outNumBytes is 0, this will return the output buffer size needed, in bytes,
*           including the NULL terminator. Otherwise, this will return the number of bytes
*           written to the output buffer.
* @bsimethod                                                    Jeff.Marker     06/08
+---------------+---------------+---------------+---------------+---------------+------*/
void AdimStringUtil::VariCharFromMSWChar (char* outVariCharString, size_t outNumBytes, WCharCP inUnicodeString, DgnFontCR effectiveFont)
    {
    if (0 == outNumBytes)
        return;
    
    bvector<VariChar> variBuffer;
    VariCharConverter::UnicodeToVariChar (variBuffer, inUnicodeString, effectiveFont.GetCodePage (), true);

    if (variBuffer.size () <= outNumBytes)
        {
        memcpy (outVariCharString, &variBuffer[0], variBuffer.size ());
        return;
        }
    
    // This would take character analysis to get right under every scenario (e.g. don't chop in the middle of a multi-byte sequence)...
    //  I don't think it's worth it at this time. Over-compensate to get something.
    BeAssert (false);
    
    // Depending on conversion and inducers, may need one or two zeroes... in this error condition, just put two zeroes to get something that won't crash.
    // !!! WARNING !!!
    // !!! WARNING !!! This can corrupt the last character.
    // !!! WARNING !!!
    memcpy (outVariCharString, &variBuffer[0], outNumBytes - 2);
    outVariCharString[outNumBytes - 2] = 0;
    outVariCharString[outNumBytes - 1] = 0;
    }

/*---------------------------------------------------------------------------------**//**
* Converts the given WChar string to a VariChar string, using the given fonts to convert
*   from Unicode to locale-encoding.
* @param    outVariCharString   IN/OUT  Resulting VariChar string buffer; unless outNumBytes is 0, must be pre-allocated
* @param    outNumBytes         IN      Size of the output buffer, in bytes, including terminating NULL
* @param    inUnicodeString     IN      Input NULL-terminated WChar string buffer
* @param    fontNumber          IN      Potential 'small' font to use to convert Unicode to locale-encoding
* @param    bigFontNumber       IN      Potential 'big' font to use to convert locale-encoding to Unicode
* @return If outNumBytes is 0, this will return the output buffer size needed, in bytes,
*           including the NULL terminator. Otherwise, this will return the number of bytes
*           written to the output buffer.
* @bsimethod                                                    Jeff.Marker     06/08
+---------------+---------------+---------------+---------------+---------------+------*/
void AdimStringUtil::VariCharFromMSWChar (char* outVariCharString, size_t outNumBytes, WCharCP inUnicodeString, UInt16 fontNumber, UInt16 bigFontNumber, DgnProjectR dgnFile)
    {
    DgnFontCR  effectiveFont = DgnFontManager::GetFontForCodePage (fontNumber, bigFontNumber, dgnFile);

    VariCharFromMSWChar (outVariCharString, outNumBytes, inUnicodeString, effectiveFont);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name      mdlDim_getHeight                                            |
|                                                                       |
| author    JVB                                         10/90           |
|                                                                       |
|           Moved out of mdlDim_getHeight - DHF                         |
|                                                                       |
+----------------------------------------------------------------------*/
int      BentleyApi::mdlDim_getHeightDirect
(
double*       pHeight,
ElementHandleCR dimElement,
int           relative
)
    {
    DimensionElm const* pDim = &dimElement.GetElementCP()->ToDimensionElm();
    switch (static_cast<DimensionType>(pDim->dimcmd))
        {
        case DimensionType::SizeArrow:
        case DimensionType::SizeStroke:
        case DimensionType::LocateSingle:
        case DimensionType::LocateStacked:
        case DimensionType::CustomLinear:
            {
            if (0 >= pDim->nPoints)
                return ERROR;

            *pHeight = pDim->GetDimTextCP(0)->offset;
            break;
            }

        case DimensionType::AngleSize:
        case DimensionType::AngleLocation:
        case DimensionType::AngleLines:
        case DimensionType::AngleAxisX:
        case DimensionType::AngleAxisY:
        case DimensionType::ArcSize:
        case DimensionType::ArcLocation:
            {
            if (1 >= pDim->nPoints)
                return ERROR;

            *pHeight = pDim->GetDimTextCP(1)->offset;
            break;
            }

        default:
            return (ERROR);
        }

    return  SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name      mdlDim_setHeightDirect                                      |
|                                                                       |
| author    JVB                                         9/92            |
|                                                                       |
+----------------------------------------------------------------------*/
int BentleyApi::mdlDim_setHeightDirect
(                                   /* <=  SUCCESS if dimension has height; else ERROR */
EditElementHandleR  dimElement,     /*  => */
double              height,         /*  => new value of height to use */
int                 relative        /*  => */
)
    {
    DimensionElm * dim = &dimElement.GetElementP()->ToDimensionElmR();
    switch (static_cast<DimensionType>(dim->dimcmd))
        {
        case DimensionType::SizeArrow:
        case DimensionType::SizeStroke:
        case DimensionType::LocateSingle:
        case DimensionType::LocateStacked:
        case DimensionType::CustomLinear:
            dim->GetDimTextP(0)->offset = height;
            break;

        case DimensionType::AngleSize:
        case DimensionType::AngleLocation:
        case DimensionType::AngleLines:
        case DimensionType::AngleAxisX:
        case DimensionType::AngleAxisY:
        case DimensionType::ArcSize:
        case DimensionType::ArcLocation:
            dim->GetDimTextP(1)->offset = height;
            break;

        default:
            return (ERROR);
        }

    return (SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/03
+---------------+---------------+---------------+---------------+---------------+------*/
int      BentleyApi::mdlDim_getTextOffsetDirect
(
DPoint2d           *pPoint,     /* <= */
int                *pJustify,   /* <= */
ElementHandleCR        dimElement,  /* => */
int                 segNo       /* => */
)
    {
    int         pointNo;

    if (SUCCESS != BentleyApi::mdlDim_getTextPointNo (&pointNo, dimElement, segNo))
        return (ERROR);

    DimensionElm const* dim = &dimElement.GetElementCP()->ToDimensionElm();
    if (pJustify)
        *pJustify = dim->GetDimTextCP(pointNo)->flags.b.just;

    if (pPoint)
        {
        pPoint->x = dim->GetDimTextCP(pointNo)->offset;
        pPoint->y = dim->GetDimTextCP(pointNo)->offsetY;
        }

    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name      mdlDim_initDimTextFromStyle                                 |
|                                                                       |
| author    JVB                                         4/90            |
|                                                                       |
+----------------------------------------------------------------------*/
void     BentleyApi::mdlDim_initDimTextFromStyle
(
DimText             *dimTextP,
DimensionStyleCP    dgnDimStyleP
)
    {
    dimTextP->offset  = 0L;
    dimTextP->offsetY = 0L;
    dimTextP->address = -1;
    dimTextP->flags.s = 0;
    dimTextP->flags.b.pushTextRight = false;

    if (NULL != dgnDimStyleP)
        {
        int just;
        dgnDimStyleP->GetIntegerProp (just, DIMSTYLE_PROP_Text_Justification_INTEGER);
        dimTextP->flags.b.just       = just;
        bool nowitness;
        dgnDimStyleP->GetBooleanProp (nowitness, DIMSTYLE_PROP_ExtensionLine_ShowAny_BOOLINT);
        dimTextP->flags.b.noWitness  = !nowitness;
        bool altColor, altWeight, altStyle;
        dgnDimStyleP->GetBooleanProp (altColor, DIMSTYLE_PROP_ExtensionLine_OverrideColor_BOOLINT);
        dgnDimStyleP->GetBooleanProp (altWeight, DIMSTYLE_PROP_ExtensionLine_OverrideWeight_BOOLINT);
        dgnDimStyleP->GetBooleanProp (altStyle, DIMSTYLE_PROP_ExtensionLine_OverrideLineStyle_BOOLINT);
        
        dimTextP->flags.b.altSymb    = (altColor || altWeight || altStyle);

        dimTextP->flags.b.pushTextRight = DIMTEXT_CENTER == just;
        }
    else
        {
        dimTextP->flags.b.just       = DIMTEXT_CENTER;
        dimTextP->flags.b.noWitness  = false;
        dimTextP->flags.b.altSymb    = false;
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name      adim_initDimText                                            |
|                                                                       |
| author    JVB                                         4/90            |
|                                                                       |
+----------------------------------------------------------------------*/
DGNPLATFORM_EXPORT void BentleyApi::adim_initDimText
(
ElementHandleCR     dimElement,
int                 iPoint,
DimText *           dimText,
DimensionStyleCR    dimStyle
)
    {
    mdlDim_initDimTextFromStyle (dimText, &dimStyle);

    // When inserting datapoints, compute the noWitness flag accurately from the
    // style and template.
    // 1. The style's "ad1.mode.witness" flag only tells you if witness lines
    //    are permitted.
    // 2. If the style allows it, we have to further look at the dimension element's
    //    "tmpl.left_witness or tmpl.right_witness" flags to decide if a specific
    //    point can have witness line (keeping in mind that a point can override
    //    the value with witCtrlLocal shield).
    
    bool witness;
    dimStyle.GetBooleanProp (witness, DIMSTYLE_PROP_ExtensionLine_ShowAny_BOOLINT);
    adim_setDimTextWitnessLineFromTemplate (dimElement, dimText, iPoint, !witness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/01
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::adim_getYVec
(
DVec3d        *pYDir,
const DVec3d  *pXDir,
AdimProcess     *pAdimProcess
)
    {
    if (pAdimProcess->GetDimElementCP()->Is3d())
        {
        RotMatrix   rMatrix;
        RotMatrix  *pDimRMatrix = &pAdimProcess->rMatrix;
        RotMatrix  *pViewRMatrix = &pAdimProcess->vuMatrix;

        BentleyApi::adim_getRMatrixFromDir (&rMatrix, pXDir, pDimRMatrix, pViewRMatrix);

        rMatrix.GetColumn(*pYDir,  1);
        }
    else
        {
        pYDir->x  = - pXDir->y;
        pYDir->y  =   pXDir->x;
        pYDir->z  =   0.0;
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          offset_txtorg                                           |
|                                                                       |
| author        RBB                                     12/86           |
|                                                                       |
+----------------------------------------------------------------------*/
void     BentleyApi::adim_offsetText
(
DPoint3d*               output_origin,    /* <= point to be offset             */
DPoint3d const* const   input_origin,     /* => point to be offset             */
DVec3d const* const     direction,        /* => line perpindicular to offset  */
double                  distance,         /* => distance to offset             */
AdimProcess*            pAdimProcess
)
    {
    DVec3d              perpvec;
    DPoint3d            tmpOrigin;

    tmpOrigin = *input_origin;

    adim_getYVec (&perpvec, direction, pAdimProcess);

    bsiDPoint3d_addScaledDVec3d (output_origin, &tmpOrigin, &perpvec, distance);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name      mdlDim_getOptionBlock                                       |
|                                                                       |
| author    JVB                                         7/90            |
|                                                                       |
+----------------------------------------------------------------------*/
void const* BentleyApi::mdlDim_getEditOptionBlockFromElement
(
DgnElementCR    liveElement, /* => Dimension element block is in  */
ElementHandleCR dimElement,
int             reqType,    /* => Requested block type           */
UInt64*      cellID           
)
    {
    //WIP: The option block code needs to re written to return a status for SUCCESS or ERROR
    DimensionElm const* dim = &liveElement.ToDimensionElm();
    DimOptionBlockHeader const* blockPtr = reinterpret_cast<DimOptionBlockHeader const*>(dim->GetDimViewBlockCP());

    for (int i=0; i<(int) dim->nOptions; i++)
        {
        if (blockPtr->type == reqType)
            {
            /* When getting a terminator block...need to get cell ids from dependency linkage */
            if (reqType == ADBLK_TERMINATOR)
                {
                DimTermBlock const* termBlockP = (DimTermBlock* const) blockPtr;

                if ((NULL != cellID) && termBlockP->flags.arrow > 1)
                    {
                    if (SUCCESS != adim_getCellId (&cellID[DEPENDENCYAPPVALUE_ArrowHeadTerminator-1], dimElement, DEPENDENCYAPPVALUE_ArrowHeadTerminator))
                        cellID[DEPENDENCYAPPVALUE_ArrowHeadTerminator-1] = INVALID_ELEMENTID;
                    }

                if ((NULL != cellID) && termBlockP->flags.stroke > 1)
                    {
                    if (SUCCESS != adim_getCellId (&cellID[DEPENDENCYAPPVALUE_StrokeTerminator-1], dimElement, DEPENDENCYAPPVALUE_StrokeTerminator))
                        cellID[DEPENDENCYAPPVALUE_StrokeTerminator-1] = INVALID_ELEMENTID;
                    }

                if ((NULL != cellID) && termBlockP->flags.origin > 1)
                    {
                    if (SUCCESS != adim_getCellId (&cellID[DEPENDENCYAPPVALUE_OriginTerminator-1], dimElement, DEPENDENCYAPPVALUE_OriginTerminator))
                        cellID[DEPENDENCYAPPVALUE_OriginTerminator-1] = INVALID_ELEMENTID;
                    }

                if ((NULL != cellID) && termBlockP->flags.dot > 1)
                    {
                    if (SUCCESS != adim_getCellId (&cellID[DEPENDENCYAPPVALUE_DotTerminator-1], dimElement, DEPENDENCYAPPVALUE_DotTerminator))
                        cellID[DEPENDENCYAPPVALUE_DotTerminator-1] = INVALID_ELEMENTID;
                    }
                }

            return (blockPtr);
            }

        blockPtr += blockPtr->nWords;
        }

    if (reqType == ADBLK_TEXT)
        return (blockPtr);

    return (NULL);
    }

/*---------------------------------------------------------------------------------**//**
//This function should not be called if the returned value is to be used for pointer
arithematic. There is no more space in an elemedescr to add a new one.
* @bsimethod                                    Abeesh.Basheer                  11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void const* BentleyApi::mdlDim_getOptionBlock (ElementHandleCR dimElement, int reqType, UInt64* cellID)
    {
    return mdlDim_getEditOptionBlockFromElement(*dimElement.GetElementCP(), dimElement, reqType, cellID);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::adim_getTextSize
(
DPoint2dP           pTextSize,
AdimProcess*        pAdimProcess,
DimStringData*      pDstr,
AdimTextSizeOption  option
)
    {
    DimMLText   *pText = NULL;

    /*---------------------------------------------------------------------------
        Added the 'option' parameter here to fix TR#318871.  The text size is
        used to position the text, so the positioning is sensitive to differences
        between the TextBlock and non-TextBlock (TextString) methods of computing
        the size.  Nominal (aka TextBlock::GetLocalRange) is closer to the
        TextString method than Exact (aka TextBlock::GetExactLocalRange).

        Probably most (all?) of the caller should be switched over to request
        the Nominal range but for now being conservative and only switching
        a couple.   JS 07/11
    ---------------------------------------------------------------------------*/

    if (NULL != pAdimProcess && dimTextBlock_getRequiresTextBlock (pAdimProcess, &pText))
        {
        if (! pAdimProcess->flags.textBlockPopulated)
            dimTextBlock_populateWithValues (pAdimProcess, pText, pDstr->GetPrimaryStrings());

        dimTextBlock_getTextSize (pAdimProcess, pTextSize, option);

        return;
        }

    pTextSize->x = pDstr->upperSize.x > pDstr->lowerSize.x ?
                   pDstr->upperSize.x : pDstr->lowerSize.x;

    pTextSize->y = pDstr->upperSize.y;

    /*---------------------------------------------------------------------------
        I can't justify why we would return upperSize.y + lowerSize.y here,
        but that's how it's been for a long time.  Since the goal of nominal
        mode is to match the TextString and TextBlock methods, don't do that
        for nominal mode.  However, it appears to me that some of the Exact
        callers do depend on this.   JS 07/11
    ---------------------------------------------------------------------------*/

    if (ADIM_TEXTSIZE_Exact == option)
        {
        if (pDstr->lowerSize.y)
            pTextSize->y += pDstr->lowerSize.y;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     11/89
+---------------+---------------+---------------+---------------+---------------+------*/
int      BentleyApi::adim_extractTextCluster
(
WString*            strings,                /* <= Dimension strings         */
DimStringConfig*    stringConfig,           /* <= Text configuration        */
ElementHandleCR     dimElement,             /* => Dimension element buffer  */
int                 pointNo,                /* => Point number of text      */
DgnFontCR           effectiveFont           /* => Font used for encoding varichar */
)
    {
    DimensionElm const* dim = &dimElement.GetElementCP()->ToDimensionElm();
    if (pointNo >= dim->nPoints)
        return ERROR;

    DimTolrBlock *tolBlock;
    DimText const *dimtxt;
    int          i;
    size_t       len;
    bool         failedSanityCheck = false;
    
    if (pointNo == LAST_POINT)
        pointNo = dim->nPoints - 1;

    char const* textHeap = (char*)mdlDim_getOptionBlock (dimElement, ADBLK_TEXT, NULL);
    dimtxt   = dim->GetDimTextCP(pointNo);

    char const* src = textHeap + dimtxt->address;

    if ((UInt32)(src - (char*)dim) >= 2 * dim->GetAttributeOffset())
        {
        /*----------------------------------------------------------------------
           The address stored in the dimension heap points past attrOffset.

           If any of the dimtxt->flags.s & (0x1 << (i + 8)) are true, then
           this is an invalid state from which there is no way to recover the
           user text.  Just display the value instead.

           Need to find out how things got fouled up.
        ----------------------------------------------------------------------*/

        failedSanityCheck = true;
        }

    for (i=0; i<6; i++)
        {
        if ( ! failedSanityCheck && dimtxt->flags.s & (0x1 << (i + 8)))
            {
            len = VariCharConverter::ComputeNumBytes (src);
            strings[i].assign(AdimStringUtil::WStringFromVariChar (src, effectiveFont));
            src += len;
            }
        else
            strings[i].clear();
        }

    if (stringConfig)
        {
        if (dim->dimcmd == static_cast<byte>(DimensionType::LabelLine))
            {
            stringConfig->tolerance = true;
            stringConfig->limit = true;
            return (SUCCESS);
            }

        if (dim->flag.dual)
            stringConfig->dual = true;

        if (dim->flag.tolerance)
            {
            if (tolBlock = (DimTolrBlock*) mdlDim_getOptionBlock (dimElement, ADBLK_TOLERANCE, NULL))
                {
                stringConfig->tolerance = true;
                if (dim->flag.tolmode)
                    stringConfig->limit = true;
                }
            }
        }

    return  SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     11/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int      adim_insertTextCluster
(
EditElementHandleR dimElement,
int             pointNo,
WStringCP       strings,
DgnFontCR       effectiveFont
)
    {
    DimText    *dt;
    char       *start, *dst, *src, *textHeap;
    int        i, j;
    ptrdiff_t  newAddress;
    size_t     length = 0;
    char       vcStrings[6][MAX_DIMSTR];
    
    if (pointNo == LAST_POINT)
        pointNo = dimElement.GetElementP()->ToDimensionElm().nPoints - 1;
    else if (pointNo < 0 || pointNo >= (int)dimElement.GetElementP()->ToDimensionElm().nPoints)
        return  ERROR;

    for (i=0; i<6; i++)
        {
        *vcStrings[i] = 0;

        if (strings[i].empty())
            continue;

        AdimStringUtil::VariCharFromMSWChar (vcStrings[i], MAX_DIMSTR, strings[i].c_str(), effectiveFont);
        length += VariCharConverter::ComputeNumBytes (vcStrings[i]);
        }

    size_t dimSize = dimElement.GetElementP()->Size ();

    if (dimSize + length > MAX_V8_ELEMENT_SIZE || !length)
        return  ERROR;

    DgnV8ElementBlank tmpElement;
    dimElement.GetElementCP()->CopyTo (tmpElement);
    DimensionElm* dim = &tmpElement.ToDimensionElmR();
    
    textHeap = (char*)mdlDim_getEditOptionBlockFromElement (tmpElement, dimElement, ADBLK_TEXT, NULL);

    start = NULL;               /* Find the first (previous) point that     */
    for (i=pointNo; i>=0; i--)  /* already has a text cluster               */
        {
        dt  = dimElement.GetElementP()->ToDimensionElmR().GetDimTextP(i);
        if (dt->address >= 0)
            {
            start = textHeap + dt->address;
            for (j=0; j<6; j++)
                if (dt->flags.s & (1 << (j + 8)))
                    start += (VariCharConverter::ComputeNumBytes (start));
            break;
            }
        }

    if (!start)   /* No previous text cluster, this one is first in heap    */
        start = textHeap;

    
    BentleyApi::shiftElementData(dim, start, (int)length);

    newAddress = start - textHeap;
    for (i=0; i<(int)dim->nPoints; i++)     /* Shift addresses of later clusters */
        if (dimElement.GetElementP()->ToDimensionElm().GetDimTextCP(i)->address >= newAddress)
            dim->GetDimTextP(i)->address += static_cast<Int16>(length);

    dt  = dim->GetDimTextP(pointNo);
    dst = start;
    dt->address = static_cast<Int16>(newAddress);
    for (i=0; i<6; i++)                /* Copy strings to text heap. Set bit*/
        {                              /* in dimtext.flags for each string  */
        src = vcStrings[i];
        if (*src)
            {
            dt->flags.s |= (1 << (i + 8));
            
            size_t copySize = VariCharConverter::ComputeNumBytes (src);
            
            memcpy (dst, src, copySize);
            dst += copySize;
            }
        }
    dimElement.ReplaceElement (&tmpElement);
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/08
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     BentleyApi::adim_changeTextHeapEncoding
(
EditElementHandleR element,
UInt32          newFont,
UInt32          newBigFont,
UInt32          oldFont,
UInt32          oldBigFont
)
    {
    /*--------------------------------------------------------------------------
       Originally TR#109508 when changing fonts from non truetype to truetype or
       viceversa we need to convert the text to/from padded multibyte.

       Extended 06/2008 to account for whenever the two fonts use a different
       CodePage.
    --------------------------------------------------------------------------*/
    DgnFontCR      newEffectiveFont = DgnFontManager::GetFontForCodePage (newFont, newBigFont, *element.GetDgnProject ());
    DgnFontCR      oldEffectiveFont = DgnFontManager::GetFontForCodePage (oldFont, oldBigFont, *element.GetDgnProject ());

    if (newEffectiveFont.GetCodePage() == oldEffectiveFont.GetCodePage())
        return;

    DimensionHandler* hdlr = dynamic_cast<DimensionHandler*> (&element.GetHandler());
    if (NULL == hdlr)
        return;

    int numSegments = hdlr->GetNumSegments (element);

    for (int iSegment = 0; iSegment < numSegments; iSegment++)
        {
        int pointNo;

        if (BentleyApi::mdlDim_getTextPointNo (&pointNo, element, iSegment))
            continue;

        WString dimStrings[6];
        adim_extractTextCluster (dimStrings, NULL, element, pointNo, oldEffectiveFont);
        mdlDim_deleteTextCluster (element, pointNo);

        adim_insertTextCluster (element, pointNo, dimStrings, newEffectiveFont);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/08
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionHandler::InsertStrings (EditElementHandleR dimElement, DimStrings const& strings, int pointNo)
    {
    LegacyTextStyle   dimTextStyle;
    GetTextStyle (dimElement, &dimTextStyle);

    DgnFontCR      effectiveFont = DgnFontManager::GetFontForCodePage (dimTextStyle.fontNo, dimTextStyle.shxBigFont, *dimElement.GetDgnProject());

    return adim_insertTextCluster (dimElement, pointNo, strings.GetStringsCP(), effectiveFont);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name      mdlDim_extractStringsWide                                   |
|                                                                       |
| author    JVB                                         11/89           |
|                                                                       |
+----------------------------------------------------------------------*/
StatusInt   DimensionHandler::GetStrings (ElementHandleCR dimElement, DimStrings& strings, int pointNo, DimStringConfig* stringConfig)
    {
    LegacyTextStyle   textStyle;
    if (SUCCESS != GetTextStyle (dimElement, &textStyle))
        return ERROR;
    
    DgnProjectP file = dimElement.GetDgnProject();
    if (NULL == file)
        return ERROR;

    DgnFontCR      effectiveFont = DgnFontManager::GetFontForCodePage (textStyle.fontNo, textStyle.shxBigFont, *file);

    return adim_extractTextCluster (strings.GetStrings(), stringConfig, dimElement, pointNo, effectiveFont);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name      mdlDim_extractPointsD                                       |
|                                                                       |
| author    JVB                                         12/90           |
|                                                                       |
+----------------------------------------------------------------------*/
Public int  BentleyApi::mdlDim_extractPointsD
(
DPoint3d     *outPoints,
ElementHandleCR dimElement,
int          pointNo,
int          nPoints
)
    {
    for (int index = 0; index <nPoints; ++index, ++outPoints)
        *outPoints = dimElement.GetElementCP()->ToDimensionElm().GetPoint (pointNo + index);

    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          adim_getCurveArcInfo                                    |
|                                                                       |
| author        JVB                                     1/91            |
|                                                                       |
+----------------------------------------------------------------------*/
static int      adim_getCurveArcInfo
(
DimArcInfo*     pDimArc,        /* <= */
RotMatrixP      pRMatrix,       /* <= */
double*         uParam,         /* <= */
MSBsplineCurveP dimCurve,       /* => */
DPoint3dP       testPoint       /* => */
)
    {
    DPoint3d    curvePoint;
    double      minDist;

    if (SUCCESS != bsprcurv_minDistToCurve (&minDist, &curvePoint, uParam, testPoint, dimCurve, NULL, NULL))
        return ERROR;

    double      curvature;
    DVec3d      frenet[3];

    double torsion;
    if (SUCCESS != dimCurve->GetFrenetFrame (frenet, curvePoint, curvature, torsion, *uParam))
        return ERROR;

    if (curvature == 0.0)
        return (ERROR);

    double      radius = 1.0 / curvature;

    pDimArc->radius   = radius;
    pDimArc->start    = pDimArc->end = curvePoint;
    pDimArc->startAng = 0.0;
    pDimArc->sweepAng = msGeomConst_2pi;

    pRMatrix->InitFromColumnVectors(*frenet, frenet[1], frenet[2]);
    bsiDPoint3d_addScaledDVec3d (&pDimArc->center, &curvePoint, frenet+1, radius);
    *testPoint = curvePoint;

    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name      BentleyApi::adim_getArcInfo - get dimensioning information from an arc  |
|                             using a tag or arc pointer.               |
|                                                                       |
| author    JVB                                         1/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public int      BentleyApi::adim_getArcInfo
(
DimArcInfo*     pDimArc,        // <=
RotMatrixP      pRMatrix,       // <=
DisplayPathCP   pathP,          // => Element to extract info from (or NULL)
AssocPoint*     assocPointP,    // => Used to find element if pathP is NULL
ElementHandleCR dimElement
)
    {
    Transform           pathTrans;
    EditElementHandle   eeh;

    memset (pDimArc, 0, sizeof (*pDimArc));

    if (pathP)
        {
        if (SUCCESS != AssociativePoint::GetTargetGeometry (eeh, &pathTrans, pathP, dimElement.GetDgnModelP ()))
            return ERROR;
        }
    else
        {
        if (SUCCESS != AssociativePoint::GetTargetGeometry (eeh, &pathTrans, *assocPointP, 0, dimElement.GetDgnModelP ()))
            return ERROR;
        }

    CurveVectorPtr pathCurve = ICurvePathQuery::ElementToCurveVector (eeh);

    if (pathCurve.IsNull () || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc != pathCurve->HasSingleCurvePrimitive ())
        return ERROR;

    DEllipse3d   ellipse = *pathCurve->front ()->GetArcCP ();

    pathTrans.Multiply (ellipse);

    if (ellipse.IsCircular ())
        {
        ellipse.GetScaledRotMatrix (pDimArc->center, *pRMatrix, pDimArc->radius, pDimArc->radius, pDimArc->startAng, pDimArc->sweepAng);
        ellipse.EvaluateEndPoints (pDimArc->start, pDimArc->end);
        }
    else
        {
        MSBsplineCurve  dimCurve;

        if (SUCCESS != dimCurve.InitFromDEllipse3d (ellipse))
            return ERROR;

        double      uParam = 0.0;
        DPoint3d    testPoint = dimElement.GetElementCP ()->ToDimensionElm().GetPoint (2);
        StatusInt   status = adim_getCurveArcInfo (pDimArc, pRMatrix, &uParam, &dimCurve, &testPoint);

        dimCurve.ReleaseMem ();

        if (SUCCESS != status)
            return ERROR;

        pDimArc->notCircular = true;
        }

    if (!dimElement.GetDgnModelP ()->Is3d ())
        {
        pDimArc->start.z    = 0.0;
        pDimArc->end.z      = 0.0;
        pDimArc->center.z   = 0.0;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     BentleyApi::adim_getRMatrixFromDir
(
RotMatrix       *pOutRMatrix,
const DVec3d    *pDirection,
const RotMatrix *pDimRMatrix,
const RotMatrix *pViewRMatrix
)
    {
    DVec3d      viewX, viewY, viewZ;
    DVec3d      dimX, dimY, dimZ;

    dimX = *pDirection;
    pDimRMatrix->GetColumn(dimZ,  2);
    bsiDVec3d_crossProduct (&dimY, &dimZ, &dimX);

    pViewRMatrix->GetRow(viewX,  0);
    pViewRMatrix->GetRow(viewY,  1);
    pViewRMatrix->GetRow(viewZ,  2);

    bsiDVec3d_crossProduct (&dimZ, &dimX, &dimY);
    if (bsiDVec3d_dotProduct (&dimZ, &viewZ) < 0.0)
        {
        bsiDVec3d_scale (&dimZ, &dimZ, -1.0);
        bsiDVec3d_scale (&dimY, &dimY, -1.0);
        }

    pOutRMatrix->InitFromColumnVectors(dimX, dimY, dimZ);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          adim_isOffsetInDockTolerance                            |
|                                                                       |
| author        petri.niiranen                          08/00           |
|                                                                       |
+----------------------------------------------------------------------*/
static bool     adim_isOffsetInDockTolerance
(
AdimProcess const* const    ep,             /* => working process data           */
bool    const               dockDimLine,    /* => allow text to dock on dimension line */
DPoint2dCP                  textSizeP,      /* => text block size info */
double                      offsetY         /* => Dimension text location etc.        */
)
    {
    double  dockTolerance = textSizeP->y;

    if  (!dockDimLine)
        {
        /*---------------------------------------------------------------
        A DWG case. But we have not reverse engineered ACAD's docking
        tolerance, specially for multiline, yet. For now we make an
        approximate tolerance closed to what we found with ACAD.
        For single line, it is 2 * char height. For multiline, a reasonable
        correlation between text location and text height are not found,
        but it appears to be around number of texts lines*nominal char height.
        ----------------------------------------------------------------*/
        int numLines = 1;

        if (!ep->m_textBlock->IsEmpty () && (numLines = ep->m_textBlock->GetLineCount (ep->m_textBlock->Begin (), ep->m_textBlock->End ()) > 1))
            dockTolerance = ep->GetDimElementCP()->geom.textMargin + ep->GetDimElementCP()->text.height * numLines;
        else
            dockTolerance = 2.0 * ep->GetDimElementCP()->text.height;
        }

    return  dockTolerance > fabs (offsetY);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt    BentleyApi::adim_createUnitBlock
(
DimUnitBlock*       pUnitBlock,        /* <= */
bool                isPrimary,
double const*       pUorPerStorage,    /* => */
UnitDefinitionCP    masterUnit,
UnitDefinitionCP    subUnit
)
    {
    /*-------------------------------------------------------------------
      This function does NOT use the unitInfo labels, which are not
      stored in the unit block
    -------------------------------------------------------------------*/
    if (NULL == pUnitBlock)
        return ERROR;

    memset (pUnitBlock, 0, sizeof (DimUnitBlock));
    pUnitBlock->nWords = sizeof (DimUnitBlock) / 2;
    pUnitBlock->type   = isPrimary ? ADBLK_PRIMARY : ADBLK_SECONDARY;

    if (NULL != pUorPerStorage)
        {
        pUnitBlock->uorPerMast          = *pUorPerStorage;
        }

    if (NULL != masterUnit)
        {
        UnitInfo    unitInfo = masterUnit->ToUnitInfo();

        pUnitBlock->masterFlags         = unitInfo.flags;
        pUnitBlock->masterNumerator     = unitInfo.numerator;
        pUnitBlock->masterDenominator   = unitInfo.denominator;
        }

    if (NULL != subUnit)
        {
        UnitInfo    unitInfo = subUnit->ToUnitInfo();

        pUnitBlock->subFlags            = unitInfo.flags;
        pUnitBlock->subNumerator        = unitInfo.numerator;
        pUnitBlock->subDenominator      = unitInfo.denominator;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  05/01
+---------------+---------------+---------------+---------------+---------------+------*/
static UnitDefinition  unitDefFromUnitData
(
UnitFlags       flags,
double          numerator,
double          denominator
)
    {
    UnitBase    base   = UnitDefinition::BaseFromInt   (flags.base);
    UnitSystem  system = UnitDefinition::SystemFromInt (flags.system);

    return UnitDefinition (base, system, numerator, denominator, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  05/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public UnitDefinition  BentleyApi::adim_getUnitDefFromDimUnit
(
DimUnits const&         dimUnit
)
    {
    return unitDefFromUnitData (dimUnit.flags, dimUnit.numerator, dimUnit.denominator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
Public DimUnits BentleyApi::adim_getDimUnitFromUnitDef
(
UnitDefinitionCR unitDef
)
    {
    DimUnits    dimUnit;

    memset (&dimUnit, 0, sizeof(dimUnit));

    dimUnit.flags.base   = static_cast<UInt32>(unitDef.GetBase());
    dimUnit.flags.system = static_cast<UInt32>(unitDef.GetSystem());
    dimUnit.numerator    = unitDef.GetNumerator();
    dimUnit.denominator  = unitDef.GetDenominator();

    return dimUnit;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt    BentleyApi::adim_extractUnitBlock
(
double              *pUorPerStorage,    /* <= */
UnitDefinition      *pMasterUnit,       /* <= */
UnitDefinition      *pSubUnit,          /* <= */
DimUnitBlock        *pUnitBlock         /* => */
)
    {
    /*-------------------------------------------------------------------
      This function does NOT extract the unit labels, which are not
      stored in the unit block
    -------------------------------------------------------------------*/
    if (NULL == pUnitBlock)
        return ERROR;

    if (pUorPerStorage)
        *pUorPerStorage = pUnitBlock->uorPerMast;

    if (pMasterUnit)
        *pMasterUnit = unitDefFromUnitData (pUnitBlock->masterFlags, pUnitBlock->masterNumerator, pUnitBlock->masterDenominator);

    if (pSubUnit)
        *pSubUnit = unitDefFromUnitData (pUnitBlock->subFlags, pUnitBlock->subNumerator, pUnitBlock->subDenominator);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Check input string for possible fraction. This check searches for digit-'/'-digit
* pattern.
* @param        pwStringIn      => string to check for fraction
* @param        ppwSlashPos     <= slash position or NULL
* @bsimethod                                                    petri.niiranen  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     BentleyApi::adim_hasFraction
(
WCharP        pwStringIn,
WCharP*       ppwSlashPos
)
    {
    WCharCP   pwSlashPos = NULL;
    WCharCP   pwPrevChar = NULL;
    WCharCP   pwNextChar = NULL;

    // search for digit-/-digit pattern
    if (NULL != (pwSlashPos = wcschr (pwStringIn, '/')))
        {
        pwPrevChar = pwSlashPos - 1;
        pwNextChar = pwSlashPos + 1;

        if (pwPrevChar >= pwStringIn && pwNextChar < pwStringIn + wcslen (pwStringIn))
            {
            if (iswdigit (*pwPrevChar) && iswdigit (*pwNextChar))
                {
                if (ppwSlashPos)
                    *ppwSlashPos = (WCharP) pwSlashPos;

                return  true;
                }
            }
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* Parse fraction components
*
* @param        pwIntegerOut     <= prevalue and integer text
* @param        pwNumeratorOut   <= numerator text
* @param        pwDenominatorOut <= denominator text
* @param        pwReminingOut    <= text following denominator
* @param        pwStringIn       => string to parse
* @return       StatusInt, SUCCESS if fraction can be parsed
* @bsimethod                                                    petri.niiranen  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt        BentleyApi::adim_parseFraction
(
WCharP        pwIntegerOut,
WCharP        pwNumeratorOut,
WCharP        pwDenominatorOut,
WCharP        pwReminingOut,
WCharCP       pwStringIn
)
    {
    WCharP        pwBuf       = NULL;
    WCharP        pwSearchPos = NULL;
    WCharP        pwSlashPos  = NULL;
    size_t          dataSize    = wcslen (pwStringIn) * sizeof (WChar);

    if (pwIntegerOut)
        *pwIntegerOut = '\0';

    if (pwNumeratorOut)
        *pwNumeratorOut = '\0';

    if (pwDenominatorOut)
        *pwDenominatorOut = '\0';

    if (pwReminingOut)
        *pwReminingOut = '\0';

    pwBuf = (WCharP) _alloca (dataSize);
    memset (pwBuf, 0, dataSize);
    wcscpy (pwBuf, pwStringIn);

    // search for digit-/-digit pattern
    if (adim_hasFraction (pwBuf, &pwSearchPos))
        {
        pwSlashPos   = pwSearchPos;
        *pwSearchPos = '\0';

        if (pwIntegerOut)
            {
            wcscpy (pwIntegerOut, pwBuf);

            // terminate string to the last space (first from the end)
            if (NULL != (pwSearchPos = wcsrchr (pwIntegerOut, ' ')))
                *pwSearchPos = '\0';
            }

        if (pwNumeratorOut)
            {
            if (NULL != (pwSearchPos = wcsrchr (pwBuf, ' ')))
                wcscpy (pwNumeratorOut, pwSearchPos + 1);
            }

        if (pwDenominatorOut)
            {
            if (iswdigit (*(pwSlashPos + 1)))
                {
                wcscpy (pwDenominatorOut, pwSlashPos + 1);
                if (NULL != (pwSearchPos = wcschr (pwDenominatorOut, ' ')))
                    *pwSearchPos = '\0';
                }
            }

        if (pwReminingOut)
            {
            if (NULL != (pwSearchPos = wcschr (pwSlashPos + 1, ' ')))
                wcscpy (pwReminingOut, pwSearchPos);
            }

        return  SUCCESS;
        }

    return  ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_getAlternateSeparator
(
WCharP    pSeparatorChar,
ElementHandleCR    dimElement,
bool        bFirstChar
)
    {
    WChar           separatorChar = L' '; // 0x20, space
    StatusInt       status        = ERROR;

    if (!dimElement.IsValid())
        return  ERROR;

    DimensionElm const* pDim = &dimElement.GetElementCP()->ToDimensionElm();
    switch (pDim->frmt.dualFormat)
        {
        case 1:
            separatorChar = bFirstChar ? L'(' : L')';
            status        = SUCCESS;
            break;

        case 2:
            separatorChar = bFirstChar ? L'[' : L']';
            status        = SUCCESS;
            break;

        case 3:
            {
            DimSymBlock *pSymBlock = NULL;

            if (NULL != (pSymBlock = (DimSymBlock*) mdlDim_getOptionBlock (dimElement, bFirstChar ? ADBLK_ALTPRESEPARATOR : ADBLK_ALTPOSTSEPARATOR, NULL)))
                {
                separatorChar = pSymBlock->symChar;
                status        = SUCCESS;
                }

            break;
            }

        case 0:
        default:
//          status = SUCCESS;
            break;
        }

    if (pSeparatorChar && SUCCESS == status)
        *pSeparatorChar = separatorChar;

    // return SUCCESS if some meaning full information was found.
    // ERROR means that separator char should not be applied

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     mdlDim_setCustomAlternateSeparator
(
EditElementHandleR dimElement,
WChar     separatorChar,
bool        bFirstChar
)
    {
    DimSymBlock     altSepBlock;
    memset (&altSepBlock, 0, sizeof (altSepBlock));
    altSepBlock.nWords  = sizeof (altSepBlock) / 2;
    altSepBlock.type    = bFirstChar ? ADBLK_ALTPRESEPARATOR : ADBLK_ALTPOSTSEPARATOR;
    altSepBlock.symChar = separatorChar;
    altSepBlock.symFont = 0xffffffff;

    return  mdlDim_insertOptionBlock (dimElement, (DimOptionBlockHeader *) &altSepBlock, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     mdlDim_setAlternateSeparator
(
DgnElement   *pElementIn,
int         type
)
    {
    DimensionElm    *pDim = (DimensionElm *) pElementIn;

    if (NULL == pElementIn)
        return  ERROR;

    pDim->frmt.dualFormat = type & 0x3;

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public WString* BentleyApi::adim_getSimpleStringPtrByType
(
DimStrings*     pDimStrings,
int             iPartType,
int             iSubType
)
    {
    WString             *pwString = NULL;

    if ( ! mdlDim_partTypeIsAnyText (iPartType))
        return NULL;

    // determine which string we got
    WString* partStrings = (ADTYPE_TEXT_LOWER == iPartType) ? pDimStrings->GetSecondaryStrings () : &pDimStrings->GetPrimaryStrings()[0];
    switch  (iSubType)
        {
        case ADSUB_NONE:
            pwString = &partStrings[0];
            break;

        case ADSUB_TOL_UPPER:
            pwString = &partStrings[1];
            break;

        case ADSUB_TOL_SINGLE:
        case ADSUB_TOL_LOWER:
            pwString = &partStrings[2];
            break;

        case ADSUB_LIM_UPPER:
        case ADSUB_LIM_SINGLE:
            pwString = &partStrings[0];
            break;

        case ADSUB_LIM_LOWER:
            pwString = &partStrings[1];;
            break;
        }

    return  pwString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public int      BentleyApi::mdlDim_getTextPointNo
(
int*                pointNo,
ElementHandleCR     dimElement,
int                 segNo
)
    {
    return DimensionHandler::GetInstance().GetTextPointNo (*pointNo, dimElement, segNo);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          adim_unpackFromFourDoubles                              |
|                                                                       |
| author        BarryBentley                            09/01           |
|                                                                       |
+----------------------------------------------------------------------*/
Public void    BentleyApi::adim_unpackFromFourDoubles
(
RotMatrix       *rMatrix,
const double    *pQuat,
bool            is3D
)
    {
    if (is3D)
        rMatrix->InitTransposedFromQuaternionWXYZ ( pQuat);
    else
        rMatrix->InitFromRowValuesXY ( pQuat);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          adim_packToFourDoubles                                  |
|                                                                       |
| author        BarryBentley                            09/01           |
|                                                                       |
+----------------------------------------------------------------------*/
Public void     BentleyApi::adim_packToFourDoubles
(
double             *pQuat,
const RotMatrix    *rmP,
bool                is3d
)
    {
    if (is3d)
        rmP->GetQuaternion(pQuat, true);
    else
        rmP->GetRowValuesXY(pQuat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt    getDimRotMatrix
(
RotMatrix           *pRMatrix,
const DimensionElm  *pDim,
const double        *pSlantAngle
)
    {
    adim_unpackFromFourDoubles (pRMatrix, pDim->quat, pDim->Is3d());

    /* Only 3d dims with arbitrary alignment use the slant angle override */
    if (pSlantAngle)
        {
        double      cc, ss;
        DVec3d      xVec, yVec, zVec;

        pRMatrix->GetColumn(xVec,  0);
        pRMatrix->GetColumn(yVec,  1);
        pRMatrix->GetColumn(zVec,  2);

        cc = cos (*pSlantAngle);
        ss = sin (*pSlantAngle);
        bsiDPoint3d_add2ScaledDPoint3d (&yVec, NULL, &xVec, cc, &yVec, ss);

        pRMatrix->InitFromColumnVectors(xVec, yVec, zVec);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            AdimProcess::InitRotMatrix()
    {

    double  slantAngle, *pSlantAngle = NULL;
    DimensionElm const* pDim = GetDimElementCP();
    DimViewBlock const* pViewBlock = NULL;
    if ((pViewBlock = (DimViewBlock const*)mdlDim_getOptionBlock (GetElemHandleCR(), ADBLK_VIEWROT, NULL)) != NULL)
        adim_unpackFromFourDoubles (&vuMatrix, pViewBlock->viewRot, pDim->Is3d());
    else
        vuMatrix.InitIdentity();
    /* Only 3d dims with arbitrary alignment use the slant angle override */
    if (3 == pDim->flag.alignment && pDim->Is3d() &&
        mdlDim_overridesGetOverallSlantAngle (&slantAngle, pOverrides, 0.0))
        {
        pSlantAngle = &slantAngle;
        }

    getDimRotMatrix (&rMatrix, pDim, pSlantAngle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt    BentleyApi::mdlDim_getDimRotMatrix
(
RotMatrix       *pRMatrix,
ElementHandleCR    dimElement
)
    {
    DimensionHandler* hdlr = dynamic_cast<DimensionHandler*> (&dimElement.GetHandler());
    if (NULL == hdlr || NULL == pRMatrix)
        return ERROR;

    return hdlr->GetRotationMatrix (dimElement, *pRMatrix);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionHandler::_GetRotationMatrix (ElementHandleCR dimElement, RotMatrixR rmatrix) const
    {
    double  slantAngle, *pSlantAngle = NULL;
    DimensionElm const* pDim = &dimElement.GetElementCP()->ToDimensionElm();
    /* Only 3d dims with arbitrary alignment use the slant angle override */
    if (3 == pDim->flag.alignment && pDim->Is3d() &&
        mdlDim_overallGetSlantAngle (&slantAngle, dimElement))  // NEEDWORK: get direct from ep
        {
        pSlantAngle = &slantAngle;
        }
    
    return (BentleyStatus) getDimRotMatrix (&rmatrix, pDim, pSlantAngle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DimensionHandler::_GetOrientation (ElementHandleCR eh, RotMatrixR rot)
    {
    if (SUCCESS != GetRotationMatrix (eh, rot))
        rot.initIdentity();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt    BentleyApi::mdlDim_getDimRawMatrix
(
RotMatrixP          pRMatrix,
ElementHandleCR     dimElement
)
    {
    DimensionElm const* pDim = &dimElement.GetElementCP()->ToDimensionElm();
    return  getDimRotMatrix (pRMatrix, pDim, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* Warning: can change the size of the element
* @bsimethod                                                    JoshSchifter    10/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt    BentleyApi::mdlDim_setDimRotMatrix
(
EditElementHandleR dimElement,
const RotMatrix     *pRMatrix
)
    {
    IDimensionEdit* dimEdit = dynamic_cast<IDimensionEdit*> (&dimElement.GetHandler());
    if (NULL == dimEdit || NULL == pRMatrix)
        return ERROR;

    return dimEdit->SetRotationMatrix (dimElement, *pRMatrix);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          adim_createRotMatrix                                    |
|                                                                       |
| author        RBB                                     11/86           |
|                                                                       |
+----------------------------------------------------------------------*/
Public void         BentleyApi::mdlDim_createRotationMatrix
(
RotMatrixR          matrixOut,
DPoint3dCR          point1,
DPoint3dCR          point2,
DPoint3dCR          point3,
int                 alignment,
RotMatrixCR         viewMatrix,
bool                dimIs3d
)
    {
    DVec3d      xvec, yvec, view_x, view_y, view_z, trans_x, trans_y, trans_z;
    double      xdot, ydot;

    switch (alignment)
        {
        case View:                      /* view axis */
            {
            viewMatrix.GetRow (view_x, 0);
            viewMatrix.GetRow (view_y, 1);
            viewMatrix.GetRow (view_z, 2);

            yvec = DVec3d::FromStartEndNormalize (point2, point1);
            xvec = DVec3d::FromStartEndNormalize (point3, point1);

            xdot = yvec.DotProduct (view_x);
            ydot = yvec.DotProduct (view_y);

            if (fabs (xdot) > fabs (ydot))
                {
                trans_y = view_x;
                if (xdot < 0)
                    trans_y.Negate ();
                }
            else
                {
                trans_y = view_y;
                if (ydot < 0)
                    trans_y.Negate (view_y);
                }

            trans_x = DVec3d::FromCrossProduct (trans_y, view_z);

            // Note: At this point, trans_x may turn out to be -ve. The code then translates
            // the -ve sign from the x-vec to the z-vec. This is not a good idea for 2d models
            // since the z-vec gets thrown away and the dimension is no longer right-handed.
            // Ideally we should not be doing this. However, Josh is not convinced about
            // removing this for a couple of reasons. One, there are already existing 2d
            // dimensions that are non-right handed. Two, we don't know why this logic was
            // originally implemented. So I am going to leave this as it is for now.
            if (xvec.DotProduct(trans_x) < 0.0)
                {
                trans_x.Negate ();
                view_z.Negate ();
                }

            trans_x.Normalize ();
            trans_y = DVec3d::FromCrossProduct (view_z, trans_x);
            trans_y.Normalize ();

            matrixOut = RotMatrix::FromColumnVectors (trans_x, trans_y, view_z);
            break;
            }

        case Drawing:                   /* database axis */
            {
            yvec = DVec3d::FromStartEndNormalize (point2, point1);
            xvec = DVec3d::FromStartEndNormalize (point3, point2);

            if (fabs(yvec.x) > fabs (yvec.y) && fabs (yvec.x) > fabs (yvec.z))
                {
                trans_y.x = (yvec.x > 0.0 ? 1.0: -1.0);
                trans_y.y = 0.0;
                trans_y.z = 0.0;

                if (fabs (xvec.y) > fabs (xvec.z))
                    {
                    trans_x.y = (xvec.y > 0.0 ? 1.0: -1.0);
                    trans_x.z = 0.0;
                    }
                else
                    {
                    trans_x.y = 0.0;
                    trans_x.z = (xvec.z > 0.0 ? 1.0: -1.0);
                    }
                trans_x.x = 0.0;
                }
            else if (fabs(yvec.y) > fabs (yvec.x) && fabs(yvec.y) > fabs(yvec.z))
                {
                trans_y.x = 0.0;
                trans_y.y = (yvec.y > 0.0 ? 1.0: -1.0);
                trans_y.z = 0.0;

                if (fabs (xvec.x) > fabs(xvec.z))
                    {
                    trans_x.x = (xvec.x > 0.0 ? 1.0: -1.0);
                    trans_x.z = 0.0;
                    }
                else
                    {
                    trans_x.x = 0.0;
                    trans_x.z = (xvec.z > 0.0 ? 1.0: -1.0);
                    }

                trans_x.y = 0.0;
                }
            else
                {
                trans_y.x = 0.0;
                trans_y.y = 0.0;
                trans_y.z = (yvec.z > 0.0 ? 1.0: -1.0);

                if (fabs (xvec.x) > fabs (xvec.y))
                    {
                    trans_x.x = (xvec.x > 0.0 ? 1.0: -1.0);
                    trans_x.y = 0.0;
                    }
                else
                    {
                    trans_x.x = 0.0;
                    trans_x.y = (xvec.y > 0.0 ? 1.0: -1.0);
                    }
                trans_x.z = 0.0;
                }

            trans_z = DVec3d::FromCrossProduct (trans_x, trans_y);
            trans_z.Normalize ();
            matrixOut = RotMatrix::FromColumnVectors (trans_x, trans_y, trans_z);
            break;
            }

        case TrueOrientation:                 /* parallel axis */
            {
            if (dimIs3d)
                {
                // Maybe we can replace this w/ RotMatrix::RotationFromOriginXY?
                matrixOut.InitRotationFromOriginXY (point1, point3, point2);
                }
            else
                {
                trans_x = DVec3d::FromStartEndNormalize (point3, point1);
                trans_y = DVec3d::FromStartEndNormalize (point2, point1);
                trans_z = DVec3d::FromCrossProduct (trans_x, trans_y);
                trans_z.Normalize ();
                matrixOut = RotMatrix::FromColumnVectors (trans_x, trans_y, trans_z);
                }
            break;
            }

        case ArbitaryOrientation:               /* Arbitrary axis */
            {
            trans_x = DVec3d::FromStartEndNormalize (point3, point1);
            trans_y = DVec3d::FromStartEndNormalize (point2, point1);
            trans_z = DVec3d::FromCrossProduct (trans_x, trans_y);
            trans_z.Normalize ();
            matrixOut = RotMatrix::FromColumnVectors (trans_x, trans_y, trans_z);
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionHandler::_SetRotationMatrix (EditElementHandleR dimElement, RotMatrixCR rmatrixIn)
    {
    RotMatrix   rMatrix = rmatrixIn;

    if (3 == dimElement.GetElementP()->ToDimensionElm().flag.alignment && dimElement.GetElementP()->ToDimensionElm().Is3d() &&
        !bsiRotMatrix_isOrthogonal (&rmatrixIn))
        {
        double      slantAngle = 0.0;
        DPoint3d    org;
        DVec3d      xVec, yVec, zVec;

        bsiDPoint3d_zero (&org);
        rmatrixIn.GetColumn(xVec,  0);
        rmatrixIn.GetColumn(yVec,  1);
        rmatrixIn.GetColumn(zVec,  2);

        slantAngle = bsiDPoint3d_angleBetweenVectors (&xVec, &yVec);

        /* store the slant angle since it won't go into quat */
        mdlDim_overallSetSlantAngle (dimElement, &slantAngle);

        /* Only ortho normal matrices can be stored by quat, but */
        /* We don't want to change the input matrix */
        rMatrix.SquareAndNormalizeColumns (rMatrix, 0, 1);
        }
    else
        {
        mdlDim_overallSetSlantAngle (dimElement, NULL);
        }

    adim_packToFourDoubles (dimElement.GetElementP()->ToDimensionElmR().quat, &rMatrix, dimElement.GetElementP()->ToDimensionElm().Is3d());
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  02/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public UInt16   BentleyApi::adim_getDimTextJustification
(
AdimProcess const* const pAdimProcess
)
    {
    DimMLText           *pText = NULL;
    DimFormattedText    *pFmt  = NULL;
    UInt16              iJust  = static_cast<UInt16>(TextElementJustification::LeftMiddle);

    if (SUCCESS == mdlDim_overridesGetDimMLText (&pText, pAdimProcess->pOverrides,
                                                 ADIM_GETSEG (pAdimProcess->partName)))
        {
        if (NULL != (pFmt = mdlDimText_getNodeFormatter (pText)) ||
            NULL != (pFmt = mdlDimText_getFormatter (pText, 0)))
            {
            iJust = pFmt->GetJustification();
            }
        }
    else
        {
        // possible override
        mdlDim_overridesGetSegmentTextJustification (&iJust, pAdimProcess->pOverrides,
                                                     ADIM_GETSEG (pAdimProcess->partName),
                                                     iJust);
        }

    return  iJust;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    sunand.sandurkar 08/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public TextElementJustification BentleyApi::adim_getDimTextJustificationHorizontal
(
AdimProcess const* const pAdimProcess
)
    {
    UInt16          iJust = static_cast<UInt16>(TextElementJustification::LeftMiddle);

    iJust = adim_getDimTextJustification (pAdimProcess) / 3;

    // Return the basic justification of the text.
    // 0, 1       - Left   Justification
    // 3, 4       - Right  Justification
    // 2, default - Center Justification

    if (iJust == 0 || iJust == 1)
        return TextElementJustification::LeftMiddle;

    else if (iJust == 3 || iJust == 4)
        return TextElementJustification::RightMiddle;

    else
        return TextElementJustification::CenterMiddle;
    }

/*---------------------------------------------------------------------------------**//**
* @return   offset at left edge of text
* @bsimethod                                                    petri.niiranen  02/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public double   BentleyApi::adim_computeLeftEdgeOffset
(
AdimProcess const* const pAdimProcess,
double      const        dOffsetIn,
double      const        dWidth
)
    {
    /*-----------------------------------------------------------------------------------
        Note!
        * bTextReversed flag is currently ignored to make behavior consistent
          with legacy versions. Upto MSJ reversed dimension text was placed from
          its right edge although the text was left aligned.
    -----------------------------------------------------------------------------------*/
    switch  (adim_getDimTextJustificationHorizontal (pAdimProcess))
        {
        case TextElementJustification::RightMiddle:
            return  dOffsetIn - dWidth;
            break;

        case TextElementJustification::CenterMiddle:
            return  dOffsetIn - (dWidth / 2.0);
            break;

        default:    // Start Justification
            return  dOffsetIn;
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  03/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public double   BentleyApi::adim_getIntersectLength
(
DPoint3d const* const pCheck,
DPoint2dCP            pTextSize,
double   const        dMargin
)
    {
    double  intersect_len;

    // NEEDSWORK : This function does not give the right value in the case of
    //             angular dimensions for the following reasons.
    // a. pCheck is wrong. It should ideally be at the center of the overlap
    //    length, but that's the value we are computing here.
    // b. The logic is based on the dimension line being linear. For the
    //    angular case, it only gives an approximate value.
    // c. The text margins should not be included here. Get the tight overlap
    //    length from this function and handle the margins outside.
    if (fabs (pCheck->y * pTextSize->x) > fabs (pCheck->x * pTextSize->y))
        intersect_len = fabs ((pTextSize->y + dMargin) / SAFE_DOUBLE (pCheck->y));
    else
        intersect_len = fabs ((pTextSize->x + dMargin) / SAFE_DOUBLE (pCheck->x));

    return  intersect_len;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    isAutoBallNChain
(
AdimProcess const* const pAdimProcess
)
    {
    DimStyleProp_BallAndChain_Mode  bncMode = DIMSTYLE_VALUE_BallAndChain_Mode_None;

    mdlDim_getBallNChainMode (&bncMode, pAdimProcess->GetElemHandleCR());

    return  DIMSTYLE_VALUE_BallAndChain_Mode_Auto == bncMode && pAdimProcess->flags.allDontFit;
    }

/*---------------------------------------------------------------------------------**//**
* Check whether dimension with leader aka ball-n-chain is in use
* @bsimethod                                                    petri.niiranen  03/02
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     adim_checkForLeaderInternal
(
UInt16                   *chainType,       /* <=  return the ball and chain type, or NULL */
AdimProcess const* const ep,               /*  => working process data           */
const double             offsetY,          /*  => text offset data               */
DPoint2dCP               textSize          /*  => text size data                 */
)
    {
    DimOffsetBlock *offsetBlock = NULL;

    /* If there isn't any y-offset, then there's no leader */
    if (LegacyMath::DEqual (offsetY, 0.0))
        return false;

    /* If the extension offset block is not available, then there's no leader */
    if (NULL == (offsetBlock = (DimOffsetBlock*) mdlDim_getOptionBlock (ep->GetElemHandleCR(), ADBLK_EXTOFFSET, NULL)))
        return false;

    /* Return the chain type if requested */
    if (chainType)
        *chainType = offsetBlock->flags.chainType;

    /* If the text is supposed to be docked on the dim line when the y-offset is within
     * tolerance AND the y-offset is infact within tolerance then there's no leader */
    if (adim_isOffsetInDockTolerance(ep, !offsetBlock->flags.noDockOnDimLine, textSize, offsetY) && !isAutoBallNChain(ep))
        {
        if (!offsetBlock->flags.noDockOnDimLine)
            return false;

        /*-------------------------------------------------------------------------------
        if text is within the docking range, but the dimension does not want to dock as
        in DWG case, force the chain type to none so we do not generate the chain.
        -------------------------------------------------------------------------------*/
        if (chainType)
            *chainType = DIMSTYLE_VALUE_BallAndChain_ChainType_None;
        }

    /* Fall thru case: true */
    if (NULL != ep->pLeaderedMask)
        (ep->pLeaderedMask)->SetBit ( ADIM_GETSEG (ep->partName),  true);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Check whether dimension with leader aka ball-n-chain is in use
* @bsimethod                                                    petri.niiranen  03/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     BentleyApi::adim_checkForLeader
(
UInt16                   *chainType,       /* <=  return the ball and chain type, or NULL */
AdimProcess const* const ep,               /*  => working process data           */
const double             offsetY,          /*  => text offset data               */
DPoint2dCP               textSize          /*  => text size data                 */
)
    {
    bool    hasLeader = adim_checkForLeaderInternal(chainType, ep, offsetY, textSize);
    int     iSegment = ADIM_GETSEG (ep->partName);

    if (hasLeader)
        {
        if (ep->pDerivedData && ep->pDerivedData->pIsTextDocked)
            ep->pDerivedData->pIsTextDocked[iSegment] = false;
        }

    if (NULL != chainType && NULL != ep->pDerivedData && NULL != ep->pDerivedData->pChainType)
        ep->pDerivedData->pChainType[iSegment] = *chainType;

    return  hasLeader;
    }

/*---------------------------------------------------------------------------------**//**
* Apply scale factor to dimension to compensate for element being cloned from a
* scaled reference file.
* @bsimethod                                                    RayBentley      04/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt        BentleyApi::adimUtil_scaleDimValue
(
EditElementHandleR  dimElement,         /* <=> */
double              scale,              /*  => */
bool                modifyAllowed       /*  => ok to change the element's size */
)
    {
    if (fabs (scale - 1.0) < mgds_fc_epsilon)
        return SUCCESS;

    DimensionHandler&   hdlr            = DimensionHandler::GetInstance();
    DimStylePropMaskPtr shieldFlags     = hdlr.GetOverrideFlags (dimElement);

    // Set the scale and the override
    dimElement.GetElementP()->ToDimensionElmR().SetScale(dimElement.GetElementP()->ToDimensionElm().GetScale()/ scale);
    shieldFlags->SetPropertyBit (DIMSTYLE_PROP_General_DimensionScale_DOUBLE, true);

    if (modifyAllowed)
        mdlDim_setOverridesDirect (dimElement, shieldFlags.get(), false);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public StackedFractionType  BentleyApi::adim_textFracTypeFromDim (int dimFracType)
    {
    switch (dimFracType)
        {
        case DIMSTYLE_VALUE_Text_StackedFractionType_Horizontal:
            {
            return StackedFractionType::HorizontalBar;
            }
        case DIMSTYLE_VALUE_Text_StackedFractionType_Diagonal:
            {
            return StackedFractionType::DiagonalBar;
            }
        case DIMSTYLE_VALUE_Text_StackedFractionType_FromFont:
        default:
            {
            return StackedFractionType::None;
            }
        }

    // Unreachable code
    // return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public int      adim_dimFracTypeFromText (StackedFractionType textFracType)
    {
    switch (textFracType)
        {
        case StackedFractionType::NoBar:
        case StackedFractionType::HorizontalBar:
            {
            return DIMSTYLE_VALUE_Text_StackedFractionType_Horizontal;
            }
        case StackedFractionType::DiagonalBar:
            {
            return DIMSTYLE_VALUE_Text_StackedFractionType_Diagonal;
            }
        case StackedFractionType::None:
        default:
            {
            return DIMSTYLE_VALUE_Text_StackedFractionType_FromFont;
            }
        }

    // Unreachable code
    // return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     BentleyApi::adim_setCurrentSegmentTextIsOutside
(
AdimProcess    *pAdimProcess,
int             dimJust
)
    {
    int segment = pAdimProcess->ep.segment;

    // 'textOutside' means that the text was 'pushed'.  Manual offset text is not considered pushed.
    if (DIMTEXT_OFFSET == dimJust)
        return;

    // Radial dims do not properly initialize segment
    if (0 >= segment)
        segment = 1;

    // flags used for current segment
    pAdimProcess->flags.pushTextOutside = true;

    // derived data for recording all segments
    if (pAdimProcess->pDerivedData && NULL != pAdimProcess->pDerivedData->pIsTextOutside)
        pAdimProcess->pDerivedData->pIsTextOutside[segment-1] = true;
    }

/*---------------------------------------------------------------------------------**//**
* This piece of code is moved from adim_generateSizeDim
*
* @bsimethod                                                    Don.Fu          02/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public void         BentleyApi::adim_getEffectiveTerminators
(
int                 *pLeftTerm,
int                 *pRightTerm,
AdimProcess const   *pAdimProcess
)
    {
    if (NULL != pLeftTerm)
        *pLeftTerm = pAdimProcess->GetDimElementCP()->tmpl.first_term ? pAdimProcess->GetDimElementCP()->tmpl.first_term : pAdimProcess->GetDimElementCP()->tmpl.left_term;

    if (NULL != pRightTerm)
        {
        if (!pAdimProcess->GetDimElementCP()->tmpl.stacked)
            *pRightTerm = !pAdimProcess->flags.lastSeg && pAdimProcess->GetDimElementCP()->tmpl.bowtie_symbol ? 0 : pAdimProcess->GetDimElementCP()->tmpl.right_term;
        else
            *pRightTerm = pAdimProcess->GetDimElementCP()->tmpl.right_term;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    isLineThruTermForFit
(
AdimProcess const   *pAdimProcess,
DimTermBlock const  *pTermBlock,
int                 termIndex
)
    {
    /*-----------------------------------------------------------------------------------
    For fit purpose, apply no-line-thru flag only to cell terminators:

    That is, default terminators are all treated as no-line-thru; cell terminators may
    be line-thru or no-line-thru depending on the flag set.
    -----------------------------------------------------------------------------------*/
    if (!adim_checkNoLineFlag(pAdimProcess, termIndex))
        {
        switch  (termIndex)
            {
            case 1:
                return  pTermBlock->flags.arrow >= 2;
            case 2:
                return  pTermBlock->flags.stroke >= 2;
            case 3:
                return  pTermBlock->flags.origin >= 2;
            case 4:
                return  pTermBlock->flags.dot >= 2;
            }
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     BentleyApi::adim_isDimlineThruEitherTerm
(
AdimProcess const   *pAdimProcess
)
    {
    DimTermBlock    *pTermBlock = (DimTermBlock*) mdlDim_getOptionBlock (pAdimProcess->GetElemHandleCR(), ADBLK_TERMINATOR, NULL);

    if (NULL != pTermBlock)
        {
        int     leftTerm = 0, rightTerm = 0;

        adim_getEffectiveTerminators (&leftTerm, &rightTerm, pAdimProcess);

        return  isLineThruTermForFit(pAdimProcess, pTermBlock, leftTerm) || isLineThruTermForFit(pAdimProcess, pTermBlock, leftTerm);
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public void         BentleyApi::adim_getEffectiveMinLeaders
(
double              *pInside,
double              *pOutside,
AdimProcess const   *pAdimProcess
)
    {
    double          insideLength = pAdimProcess->GetDimElementCP()->geom.margin;
    double          outsideLength = insideLength;

    if (pAdimProcess->flags.ignoreMinLeader)
        {
        bool        isThruTerm = adim_isDimlineThruEitherTerm (pAdimProcess);

        /*-------------------------------------------------------------------------------
        Total inside min leader is made up by term width plus an extra dim line which
        is the same as text margin.
        For line-thru-term case, the inside min leader is made up only by the same extra
        dim line which again equals to text margin.
        Keep in mind that neither case by no means imply that the min leader includes a
        text margin!
        -------------------------------------------------------------------------------*/
        if (DIM_ANGULAR(static_cast<DimensionType>(pAdimProcess->GetDimElementCP()->dimcmd)))
            insideLength = pAdimProcess->GetDimElementCP()->geom.termWidth;
        else if (isThruTerm)
            insideLength = pAdimProcess->GetDimElementCP()->geom.textMargin;
        else
            insideLength = pAdimProcess->GetDimElementCP()->geom.termWidth + pAdimProcess->GetDimElementCP()->geom.textMargin;

        outsideLength = pAdimProcess->GetDimElementCP()->geom.termWidth;
        if (!pAdimProcess->flags.fitTermsInside)
            outsideLength *= 2;
        }

    /* preserve legacy behavior for 0 length of min. leader */
    if (0.0 == insideLength)
        insideLength = 3 * pAdimProcess->strDat.charWidth;
    if (0.0 == outsideLength)
        outsideLength = 3 * pAdimProcess->strDat.charWidth;

    if (NULL != pInside)
        *pInside = insideLength;

    if (NULL != pOutside)
        *pOutside = outsideLength;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/06
+---------------+---------------+---------------+---------------+---------------+------*/
Public double   BentleyApi::adim_projectTextSize
(
DPoint2dCP      pTextSize,
DPoint3dCP      pTextDir,
double          textMargin
)
    {
    /* project text width on dimension line which is defined by pTextDir */
    double  boxWidth  = pTextSize->x + 2 * textMargin;
    double  boxHeight = pTextSize->y + 2 * textMargin;

    double  projectedWidth = boxWidth * fabs(pTextDir->x) + boxHeight * fabs(pTextDir->y);

    return  projectedWidth;
    }
