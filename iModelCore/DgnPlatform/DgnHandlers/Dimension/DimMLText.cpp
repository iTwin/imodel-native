/*----------------------------------------------------------------------+
|
|   $Source: DgnHandlers/Dimension/DimMLText.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
 
const WChar DIMMLTEXT_ValuePlaceHolder[2] = {'*',0};

#define DIMMLTEXT_FractionStart             (L"\\S")
#define DIMMLTEXT_FractionSeparator         (L"/")
#define DIMMLTEXT_FractionEnd               (L";")

#define DIMMLTEXT_SuperscriptStart          (L"\\U")
#define DIMMLTEXT_SuperscriptEnd            (L";")

/*-----------------------------------------------------------------------
Needs work: the 255 limit is due to the numChars:8 in below struct
dimformattedtext.  Will revise this in the future.
------------------------------------------------------------------------*/
#define DIMTEXT_MAX_STRINGLENGTH            255

typedef struct formatptr
    {
    DimFormattedText    *pFormatter;
    int                 row;
    } FormatPtr;

struct DgnPlatform::DimMLText
    {
    int             nFormats;
    FormatPtr       *pFormats;
    bool            enumerationIsValid;
    };

static void    dimTextBlock_getViewRotation
(
AdimProcess     *pAdimProcess
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
DimFormattedText::DimFormattedText ()
    {
    m_component  = 0;
    m_nodeNumber = 0;
    m_rotation   = 0.0;

    m_origin.zero();
    m_size.setComponents (1.0, 1.0);
    m_scale.setComponents (1.0, 1.0);

    memset (&m_textParams, 0, sizeof(m_textParams));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
int             DimFormattedText::GetLinkageMaxNumBytes () const
    {
    return (sizeof (*this) + GetVariStringNumBytes ());
    }

#if defined (CAN_REMOVE)
/*---------------------------------------------------------------------------------**//**
* NEEDSWORK: replaces mdlDimFormattedText_getSize
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
int             DimFormattedText::GetSize () const
    {
    int     structSize = sizeof (*this);

    structSize += GetStringSize ();
    structSize += GetWideParamBlobSize ();

    return  structSize;
    }

/*---------------------------------------------------------------------------------**//**
* NEEDSWORK: replaces mdlDimFormattedText_getStringCharCount
* Return the number of (VariChar) characters in string.
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
int             DimFormattedText::GetStringCharCount () const
    {
    return flags.numChars;
    }

/*---------------------------------------------------------------------------------**//**
* NEEDSWORK: replaces mdlDimFormattedText_getStringSize
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
int             DimFormattedText::GetStringSize () const
    {
    return (sizeof (char)) * GetStringCharCount ();
    }

/*---------------------------------------------------------------------------------**//**
* NEEDSWORK: replaces mdlDimFormattedText_getWideParamBlobSize
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
int             DimFormattedText::GetWideParamBlobSize () const
    {
    return paramsSize;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* NEEDSWORK: replaces mdlDimFormattedText_getString
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
WString         DimFormattedText::GetString (DgnProjectR dgnFile) const
    {
    int     numBytes   = GetVariStringNumBytes();

    if (0 >= numBytes)
        return L"";

    char*   variString = (char*) _alloca (numBytes + 2);

    memcpy (variString, GetVariString(), numBytes);
    variString[numBytes]   = '\0';
    variString[numBytes+1] = '\0';

    UInt16  fontNum     = (UInt16)m_textParams.font;
    UInt16  shxBigFontNum  = (UInt16)m_textParams.shxBigFont;

    return AdimStringUtil::WStringFromVariChar (variString, fontNum, shxBigFontNum, dgnFile);
    }

/*---------------------------------------------------------------------------------**//**
* NEEDSWORK: replaces mdlDimFormattedText_setString
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimFormattedText::SetString (WCharCP unicodeString, DgnProjectR dgnFile)
    {
    if (NULL == unicodeString)
        { BeAssert (false); return; }
    
    DgnFontCR           fontForCodePage = DgnFontManager::GetFontForCodePage (m_textParams.font, m_textParams.shxBigFont, dgnFile);
    bvector<VariChar>   variBuffer;
    
    VariCharConverter::UnicodeToVariChar (variBuffer, unicodeString, fontForCodePage.GetCodePage (), false);

    SetVariString (&variBuffer[0], static_cast<int>(variBuffer.size ()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
TextParamWide   DimFormattedText::GetTextParamWide () const
    {
    return m_textParams;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimFormattedText::SetTextParamWide (TextParamWideCR params)
    {
    m_textParams = params;

    // We don't store annotationscale value in the text linkage. So turn off
    // the corresponding flag in case it was left on in legacy dimensions.
    m_textParams.exFlags.annotationScale = false;
    }

/*---------------------------------------------------------------------------------**//**
* NEEDSWORK: replaces mdlDimFormattedText_getText
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
const char*     DimFormattedText::GetVariString () const
    {
    return (CharCP) &m_variStringBuffer[0];
    }

/*---------------------------------------------------------------------------------**//**
* NEEDSWORK: replaces mdlDimFormattedText_setText
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimFormattedText::SetVariString (const char* pString, int nBytesIn)
    {
    /*-----------------------------------------------------------------------------------
    The structure of DimFormattedText needs to be reworked in the future to allow larger
    numChars for text string, but for now to avoid memory error we simply truncate the
    string to the max size of DIMTEXT_MAX_STRINGLENGTH (currently 255).
    -----------------------------------------------------------------------------------*/
    int numBytes = nBytesIn > DIMTEXT_MAX_STRINGLENGTH ? DIMTEXT_MAX_STRINGLENGTH : nBytesIn;

    m_variStringBuffer.resize (numBytes, 0);

    char* variP = (char*)&(m_variStringBuffer)[0];
    memcpy (variP, pString, numBytes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
TextFormattingLinkage        DimFormattedText::GetTextParamAsLinkageData () const
    {
    TextFormattingLinkage    linkageData;

    linkageData.InitializeFromTextParams (m_textParams);

    return linkageData;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimFormattedText::SetTextParamFromLinkageData (TextFormattingLinkageCR textLink)
    {
    TextParamWide textParams;

    textLink.FillTextParams (textParams);

    SetTextParamWide (textParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       mdlDimFormattedText_scale
(
DimFormattedText    **ppFmt,
double              scale
)
    {
// NEEDSWORK: make this a method?
    if (NULL == ppFmt || NULL == *ppFmt)
        { BeAssert (0); return ERROR; }

    (*ppFmt)->SetWidth  (scale * (*ppFmt)->GetWidth());
    (*ppFmt)->SetHeight (scale * (*ppFmt)->GetHeight());

    TextParamWide   textParamWide = (*ppFmt)->GetTextParamWide ();
    DPoint2d        scaleFactor = { scale, scale };

    if (textParamWide.ApplyScaleFactor (scaleFactor, (*ppFmt)->IsNodeComponent(), false))
        (*ppFmt)->SetTextParamWide (textParamWide);

    DPoint2d glyphSize = (*ppFmt)->GetScale();
    glyphSize.x *= scale;
    glyphSize.y *= scale;
    (*ppFmt)->SetScale (glyphSize);

    DPoint2d origin = (*ppFmt)->GetOrigin();
    origin.x *= scale;
    origin.y *= scale;
    (*ppFmt)->SetOrigin(origin);

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt        mdlDimFormattedText_dump
(
const DimFormattedText *pFmt,
const char             *pFileNameIn,
const bool              bSimpleDump,
DgnProjectP              dgnFile
)
    {
    FILE    *pFile = NULL;

    if (NULL != pFmt)
        {
        if (NULL == pFileNameIn || strlen (pFileNameIn) <= 0 || NULL == (pFile = fopen (pFileNameIn, "w")))
            pFile = stdout;

        if (! bSimpleDump)
            {
            fprintf (pFile, "--- Beginning of dump ---\n");
            fprintf (pFile, "\tDimFormattedText dump at %p\n", (void*)pFmt);
            fprintf (pFile, "\tNum char bytes %d\n",   pFmt->GetVariStringNumBytes ());
            }

        WString     unicodeString = pFmt->GetString(*dgnFile); 

        if (! bSimpleDump)
            fprintf (pFile, "\tString: [%ls]\n", unicodeString.c_str());
        else
            fprintf (pFile, "%ls", unicodeString.c_str());

        if (! bSimpleDump)
            {
            DPoint2d origin = pFmt->GetOrigin();
            fprintf (pFile, "\tOrigin: (%g, %g)\n", origin.x, origin.y);

            DPoint2d scale = pFmt->GetScale();
            fprintf (pFile, "\tScale: x=%g, y=%g\n", scale.x, scale.y);

            fprintf (pFile, "\tText width: %g\n", pFmt->GetWidth());
            fprintf (pFile, "\tText height %g\n", pFmt->GetHeight());
            }

        TextParamWide   wideParam = pFmt->GetTextParamWide();

        if (! bSimpleDump)
            {
            fprintf (pFile, "\tFont: %u\n", (unsigned int)wideParam.font);
            }
        else
            {
            if (wideParam.exFlags.crCount)
                {
                UInt16 index;

                for (index = 0; index < wideParam.exFlags.crCount; index++)
                    fprintf (pFile, "\n");
                }
            }

        if (! bSimpleDump)
            fprintf (pFile, "--- End of dump ---\n");

        if (stdout != pFile)
            fclose (pFile);
        }

    return  SUCCESS;
    }

/*=================================================================================**//**
*
* formatPtr_xxx functions
*
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    formatPtr_setRow
(
FormatPtr   *pFtr,
const int   row
)
    {
    if (NULL != pFtr)
        pFtr->row = row;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
static int     formatPtr_getRow
(
const FormatPtr   *pFtr
)
    {
    return  (NULL != pFtr) ? pFtr->row : -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    formatPtr_setFormatter
(
FormatPtr           *pFtr,
DimFormattedText    *pFmt
)
    {
    if (NULL != pFtr)
        pFtr->pFormatter = pFmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
static DimFormattedText        *formatPtr_getFormatter
(
const FormatPtr   *pFtr
)
    {
    return  (NULL != pFtr) ? pFtr->pFormatter : NULL;
    }

/*=================================================================================**//**
*
* mdlDimText_xxx functions
*
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
@Description    Creates multiline text holder for dimension.
* @param        ppText      IN pointer to be allocated
* @return       SUCCESS if the text holder can be allocated, otherwise ERROR
* @ALinkJoin    usmthmdlDimText_free
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDimText_create
(
DimMLText    **ppText
)
    {
    StatusInt       status = ERROR;

    if (NULL == ppText)
        return  ERROR;

    if (NULL != (*ppText = (DimMLText*)  calloc (1, sizeof (**ppText))))
        {
        status = SUCCESS;
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    mdlDimText_setEnumerationValidity
(
DimMLText   *pText,
bool        validityState
)
    {
    if (NULL != pText)
        pText->enumerationIsValid = validityState;
    }

#ifdef UNUSED_FUNCTION
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool            mdlDimText_isEnumerationValid
(
const DimMLText   *pText
)
    {
    return  (NULL != pText) ? pText->enumerationIsValid : false;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static int     mdlDimText_getFormatterCount
(
const DimMLText   *pText
)
    {
    return  (NULL != pText) ? pText->nFormats : -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    mdlDimText_setFormatterCount
(
DimMLText *pText,
int       nFormats
)
    {
    if (NULL != pText)
        pText->nFormats = nFormats;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
static FormatPtr       *mdlDimText_getFormatPtr
(
const DimMLText     *pText,
const int           nth
)
    {
    if (NULL == pText)
        return  NULL;

    if (NULL != pText->pFormats && -1 < nth && nth < mdlDimText_getFormatterCount (pText))
        {
        return  &pText->pFormats[nth];
        }

    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    mdlDimText_setFormatter
(
DimMLText           *pText,
DimFormattedText    *pFmt,
int                 nth
)
    {
    if (NULL == pText)
        return  ;

    if (NULL != pFmt && -1 < nth && nth < mdlDimText_getFormatterCount (pText))
        pText->pFormats[nth].pFormatter = pFmt;

    mdlDimText_setEnumerationValidity (pText, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
DimFormattedText*   BentleyApi::mdlDimText_getFormatter
(
const DimMLText   *pText,
const int         nth
)
    {
    return  formatPtr_getFormatter (mdlDimText_getFormatPtr (pText, nth));
    }

#ifdef UNUSED_FUNCTION
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static DimFormattedText*    mdlDimText_getLastFormatter
(
const DimMLText   *pText
)
    {
    return  mdlDimText_getFormatter (pText, mdlDimText_getFormatterCount (pText) - 1);;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
static DimFormattedText        *mdlDimText_getFormatterByType
(
const DimMLText *pText,
const int       searchType
)
    {
    DimFormattedText    *pFmt = NULL;
    int                 nth   = 0;
    bool                match = false;

    /*-----------------------------------------------------------------------------------
        We make an assumption that there will be only on formatter for pre/suffix
        and want to get that one as soon as possible
    -----------------------------------------------------------------------------------*/
    while (NULL != (pFmt = mdlDimText_getFormatter (pText, nth)))
        {
        if (searchType == pFmt->GetComponentID())
            {
            match = true;
            break;
            }

        pFmt = NULL;
        nth++;
        }

    return  match ? pFmt : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
DimFormattedText         *BentleyApi::mdlDimText_getNodeFormatter
(
const DimMLText *pText
)
    {
    return  mdlDimText_getFormatterByType (pText, DimFormattedText::COMPONENTID_NodeProperties);
    }

/*---------------------------------------------------------------------------------**//**
@Description    Releases multiline text holder for dimension.
* @param        ppText      IN pointer to be freed
* @return       SUCCESS if the text holder is valid and can be freed, otherwise ERROR
* @ALinkJoin    usmthmdlDimText_create
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDimText_free
(
DimMLText    **ppText
)
    {
    DimMLText   *pText = *ppText;

    if (NULL == ppText || NULL == *ppText)
        return  ERROR;

    if (pText->pFormats)
        {
        DimFormattedText    *pFmt = NULL;
        int                 i     = 0;

        while (NULL != (pFmt = mdlDimText_getFormatter (pText, i)))
            {
            DELETE_AND_CLEAR (pFmt);
            i++;
            }

        free (pText->pFormats);
        pText->pFormats = NULL;
        }

    free (*ppText);
    *ppText = NULL;

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt        BentleyApi::mdlDimText_addFormatter
(
DimMLText           *pText,
DimFormattedText    *pFmt
)
    {
    int             nFormats = 0;

    if (NULL == pText || NULL == pFmt)
        return  ERROR;

    nFormats = mdlDimText_getFormatterCount (pText);
    mdlDimText_setFormatterCount (pText, ++nFormats);

    pText->pFormats = (NULL == pText->pFormats) ?
                        (FormatPtr*) calloc (nFormats, sizeof (FormatPtr)) :
                        (FormatPtr*) realloc (pText->pFormats, nFormats * sizeof (FormatPtr));

    mdlDimText_setFormatter (pText, pFmt, nFormats - 1);
    mdlDimText_setEnumerationValidity (pText, false);

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt        BentleyApi::DgnPlatform::mdlDimText_traverseFormatters
(
DimMLText                       *pText,
PFDimTextFmtTraverseFunction    pUserTraverseFunction,
void                            *pUserData,
DgnModelP                    modelRef
)
    {
    int                 nth, component;
    int                 currentRow = 0;
    StatusInt           status     = SUCCESS;
    DimFormattedText    *pFmt      = NULL;
    FormatPtr           *pFtr      = NULL;

    if (NULL == pText || NULL == pUserTraverseFunction)
        return  ERROR;

    if (NULL == pText->pFormats)
        return  SUCCESS;

    // traverse in component order
    for (component = DimFormattedText::s_firstComponentID;
         component <= DimFormattedText::s_lastComponentID && SUCCESS == status;
         component++)
        {
        nth = 0;

        while (NULL != (pFtr = mdlDimText_getFormatPtr (pText, nth)))
            {
            formatPtr_setRow (pFtr, currentRow);
            pFmt = formatPtr_getFormatter (pFtr);

            if (pFmt->GetComponentID() == component)
                {
                if (NULL != pUserTraverseFunction)
                    pUserTraverseFunction (&pFmt, pUserData, modelRef, pText, formatPtr_getRow (pFtr));

                formatPtr_setFormatter (pFtr, pFmt);

                if (0 != pFmt->GetCrCount())
                    currentRow++;
                }

            if (SUCCESS != status)
                break;

            pFmt = NULL;
            nth++;
            }
        }

    mdlDimText_setEnumerationValidity (pText, SUCCESS == status);

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       scaleDimMLTextProcessFunction
(
DimFormattedText    **ppFmt,
void                *pData,
DgnModelP        modelRef,
DimMLText           *pText,
int                 currentRow
)
    {
    double  scale = (*((double *) pData));

    mdlDimFormattedText_scale (ppFmt, scale);

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Scale text components by uniform scale.
*
* @param        pText   <=> MLText context to scale
* @param        scaleIn  => scale factor
* @return       StatusInt
* @bsimethod                                                    petri.niiranen  08/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDimText_scale
(
DimMLText   *pText,
double      scaleIn
)
    {
    if (NULL == pText)
        return  ERROR;

    mdlDimText_traverseFormatters (pText, scaleDimMLTextProcessFunction, (void *) &scaleIn, NULL);

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       mdlDimText_setTextSizeFromDescr
(
DimMLText       *pText,
ElementHandleCR    textNodeElement
)
    {
    StatusInt           status = ERROR;
    DimFormattedText    *pFmt  = NULL;

    if (NULL == pText || !textNodeElement.IsValid())
        return  status;

    if (NULL != (pFmt = mdlDimText_getNodeFormatter (pText)))
        {
        TextBlockPtr text = TextBlock::Create (textNodeElement);
        if (!text.IsValid())
            return ERROR;

        pFmt->SetWidth(text->GetExactWidth ());
        pFmt->SetHeight (text->GetExactHeight ());
        status = SUCCESS;
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
typedef struct dimtextextractparams
    {
    DimMLText       *pText;
    Int16           component;
    DPoint3d        nodeOrigin;
    bool            hasValuePlaceHolder;
    DgnModelP    modelRef;
    } DimTextExtractParams;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    getOffsetFromNodeOrigin
(
DPoint2d                *pOriginOffset,
DimTextExtractParams const& extract,
DPoint3d                *pTextOrigin
)
    {
    DVec3d    tmpOffset;

    bsiDVec3d_subtractDPoint3dDPoint3d (&tmpOffset, &extract.nodeOrigin, pTextOrigin);
    pOriginOffset->x = tmpOffset.x;
    pOriginOffset->y = tmpOffset.y;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    toFormatter
(
DimFormattedText**      ppFmt,
ElementHandleCR            eh,
DimTextExtractParams const &   extract
)
    {
    DPoint2d        textSize;
    DPoint2d        origin;
    TextParamWide   textWide;
    RotMatrix       rMatrix;

    rMatrix.InitIdentity ();

    if (TEXT_NODE_ELM == eh.GetLegacyType())
        {
        TextNodeHandler::GetOrientation (eh, rMatrix);
        TextNodeHandler::GetFontSize    (eh, textSize);
        TextNodeHandler::GetTextParams  (eh, textWide);
        }
    else
        {
        TextElemHandler::GetOrientation (eh, rMatrix);
        TextElemHandler::GetFontSize    (eh, textSize);
        TextElemHandler::GetTextParams  (eh, textWide);
        }

    if (!textWide.flags.textStyle)
        {
        /* Text elements with no style, have the color in their dhdr.  For dimensions,
           we need to store the color in the linkage, so the line arranger can propagate
           it back out onto the dhdr when the text element is recreated */
        textWide.exFlags.color = true;
        textWide.color = eh.GetElementCP ()->GetSymbology().color;
        }

    // initialize formatter
    *ppFmt = new DimFormattedText ();

    (*ppFmt)->SetHeight         (textSize.y);
    (*ppFmt)->SetWidth          (textSize.x);
    (*ppFmt)->SetComponentID    (extract.component);
    (*ppFmt)->SetTextParamWide  (textWide);
    (*ppFmt)->SetRotation       (rMatrix.ColumnXAngleXY ());

    if (TEXT_NODE_ELM == eh.GetLegacyType())
        {
        (*ppFmt)->SetNodeNumber (eh.GetElementCP ()->ToText_node_2d().nodenumber);
        }
    else
        {
        DPoint3d    textOrigin;
        WString     uniString;

        TextElemHandler::GetElementOrigin (eh, textOrigin);
        TextElemHandler::GetString (eh, uniString);

        getOffsetFromNodeOrigin (&origin, extract, &textOrigin);

        (*ppFmt)->SetOrigin (origin);
        (*ppFmt)->SetScale  (textSize);
        (*ppFmt)->SetString (uniString.c_str(), extract.modelRef->GetDgnProject());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       formatterFromElement
(
DimFormattedText**      ppFmt,
ElementHandleCR            templateElm,
WStringCR               string,
DPoint3dCR              textOrigin,
RotMatrixCR             rMatrix,
DPoint2dCR              textSize,
TextParamWideCR         textParamWide,
DimTextExtractParams const& extract
)
    {
    if (string.empty())
        return  ERROR;

    EditElementHandle  eeh;

    TextElemHandler::CreateElement (eeh, &templateElm, textOrigin, rMatrix, textSize, textParamWide, string.c_str(), 
                                    extract.modelRef->Is3d(), *extract.modelRef);

    toFormatter (ppFmt, eeh, extract);

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @return       what the return value means
* @bsimethod                                                    petri.niiranen  08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    parseValueString
(
WStringR    pre,
WStringR    value,
WStringR    post,
WStringCR   original
)
    {
    WString::size_type offset;
    if (WString::npos == (offset = original.find(DIMMLTEXT_ValuePlaceHolder)))
        return;
    
    WString::const_iterator iter(original.begin() + offset);
    pre.append(original.begin(), iter);
    value.append(DIMMLTEXT_ValuePlaceHolder);
    post.append (iter+1, original.end());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       extractString (ElementHandleCR element, DimTextExtractParams& extract)
    {
    
    TextParamWide           textWide;
    int                     iElementType    = element.GetLegacyType();
    
    if (DimFormattedText::COMPONENTID_NodeProperties == extract.component)
        {
        if (TEXT_NODE_ELM == iElementType)
            {
            DimFormattedText*   pFmt    = NULL;

            if (NULL == (pFmt = mdlDimText_getNodeFormatter (extract.pText)))
                {
                toFormatter (&pFmt, element, extract);
                mdlDimText_addFormatter (extract.pText, pFmt);
                }
            else
                {
                TextNodeHandler::GetTextParams (element, textWide);

                pFmt->SetTextParamWide (textWide);
                }

            // exit immediately
            return  ERROR;
            }
        }
    else if (TEXT_ELM == iElementType)
        {
        DPoint2d            textSize;
        DimFormattedText    *pFmt = NULL;
        DPoint3d            textOrigin;
        DVec3d              tmpOffset;
        RotMatrix           rMatrix;
        int                 saveCrCount = 0;

        TextElemHandler::GetElementOrigin (element, textOrigin);
        TextElemHandler::GetOrientation (element, rMatrix);
        TextElemHandler::GetFontSize (element, textSize);
        TextElemHandler::GetTextParams (element, textWide);

        WString     uniString;
        DgnFontCR      effectiveFont = DgnFontManager::GetFontForCodePage (textWide.font, textWide.shxBigFont, extract.modelRef->GetDgnProject());

        TextElemHandler::GetString (element, uniString, effectiveFont);

        // check for value placeholder
        if (extract.hasValuePlaceHolder &&
            NULL != ::wcsstr (uniString.c_str(), DIMMLTEXT_ValuePlaceHolder))
            {
            DimFormattedText    *pPreFmt = NULL;
            DimFormattedText    *pPostFmt = NULL;
            DPoint2d            scale;
            WString             pre;
            WString             value;
            WString             post;

            /*---------------------------------------------------------------------------
                In order to be able of creating various formats for prefices and suffices
                value string must be chopped to different components: pre, value and post.
                Each of these components will have similar formatting but origin changes.
                Since these are separate formatters we are able to generate any formats
                between them.
            ---------------------------------------------------------------------------*/
            // chop string into three elements
            parseValueString (pre, value, post, uniString);

            // do not apply cr's to the chopped text until we reach the last since
            // all of these chopped texts should be on the same line.
            saveCrCount              = textWide.exFlags.crCount;
            textWide.exFlags.crCount = 0;

            tmpOffset.x = tmpOffset.y = tmpOffset.z = 0.0;
            if (SUCCESS == formatterFromElement (&pPreFmt, element, pre, textOrigin, rMatrix, textSize, textWide, extract))
                {
                scale = pPreFmt->GetScale();
                tmpOffset.x = scale.x;
                mdlDimText_addFormatter (extract.pText, pPreFmt);
                }

            // advance by previous size
            extract.component = DimFormattedText::COMPONENTID_Value;
            rMatrix.Multiply(tmpOffset);
            bsiDPoint3d_addDPoint3dDPoint3d (&textOrigin, &textOrigin, &tmpOffset);
            tmpOffset.x = tmpOffset.y = tmpOffset.z = 0.0;

            // if value is the last component then set cr's
            if (post.empty())
                textWide.exFlags.crCount = saveCrCount;

            if (SUCCESS == formatterFromElement (&pFmt, element, value, textOrigin, rMatrix, textSize, textWide, extract))
                {
                scale = pFmt->GetScale();
                tmpOffset.x = scale.x;
                mdlDimText_addFormatter (extract.pText, pFmt);
                }

            // now it is time to apply cr's if any
            textWide.exFlags.crCount = saveCrCount;

            // advance by previous size
            extract.component = DimFormattedText::COMPONENTID_PostValue;
            rMatrix.Multiply(tmpOffset);
            bsiDPoint3d_addDPoint3dDPoint3d (&textOrigin, &textOrigin, &tmpOffset);
            tmpOffset.x = tmpOffset.y = tmpOffset.z = 0.0;
            if (SUCCESS == formatterFromElement (&pPostFmt, element, post, textOrigin, rMatrix, textSize, textWide, extract))
                {
                scale = pPostFmt->GetScale();
                tmpOffset.x = scale.x;
                mdlDimText_addFormatter (extract.pText, pPostFmt);
                }
            }
        else
            {
            toFormatter (&pFmt, element, extract);
            mdlDimText_addFormatter (extract.pText, pFmt);
            }
        }

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt     setStringsFromTextNode
(
DimMLText       *pText,
ElementHandleCR    textNodeElement,
bool            bContainsValuePlaceHolder,
Int16           component
)
    {
    if (NULL == pText || !textNodeElement.IsValid())
        return  ERROR;

    DimTextExtractParams    extract;

    memset (&extract, 0, sizeof (extract));
    extract.pText               = pText;
    extract.component           = component;
    extract.hasValuePlaceHolder = bContainsValuePlaceHolder;
    extract.modelRef            = textNodeElement.GetDgnModelP();

    /*-----------------------------------------------------------------------------------
        to avoid duplicating arranging code assume that text in node is positioned
        correctly as per placement point so that only value change delta can be applied
        directly.
    -----------------------------------------------------------------------------------*/
    
    if (TEXT_NODE_ELM == textNodeElement.GetLegacyType())
        TextNodeHandler::GetUserOrigin (textNodeElement, extract.nodeOrigin);
    
    // Else we're a text element, and don't care about the origin anyway, so keep it 0,0,0.

    for (ChildElemIter child(textNodeElement, ExposeChildrenReason::Count); child.IsValid(); child = child.ToNext())
        extractString (child, extract);

    mdlDimText_setEnumerationValidity (pText, false);

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set multi line text node properties for dimension text using text node descriptor
* @param        pText           IN text holder
* @param        pNodeDescrIn    IN text node descriptor with text and formatting
* @return       SUCCESS if the text holder is valid and strings can be set.
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       mdlDimText_setNodeParamsFromTextNode
(
DimMLText       *pText,
ElementHandleCR    textNode
)
    {
    setStringsFromTextNode (pText, textNode, false, DimFormattedText::COMPONENTID_NodeProperties);
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt BentleyApi::mdlDimText_setStringsFromTextNode
(
DimMLText       *pText,
ElementHandleCR    textNodeElement,
bool            bContainsValuePlaceHolder
)
    {
    StatusInt   status;

    mdlDimText_setNodeParamsFromTextNode (pText, textNodeElement);

    if (SUCCESS == (status = setStringsFromTextNode (pText, textNodeElement,
                                                     bContainsValuePlaceHolder,
                                                     DimFormattedText::COMPONENTID_PreValue)))
        {
        // it is all right if set text size fails due to not having
        // node element. Get size then tries to recover the size
        // from value component.
        mdlDimText_setTextSizeFromDescr (pText, textNodeElement);
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDimText_setPrefixStringsFromTextNode
(
DimMLText       *pText,
ElementHandleCR    textNodeElement
)
    {
    return  setStringsFromTextNode (pText, textNodeElement, false, DimFormattedText::COMPONENTID_Prefix);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDimText_setSuffixStringsFromTextNode
(
DimMLText       *pText,
ElementHandleCR    textNodeElement
)
    {
    return  setStringsFromTextNode (pText, textNodeElement, false, DimFormattedText::COMPONENTID_Suffix);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool            mdlDimText_hasBodyText
(
DimMLText       *pText
)
    {
    FormatPtr           *pFtr  = NULL;
    int                 iNth   = 0;
    bool                bMatch = false;

    // the order of formatters is not an issue here, do shortest possible loop
    while (NULL != (pFtr = mdlDimText_getFormatPtr (pText, iNth)) && !bMatch)
        {
        switch (formatPtr_getFormatter(pFtr)->GetComponentID())
            {
            case DimFormattedText::COMPONENTID_PreValue:
            case DimFormattedText::COMPONENTID_PostValue:
                bMatch = true;
                break;

            default:
                break;
            }

        iNth++;
        }

    return  bMatch;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     mdlDimText_isValueOnly
(
DimMLText       *pText
)
    {
    return  !mdlDimText_hasBodyText (pText);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       getTextDescr
(
DimMLText       *pText,
EditElementHandleR textNodeElement,
ElementHandleCR    dimensionElementIn,
DPoint3d        *pOrigin,
DVec3d          *pDirection,
WCharCP       pwPlaceholderStr
)
    {
    if (!dimensionElementIn.IsValid())
        return  ERROR;

    // TR#263370 We are about to tweak the element so make a local copy.  The
    // pointer we have might point directly into the element cache protected memory.
    

    // we want to create the text with placeholder, so disable tolerance and dual for text generation
    
    
    EditElementHandle dimElement(dimensionElementIn, true);
    dimElement.GetElementP()->ToDimensionElmR().flag.dual       = 0;
    dimElement.GetElementP()->ToDimensionElmR().flag.tolerance  = 0;
    dimElement.GetElementP()->ToDimensionElmR().frmt.dualFormat = 0;
    
    NullOutput  output;
    NullContext context (&output);

    context.SetDgnProject (*dimElement.GetDgnProject());

    AdimProcess adimProcess(dimElement, &context);
    adimProcess.Init();

    WString         main[3];
    
    dimTextBlock_getViewRotation (&adimProcess);

    // assign placeholder string
    if (NULL == pwPlaceholderStr)
        main[0] = DIMMLTEXT_ValuePlaceHolder;
    else
        main[0] = pwPlaceholderStr;

    // push it into processor
    dimTextBlock_populateWithValues (&adimProcess, pText, &main[0]);
    textNodeElement.ReplaceElementDescr(dimTextBlock_getTextDescr(&adimProcess, pText, pOrigin, pDirection).get());

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       getTextParamWideFromDimension
(
TextParamWide   *pTextParamWide,
ElementHandleCR    element
)
    {
    LegacyTextStyle    textStyle;
    DimensionElm const  *pDim = &element.GetElementCP()->ToDimensionElm();

    if (!element.IsValid() || NULL == pTextParamWide)
        return  ERROR;

    memset (pTextParamWide, 0, sizeof (*pTextParamWide));

    if (SUCCESS == DimensionHandler::GetInstance().GetTextStyle (element, &textStyle))
        {
        DgnTextStylePtr dgnTextStyle = DgnTextStyle::Create (*element.GetDgnProject());
        DgnTextStylePersistence::Legacy::FromLegacyStyle(*dgnTextStyle, textStyle);
        *pTextParamWide = DgnTextStylePersistence::Legacy::ToTextParamWide(*dgnTextStyle, NULL, NULL);
        }

    pTextParamWide->just                = static_cast<int>(TextElementJustification::LeftMiddle);
    pTextParamWide->lineStyle_deprecated= 0;

    pTextParamWide->exFlags.color = 1;
    pTextParamWide->color         = pDim->text.b.useColor ? pDim->text.color : pDim->GetSymbology().color;

    pTextParamWide->exFlags.stackedFractionType  = pDim->text.b.stackedFractionType;
    pTextParamWide->exFlags.stackedFractionAlign = pDim->text.b.stackedFractionAlign;

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @return   bool        true if user text was found, false otherwise
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool         mdlDim_getTextComponentDescr
(
EditElementHandleR  textElement,
ElementHandleCR     elementIn,
int                 iSegmentNo,
int                 iPartType,
int                 iSubType
)
    {
    bool                bRealUserText = false;
    TextParamWide       textParamWide;
    DimStrings          dimStrings;
    DimStringConfig     config;
    WString*            pwString = NULL;
    int                 iTruePointNo;

    // get the existing strings from dimension
    BentleyApi::mdlDim_getTextPointNo (&iTruePointNo, elementIn, iSegmentNo);

    DimensionHandler* hdlr = dynamic_cast<DimensionHandler*> (&elementIn.GetHandler());
    if (NULL == hdlr)
        return false;
    
    hdlr->GetStrings (elementIn, dimStrings, iTruePointNo, &config);

    if (NULL == (pwString = adim_getSimpleStringPtrByType (&dimStrings, iPartType, iSubType)))
        {
        // Give up if there is no string for this partType, subType
        return false;
        }

    WChar str[MAX_DIMSTR+1] = { '\0' };

    if (pwString->empty())
        {
        // default string to value placeholder if empty
        wcscat (str, DIMMLTEXT_ValuePlaceHolder);
        }
    else
        {
        wcscat (str, pwString->c_str());
        bRealUserText = true;
        }

    // get current formatting
    DimMLText           *pText = NULL;
    DimFormattedText    *pFmt  = NULL;

    if (SUCCESS == mdlDimText_create (&pText) &&
        SUCCESS == mdlDim_getText (pText, elementIn, iSegmentNo) &&
        NULL != (pFmt = mdlDimText_getFormatterByType (pText, DimFormattedText::COMPONENTID_Value)))
        {
        textParamWide = pFmt->GetTextParamWide();
        }
    else
        {
        getTextParamWideFromDimension (&textParamWide, elementIn);
        }

    if (pText)
        mdlDimText_free (&pText);

    // Create a text element
    DPoint2d        textSize = { elementIn.GetElementCP()->ToDimensionElm().text.width, elementIn.GetElementCP()->ToDimensionElm().text.height };
    DPoint3d        origin = { 0, 0, 0 };

    RotMatrix       rMatrix;

    rMatrix.initIdentity();

    TextElemHandler::CreateElement (textElement, NULL, origin, rMatrix, textSize, textParamWide, str, 
                                    elementIn.GetElementCP()->ToDimensionElm().Is3d(), *elementIn.GetDgnModelP());

    return  bRealUserText;
    }

/*---------------------------------------------------------------------------------**//**
* Get DimMLText as an element descriptor.
* @param    pText           => text context
* @param    ppTextDescrOut  <= generated element descriptor
* @param    pElementIn      => element to supply defaults
* @param    modelRef        =>
* @param    pwPlaceholderStr => custom place holder str or NULL for default '*'.
* @return   StatusInt
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
 StatusInt     BentleyApi::mdlDimText_getTextDescr
(
DimMLText       *pText,
EditElementHandleR textNodeElement,
ElementHandleCR    dimElement,
WCharCP       pwPlaceholderStr
)
    {
    DPoint3d    origin;
    DVec3d      direction;

    origin.x    = origin.y = origin.z = 0.0;
    direction.x = 1.0;
    direction.y = direction.z = 0.0;

    return  getTextDescr (pText, textNodeElement, dimElement, &origin, &direction, pwPlaceholderStr);
    }

/*---------------------------------------------------------------------------------**//**
* Return dimension text as an element descriptor from given segment.
*
* @param        ppTextDescrOut  <=>
* @param        pElementIn      =>
* @param        iSegmentNo      =>
* @param        modelRef        =>
* @return       StatusInt
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt    DimensionHandler::GetTextDescr (ElementHandleCR dimElement, EditElementHandleR textElement, int iSegmentNo, int iPartType, int iSubType) const
    {
    StatusInt   status         = ERROR;
    DimMLText   *pText         = NULL;

    if (!dimElement.IsValid())
        return  ERROR;

    // extract legacy style text first
    if (mdlDim_getTextComponentDescr (textElement, dimElement, iSegmentNo, iPartType, iSubType))
        {
        // don't go any further since we already have user text
        return  SUCCESS;
        }

    // main text
    if ((ADTYPE_TEXT_SINGLE == iPartType || ADTYPE_TEXT_UPPER == iPartType) && ADSUB_NONE == iSubType)
        {
        if (SUCCESS == mdlDimText_create (&pText))
            {
            if (SUCCESS == mdlDim_getText (pText, dimElement, iSegmentNo))
                {
                status = mdlDimText_getTextDescr (pText, textElement, dimElement, NULL);
                }

            mdlDimText_free (&pText);
            }
        }

    return  status;
    }

/*=================================================================================**//**
*
* dimTextBlock functions
*
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
#define ZERO_ALLOCATE(p,s) \
        p=(WChar**)_alloca(s); \
        memset(p,0,s);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
typedef struct processflags
    {
    UInt16      tolerance:1;
    UInt16      tolmode:1;
    UInt16      dual:1;
    UInt16      dualInline:1;
    UInt16      stackedFractionType:2;
    UInt16      stackedFractionAlign:2;

    UInt16      reserved:8;
    } ProcessFlags;

typedef struct mltextprocess
    {
    AdimProcess*        pAdimProcess;
    WString*            strings;
    } MLTextProcess;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/02
+---------------+---------------+---------------+---------------+---------------+------*/
static  StatusInt      dimTextBlock_verifyStackedFractionMarkup
(
WCharCP       pString
)
    {
    if (NULL == (pString = ::wcsstr (pString, DIMMLTEXT_FractionStart)))
        return ERROR;

    if (NULL == (pString = ::wcsstr (pString, DIMMLTEXT_FractionSeparator)))
        return ERROR;

    if (NULL == (pString = ::wcsstr (pString, DIMMLTEXT_FractionEnd)))
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/04
+---------------+---------------+---------------+---------------+---------------+------*/
static  StatusInt  dimTextBlock_verifySuperscriptMarkup
(
WCharCP       pString
)
    {
    if (NULL == (pString = ::wcsstr (pString, DIMMLTEXT_SuperscriptStart)))
        return ERROR;

    if (NULL == (pString = ::wcsstr (pString, DIMMLTEXT_SuperscriptEnd)))
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BentleyApi::dimTextBlock_stringContainsMarkup
(
WCharCP   pString
)
    {
    return (SUCCESS == dimTextBlock_verifyStackedFractionMarkup (pString) ||
            SUCCESS == dimTextBlock_verifySuperscriptMarkup (pString));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Venkat.Kalyan                   05/2007
+---------------+---------------+---------------+---------------+---------------+------*/
static void    addText (TextBlockR textBlock, WCharCP string, size_t nChars, TextParamWideCP pTextParamWide, DPoint2dCP pTextSize, DgnProjectP dgnFile, LineAlignment lineAlignment)
    {
    RunProperties runProperties (*pTextParamWide, *pTextSize, textBlock.GetDgnModelR ());
    runProperties.SetLineAlignment (lineAlignment);
    
    textBlock.SetRunPropertiesForAdd (runProperties);
    if (nChars > 0)
        {
        WString croppedString (string, nChars);
        textBlock.AppendText (croppedString.c_str());
        }

    if (0 != pTextParamWide->exFlags.crCount)
        {
        for (size_t i = 0; i < pTextParamWide->exFlags.crCount; ++i)
            textBlock.AppendParagraphBreak ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BentleyApi::dimTextBlock_appendMarkedUpString
(
TextBlockR      textBlock,
WCharCP         pwString,
TextParamWide*  pTextParamWide,
DPoint2dCP      pTextSize,
AdimProcess*    pAdimProcess
)
    {
    StackedFractionType dimFractionType = BentleyApi::adim_textFracTypeFromDim (pAdimProcess->GetDimElementCP()->text.b.stackedFractionType);

    /*--------------------------------------------------------------------------
        NOTE: This function implements a very specific and fragile markup
              parser.  It assumes that the string can contain either:
                1) No Markup, OR
                2) Exactly one stacked fraction, OR
                3) Exactly one superscript

        A fraction is denoted by the pattern "\S" - "num" - "/" - "den" - ";"
        A superscript is denoted by the pattern "\U" - "super" - ";"

        The eventual goal should be general markup parsing of both the value
        and user text.  This would allow very general text formatting beyond
        just stacked fractions and superscripts.
    --------------------------------------------------------------------------*/
    bool    hasFraction = StackedFractionType::None != dimFractionType &&
                          SUCCESS == dimTextBlock_verifyStackedFractionMarkup (pwString);

    bool    hasSuperscript = pAdimProcess->GetDimElementCP()->frmt.superscriptLSD &&
                             SUCCESS == dimTextBlock_verifySuperscriptMarkup (pwString);

    if ( !hasFraction && !hasSuperscript)
        {
        addText (textBlock, pwString, wcslen (pwString), pTextParamWide, pTextSize, &pAdimProcess->GetDgnModelP()->GetDgnProject(), LINE_ALIGNMENT_Bottom);
        }
    else
        {
        int                 crCount;
        ptrdiff_t           tokenLength;
        int                 fractionAlign;
        StackedFractionType fractionType;
        WCharCP             pTokenStart, pTokenEnd;
        WChar               wNumBuf[64], wDenBuf[64];
        WChar               markupInducer[5], numeratorTerminator[5];
        DPoint2d            fractionScale;
        DimensionElm const* pDim = pAdimProcess->GetDimElementCP();

        if (hasFraction)
            {
            double      dFractionScale = 1.0;

            fractionType  = dimFractionType;
            fractionAlign = pDim->text.b.stackedFractionAlign;

            if (pAdimProcess->pOverrides)
                mdlDim_extensionsGetStackedFractionScale (&dFractionScale, pAdimProcess->pOverrides, 1.0);

            fractionScale.x = fractionScale.y = dFractionScale;

            wcscpy (markupInducer, DIMMLTEXT_FractionStart);
            wcscpy (numeratorTerminator, DIMMLTEXT_FractionSeparator);
            }
        else
            {
            fractionType    = StackedFractionType::NoBar;
            fractionAlign   = static_cast<int>(StackedFractionAlignment::Middle);
            fractionScale.x = fractionScale.y = 0.7;

            wcscpy (markupInducer, DIMMLTEXT_SuperscriptStart);
            wcscpy (numeratorTerminator, DIMMLTEXT_SuperscriptEnd);
            }

        // Zero crCount so we don't get any newlines within the fraction
        crCount = pTextParamWide->exFlags.crCount;
        pTextParamWide->exFlags.crCount = 0;

        // add pre fraction text
        pTokenStart = pwString;
        pTokenEnd = ::wcsstr (pwString, markupInducer);
        tokenLength = pTokenEnd - pTokenStart;
        addText (textBlock, pTokenStart, tokenLength, pTextParamWide, pTextSize, &pAdimProcess->GetDgnModelP()->GetDgnProject(), (LineAlignment) fractionAlign);

        // find numerator
        pTokenStart = pTokenEnd + 2;
        pTokenEnd = ::wcsstr (pTokenStart, numeratorTerminator);
        tokenLength = pTokenEnd - pTokenStart;
        wcsncpy (wNumBuf, pTokenStart, tokenLength);
        wNumBuf[tokenLength] = '\0';

        // find denominator
        if ( !hasFraction)
            {
            wDenBuf[0] = '\0';
            }
        else
            {
            pTokenStart = pTokenEnd + 1;
            pTokenEnd = ::wcsstr (pTokenStart, DIMMLTEXT_FractionEnd);
            tokenLength = pTokenEnd - pTokenStart;
            wcsncpy (wDenBuf, pTokenStart, tokenLength);
            wDenBuf[tokenLength] = '\0';
            }

        // add fraction
        RunProperties runProperties (*pTextParamWide, *pTextSize, textBlock.GetDgnModelR ());
        textBlock.SetRunPropertiesForAdd (runProperties);
        textBlock.AppendStackedFraction ((WCharCP)wNumBuf, (WCharCP)wDenBuf, fractionType, (StackedFractionAlignment) fractionAlign, &fractionScale);

        // restore cr count for newlines after the fraction
        pTextParamWide->exFlags.crCount = crCount;

        // post fraction text
        pTokenStart = pTokenEnd + 1;
        pTokenEnd = pwString + wcslen (pwString);
        tokenLength = pTokenEnd - pTokenStart;
        addText (textBlock, pTokenStart, tokenLength, pTextParamWide, pTextSize, &pAdimProcess->GetDgnModelP()->GetDgnProject(), (LineAlignment) fractionAlign);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    dimTextBlock_appendStackedValue
(
AdimProcess         *pAdimProcess,
WCharCP             pwStringIn,
TextParamWide const* pTextParamWideIn,
DPoint2d const*      pTextSizeIn
)
    {
    if (pAdimProcess->m_textBlock.IsNull())
        return;

    DgnProjectP          dgnFile = &pAdimProcess->GetDgnModelP()->GetDgnProject();
    TextParamWide       textParams = *pTextParamWideIn;
    textParams.exFlags.stackedFractionType = static_cast<UInt32>(StackedFractionType::None);

    WChar             fraction[4][MAX_DIMSTR];
    if (NULL != pTextParamWideIn &&
        StackedFractionType::None != static_cast<StackedFractionType>(textParams.exFlags.stackedFractionType) &&
        SUCCESS == BentleyApi::adim_parseFraction (fraction[0], fraction[1], fraction[2], fraction[3], pwStringIn))
        {
        double                      dSpaceWidth = 0.5 * pTextSizeIn->x;
        StackedFractionType         fractionType = (StackedFractionType) textParams.exFlags.stackedFractionType;
        StackedFractionAlignment    fractionAlignment = (StackedFractionAlignment) textParams.exFlags.stackedFractionAlign;

        addText (*pAdimProcess->m_textBlock, fraction[0], wcslen (fraction[0]), pTextParamWideIn, pTextSizeIn, dgnFile, (LineAlignment) fractionAlignment);

        RunProperties runProperties (*pTextParamWideIn, *pTextSizeIn, pAdimProcess->m_textBlock->GetDgnModelR ());
        pAdimProcess->m_textBlock->SetRunPropertiesForAdd (runProperties);
        pAdimProcess->m_textBlock->AppendFixedSpace (dSpaceWidth);

        pAdimProcess->m_textBlock->AppendStackedFraction (fraction[1], fraction[2], fractionType, fractionAlignment, NULL);
        pAdimProcess->m_textBlock->AppendFixedSpace (dSpaceWidth);
        addText (*pAdimProcess->m_textBlock, fraction[3], wcslen (fraction[3]), pTextParamWideIn, pTextSizeIn, dgnFile, (LineAlignment) fractionAlignment);
        }
    else
        {
        // simple straight text
        addText (*pAdimProcess->m_textBlock, pwStringIn, wcslen (pwStringIn), pTextParamWideIn, pTextSizeIn, dgnFile, LINE_ALIGNMENT_Bottom);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    dimTextBlock_processValue
(
AdimProcess*    pAdimProcess,
TextParamWide*  pTextParamWide,
DPoint2d*       pTextSize,
WString*       ppwValue
)
    {
    if (NULL == pAdimProcess ||pAdimProcess->m_textBlock.IsNull() || NULL == pAdimProcess->GetDimElementCP() || NULL == pTextParamWide)
        return;

    TextBlockPtr textBlock      = pAdimProcess->m_textBlock;
    DimensionElm const *pDim    = pAdimProcess->GetDimElementCP();

    /*-------------------------------------------------------------------------------
        There is a limitation of how fraction can be used in tolerances.
        Value can have fractions formatted, but tolerances both limit and plus/minus
        will only have inline representation.
    -------------------------------------------------------------------------------*/
    RunProperties runProperties (*pTextParamWide, *pTextSize, pAdimProcess->m_textBlock->GetDgnModelR ());
    pAdimProcess->m_textBlock->SetRunPropertiesForAdd (runProperties);
    
    if (pDim->flag.tolerance)
        {
        DimTolrBlock    *pTolBlock = NULL;

        if (NULL == (pTolBlock = (DimTolrBlock*) mdlDim_getOptionBlock (pAdimProcess->GetElemHandleCR(), ADBLK_TOLERANCE, NULL)))
            return  ;

        if (pDim->flag.tolmode)
            {
            // limit
            if ((0 == pTolBlock->lowerTol) && (0 == pTolBlock->upperTol))
                {
                ADIM_SETNAME (pAdimProcess->partName, ADTYPE_INHERIT, ADSUB_LIM_SINGLE);
                dimTextBlock_appendStackedValue (pAdimProcess, ppwValue[0].c_str(), pTextParamWide, pTextSize);
                }
            else
                {
                /*------------------------------------------------------------------------------
                * NEEDS_WORK: The original code attaches separate and distinct part names for the
                * upper and lower parts.  I am not sure how to do it as yet. Neither am I sure
                * as to how it is used since it is not persistent. - Need further discussions
                * with Josh.
                ADIM_SETNAME (pAdimProcess->partName, ADTYPE_INHERIT, ADSUB_LIM_UPPER);
                ADIM_SETNAME (pAdimProcess->partName, ADTYPE_INHERIT, ADSUB_LIM_LOWER);
                -------------------------------------------------------------------------------*/
                pAdimProcess->m_textBlock->AppendStackedFraction (ppwValue[0].c_str(), ppwValue[1].c_str(), (StackedFractionType) StackedFractionType::NoBar,
                                                    (StackedFractionAlignment) pDim->text.b.stackedFractionAlign, NULL);
                }
            }
        else
            {
            DPoint2d   textSize;

            // pm
            ADIM_SETNAME (pAdimProcess->partName, ADTYPE_INHERIT, ADSUB_NONE);
            dimTextBlock_appendStackedValue (pAdimProcess, ppwValue[0].c_str(), pTextParamWide, pTextSize);

            textBlock->AppendFixedSpace (pTolBlock->tolHorizSep);

            if (! pTolBlock->flags.stackEqual && pTolBlock->lowerTol == pTolBlock->upperTol)
                {
                ADIM_SETNAME (pAdimProcess->partName, ADTYPE_INHERIT, ADSUB_TOL_SINGLE);
                dimTextBlock_appendStackedValue (pAdimProcess, ppwValue[1].c_str(), pTextParamWide, pTextSize);
                }
            else
                {
                /*------------------------------------------------------------------------------
                * NEEDS_WORK: The original code attaches separate and distinct part names for the
                * upper and lower parts.  I am not sure how to do it as yet. Neither am I sure
                * as to how it is used since it is not persistent. - Need further discussions
                * with Josh.
                ADIM_SETNAME (pAdimProcess->partName, ADTYPE_INHERIT, ADSUB_LIM_UPPER);
                ADIM_SETNAME (pAdimProcess->partName, ADTYPE_INHERIT, ADSUB_LIM_LOWER);
                -------------------------------------------------------------------------------*/
                ADIM_SETNAME (pAdimProcess->partName, ADTYPE_INHERIT, ADSUB_TOL_UPPER);
                textSize   = *pTextSize;
                textSize.x = pTolBlock->tolWidth;
                textSize.y = pTolBlock->tolHeight;
                pAdimProcess->m_textBlock->AppendStackedFraction (ppwValue[1].c_str(), ppwValue[2].c_str(), (StackedFractionType) StackedFractionType::NoBar,
                                                    (StackedFractionAlignment) pDim->text.b.stackedFractionAlign, NULL);
                }
            }
        }
    else
        {
        // "single" text
        ADIM_SETNAME (pAdimProcess->partName, ADTYPE_INHERIT, ADSUB_NONE);
        dimTextBlock_appendMarkedUpString (*pAdimProcess->m_textBlock, ppwValue[0].c_str(), pTextParamWide, pTextSize, pAdimProcess);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    dimTextBlock_insertSeparator
(
AdimProcess*    pAdimProcess,
WChar         separatorChar,
TextParamWide*  pTextParamWide,
DPoint2d*       pTextSize
)
    {
    WChar         dualSeparator[2] = { separatorChar, 0 };
    TextParamWide   textParam;
    DPoint2d        textSize;

    textParam                 = *pTextParamWide;
    textParam.exFlags.crCount = 0;

    // apparently text size requires some scaling...
    textSize = *pTextSize;
    textSize.x *= 2.5;
    textSize.y *= 2.5;

    addText (*pAdimProcess->m_textBlock, dualSeparator, 1, &textParam, &textSize, &pAdimProcess->GetDgnModelP()->GetDgnProject(), LINE_ALIGNMENT_Bottom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    dimTextBlock_appendValues
(
AdimProcess*    pAdimProcess,
TextParamWide*  pTextParamWide,
DPoint2d*       pTextSize,
WString*        strings
)
    {
    DimensionElm const* pDim = pAdimProcess->GetDimElementCP();
    RunProperties   runProperties   (*pTextParamWide, *pTextSize, pAdimProcess->m_textBlock->GetDgnModelR ());

    if (! pDim->flag.dual)
        {
        ADIM_SETNAME (pAdimProcess->partName, ADTYPE_TEXT_SINGLE, ADSUB_NONE);
        dimTextBlock_processValue (pAdimProcess, pTextParamWide, pTextSize, strings);
        }
    else
        {
        DimMLText   *pText = NULL;

        // Traditional dual is allowed even with complex text (although not with mutliple lines).
        mdlDim_overridesGetDimMLText (&pText, pAdimProcess->pOverrides, ADIM_GETSEG (pAdimProcess->partName));

        if ( ! pDim->frmt.dualFormat)       // above/below dual
            {
            ADIM_SETNAME (pAdimProcess->partName, ADTYPE_TEXT_UPPER, ADSUB_NONE);
            dimTextBlock_processValue (pAdimProcess, pTextParamWide, pTextSize, strings);
            }
        else                                // inline dual
            {
            WChar  sepChar;
            WChar  extraSpace[2] = {32, 0}; // separate main and dual value at least by space

            // main value
            ADIM_SETNAME (pAdimProcess->partName, ADTYPE_TEXT_UPPER, ADSUB_NONE);
            dimTextBlock_processValue (pAdimProcess, pTextParamWide, pTextSize, strings);

            addText (*pAdimProcess->m_textBlock, extraSpace, 1, pTextParamWide, pTextSize, &pAdimProcess->GetDgnModelP()->GetDgnProject(), LINE_ALIGNMENT_Bottom);
            
            ElementHandleCR dimElement = pAdimProcess->GetElemHandleCR();
            // inline dual measure
            if (SUCCESS == BentleyApi::mdlDim_getAlternateSeparator (&sepChar, dimElement, true))
                dimTextBlock_insertSeparator (pAdimProcess, sepChar, pTextParamWide, pTextSize);

            ADIM_SETNAME (pAdimProcess->partName, ADTYPE_TEXT_LOWER, ADSUB_NONE);
            dimTextBlock_processValue (pAdimProcess, pTextParamWide, pTextSize,  &strings[3]);

            if (SUCCESS == BentleyApi::mdlDim_getAlternateSeparator (&sepChar, dimElement, false))
                dimTextBlock_insertSeparator (pAdimProcess, sepChar, pTextParamWide, pTextSize);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/08
+---------------+---------------+---------------+---------------+---------------+------*/
static void    addFraction (TextBlockR textBlock, WCharCP numerator, WCharCP denominator, TextParamWideCR textParamWide, DPoint2dCR textSize, DgnProjectP dgnFile)
    {
    DPoint2d            scale     = {1.0, 1.0};
    StackedFractionType        type      = (StackedFractionType)      textParamWide.exFlags.stackedFractionType;
    StackedFractionAlignment   alignment = (StackedFractionAlignment) textParamWide.exFlags.stackedFractionAlign;
    RunProperties       runProperties (textParamWide, textSize, textBlock.GetDgnModelR ());

    textBlock.SetRunPropertiesForAdd (runProperties);
    textBlock.AppendStackedFraction (numerator, denominator, type, alignment, &scale);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/08
+---------------+---------------+---------------+---------------+---------------+------*/
static void    addFractionComponent (TextBlockR textBlock, WCharCP string, TextParamWideCR textParamWide, DPoint2dCR textSize, DgnProjectP dgnFile)
    {
    /*--------------------------------------------------------------------------
      Here, we are building a text block from stored MLTextLinkages.  The
      fraction numerator and denominator come through independently.  Strategy
      is to add the numerator as a fraction run, and then modify that run
      when the denominator comes along.

      NOTE: it is valid to have a fraction that is missing numerator or
      denominator.  That would be drawn as a super or subscript.
    --------------------------------------------------------------------------*/

    // For numerators, create a fraction with a numerator only
    if (StackedFractionSection::Numerator == static_cast<StackedFractionSection>(textParamWide.exFlags.stackedFractionSection))
        {
        addFraction (textBlock, string, NULL, textParamWide, textSize, dgnFile);
        return;
        }

    if (StackedFractionSection::Denominator != static_cast<StackedFractionSection>(textParamWide.exFlags.stackedFractionSection))
        { BeAssert (0); return; }

    // Check if the last run is already a fraction
    Caret caret = textBlock.End();
    FractionCP  fraction = dynamic_cast <FractionCP> (caret.GetCurrentRunCP ());

    // There is no existing fraction, create a fraction with denominator only
    if (NULL == fraction)
        {
        addFraction (textBlock, NULL, string, textParamWide, textSize, dgnFile);
        return;
        }

    // There is an existing fraction, modify it to include the denominator
    FractionP newFraction = (FractionP) fraction->Clone();

    newFraction->SetDenominatorText (string);

    TextBlockNodeArray units;
    units.push_back (newFraction);

    Caret end = textBlock.End();
    textBlock.ReplaceNodes (caret, end, units);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       populateWithValues
(
DimFormattedText    **ppFmt,
void                *pData,
DgnModelP        modelRef,
DimMLText           *pText,
int                 currentRow
)
    {
    TextParamWide       textParamWide;
    DPoint2d            size;
    MLTextProcess       *pTextProc     = (MLTextProcess *) pData;
    AdimProcess         *pAdimProcess  = pTextProc->pAdimProcess;
    DgnProjectP          dgnFile        = &pAdimProcess->GetDgnModelP()->GetDgnProject();

    switch ((*ppFmt)->GetComponentID())
        {
        case DimFormattedText::COMPONENTID_Value:
            {
            textParamWide = (*ppFmt)->GetTextParamWide();
            size = (*ppFmt)->GetTileSize();

            // make an assumption that value component has only placeholder.
            dimTextBlock_appendValues (pAdimProcess, &textParamWide, &size, pTextProc->strings);

            break;
            }

        case DimFormattedText::COMPONENTID_PreValue:
        case DimFormattedText::COMPONENTID_PostValue:
        case DimFormattedText::COMPONENTID_Prefix:
        case DimFormattedText::COMPONENTID_Suffix:
            {
            ADIM_SETNAME (pAdimProcess->partName, ADTYPE_TEXT_SINGLE, ADSUB_NONE);
            textParamWide = (*ppFmt)->GetTextParamWide();
            size = (*ppFmt)->GetTileSize();

            WString unicodeString = (*ppFmt)->GetString (*dgnFile);

            if (StackedFractionType::None    == static_cast<StackedFractionType>(textParamWide.exFlags.stackedFractionType) ||
                StackedFractionSection::None == static_cast<StackedFractionSection>(textParamWide.exFlags.stackedFractionSection))
                {
                addText (*pAdimProcess->m_textBlock, unicodeString.c_str(), wcslen (unicodeString.c_str()), &textParamWide, &size, dgnFile, LINE_ALIGNMENT_Bottom);
                }
            else
                {
                addFractionComponent (*pAdimProcess->m_textBlock, unicodeString.c_str(), textParamWide, size, dgnFile);
                }

            break;
            }

        case DimFormattedText::COMPONENTID_NodeProperties:
        default:
            break;
        }

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::dimTextBlock_setPopulated
(
AdimProcess     *pAdimProcess,
bool            bState
)
    {
    if (NULL != pAdimProcess)
        pAdimProcess->flags.textBlockPopulated = bState;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::dimTextBlock_isPopulated
(
AdimProcess     *pAdimProcess
)
    {
    return  (NULL != pAdimProcess) ? pAdimProcess->flags.textBlockPopulated : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt        BentleyApi::dimTextBlock_populateWithValues
(
AdimProcess     *pAdimProcess,
DimMLText       *pText,
WString*        strings
)
    {
    StatusInt       status;
    MLTextProcess   textProc;

    if (NULL == pAdimProcess || pAdimProcess->m_textBlock.IsNull())
        return  ERROR;

    textProc.pAdimProcess   = pAdimProcess;
    textProc.strings        = strings;

    DimensionElm const* pDim = pAdimProcess->GetDimElementCP();
    /*-----------------------------------------------------------------------------------
        append texts to arranger
    -----------------------------------------------------------------------------------*/
    if (NULL == pText)
        {
        TextParamWide   textParamWide;
        DPoint2d        size;

        size.x = pDim->text.width;
        size.y = pDim->text.height;

        //ADIM_SETNAME (pAdimProcess->partName, ADTYPE_MLTEXT, ADSUB_VALUE);
        getTextParamWideFromDimension (&textParamWide, textProc.pAdimProcess->GetElemHandleCR());
        dimTextBlock_appendValues (pAdimProcess, &textParamWide, &size, strings);
        status     = SUCCESS;
        }
    else
        {
        DimFormattedText    *pFormattedText = NULL;

        if (NULL != (pFormattedText = mdlDimText_getNodeFormatter (pText)))
            {
            TextParamWide   wide = pFormattedText->GetTextParamWide();

            ParagraphRange paragraphRange (*pAdimProcess->m_textBlock);
            pAdimProcess->m_textBlock->SetJustification ((TextElementJustification) wide.just, paragraphRange);
            pAdimProcess->m_textBlock->SetJustificationOverrideFlag (true, paragraphRange);
            pAdimProcess->m_textBlock->SetLineSpacingType ((DgnLineSpacingType) wide.exFlags.acadLineSpacingType, paragraphRange);
            pAdimProcess->m_textBlock->SetLineSpacingTypeOverrideFlag (true, paragraphRange);
            pAdimProcess->m_textBlock->SetLineSpacingValue (wide.lineSpacing, paragraphRange);
            pAdimProcess->m_textBlock->SetLineSpacingValueOverrideFlag (true, paragraphRange);
            }
        else
            {
            if (mdlDimText_getFormatterCount (pText) > 0 && NULL != (pFormattedText = mdlDimText_getFormatter (pText, 0)))
                {
                ParagraphRange paragraphRange (*pAdimProcess->m_textBlock);
                pAdimProcess->m_textBlock->SetJustification ((TextElementJustification) pFormattedText->GetJustification(), paragraphRange);
                pAdimProcess->m_textBlock->SetJustificationOverrideFlag (true, paragraphRange);
                }
            }

        status = mdlDimText_traverseFormatters (pText, populateWithValues, (void *) &textProc, pAdimProcess->GetDgnModelP());
        }

    // If we don't set a text node number, line arranger will assign one and bump
    // up the highest node number (tcb->canode).  The actual number assigned here
    // should not matter at all
    pAdimProcess->m_textBlock->SetNodeNumber (1);

    /*-----------------------------------------------------------------------------------
        call process that we are ready for next steps
    -----------------------------------------------------------------------------------*/
    pAdimProcess->m_textBlock->PerformLayout ();
    dimTextBlock_setPopulated (pAdimProcess, true);

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BentleyApi::dimTextBlock_getTextSize
(
AdimProcess*        pAdimProcess,
DPoint2dP           pSizeOut,
AdimTextSizeOption  option
)
    {
    if (NULL == pAdimProcess || pAdimProcess->m_textBlock.IsNull() || NULL == pSizeOut)
        return  ERROR;

    TextBlockPtr    textBlock = pAdimProcess->m_textBlock;
    DRange3d        range;

    if (ADIM_TEXTSIZE_Exact == option)
        range = textBlock->GetExactRange ();
    else
        range = textBlock->GetNominalRange ();

    if (!bsiDRange3d_isNull (&range))
        {
        pSizeOut->x = range.high.x - range.low.x;
        pSizeOut->y = range.high.y - range.low.y;
        }
    else
        pSizeOut->x = pSizeOut->y = 0.0;

    return  SUCCESS;
    }

/*----------------------------------------------------------------------+
|  The default symbology for dimension text should come from the dimension
|  text settings. To avoid getting the current element and/or text settings
|  by default, we set up a template text element w/ the dimension
|  text settings to pass into the text-node creation function.
|                                                                       |
| author    SamWilson                                   11/01           |
|                                                                       |
+----------------------------------------------------------------------*/
static int  adim_generateTextNodeTemplate
(
EditElementHandleR textElm,       /* <= */
AdimProcess  *ep,
DPoint3d     *rtxtorg,
RotMatrix    *rotMatrix
)
    {
    UInt16          maxLineLength = 0;
    DPoint2d        textSize = { 0.0, 0.0 };
    TextParamWide   textParams;

    if (SUCCESS == TextNodeHandler::CreateElement (textElm, NULL, *rtxtorg, *rotMatrix, textSize, textParams, 
                                                   maxLineLength, ep->GetDimElementCP()->Is3d(), *ep->GetDgnModelP()))
        {
        DgnElementP elem = textElm.GetElementP();       
        DimensionElm const* dim = ep->GetDimElementCP();
        //Symbology       symb = primitive->GetSymbology();
        elem->SetLevel(dim->GetLevelValue());
        elem->SetElementClass(DgnElementClass::Dimension);

        Symbology symb;
        adim_getEffectiveSymbology (&symb, ep->GetElemHandleCR(), DIM_MATERIAL_Text, ep->partName, ep->pOverrides);
        
        //WIP TODO test whether we get invalid symbology from a dimension. mdlElement_setSymbology2 used to do more
        //rather than a simple symbology assignment which would be sufficient here.
        elem->GetSymbologyR().color = symb.color;
        elem->GetSymbologyR().style = symb.style;
        elem->GetSymbologyR().weight = symb.weight;
        
        if (!dim->Is3d())
            elem->GetRangeR().low.z = elem->GetRangeR().high.z = 0.0;

        elem->SetDisplayPriority(dim->GetDisplayPriority());
        //TODO test we do this for symbology. Do we need to do transparency
        //mdlElement_setTransparency (elem, mdlElement_getTransparency ((DgnElementP) dim));
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
MSElementDescrPtr BentleyApi::dimTextBlock_getTextDescr
(
AdimProcess     *pAdimProcess,
DimMLText       *pText,
DPoint3d        *pTxtOrigin,
DVec3d          *pDirection
)
    {
    RotMatrix           rotMatrix;
    EditElementHandle      elTemplate;   // (has room for style linkage...)
    if (NULL == pAdimProcess || pAdimProcess->m_textBlock.IsNull() || NULL == pAdimProcess->GetDimElementCP())
        return  NULL;

    TextBlockPtr textBlock   = pAdimProcess->m_textBlock;
    DgnModelP modelRef    = pAdimProcess->GetDgnModelP();

    BentleyApi::adim_getRMatrixFromDir (&rotMatrix, pDirection, &pAdimProcess->rMatrix, &pAdimProcess->vuMatrix);

    //  The default symbology for dimension text should come from the dimension
    //  text settings. To avoid getting the current element and/or text settings
    //  by default, we set up and pass in a template text element w/ the dimension
    //  text settings.
    adim_generateTextNodeTemplate (elTemplate, pAdimProcess, pTxtOrigin, &rotMatrix);

    // Populate textblock
    if (! dimTextBlock_isPopulated (pAdimProcess))
        dimTextBlock_populateWithValues (pAdimProcess, pText, pAdimProcess->strDat.GetPrimaryStrings());

    //  Create text node representing all dimension text w/ styles applied
    EditElementHandle  editElemHandle;

    textBlock->SetUserOrigin (*pTxtOrigin);
    textBlock->SetOrientation (rotMatrix);
    textBlock->SetForceTextNodeFlag (TEXT_NODE_ELM == elTemplate.GetLegacyType());
    textBlock->ToElement (editElemHandle, modelRef, &elTemplate);

    if (!editElemHandle.IsValid ())
        return NULL;

    MSElementDescrPtr pTextDescr = editElemHandle.ExtractElementDescr();
    //  Communicate the actual text origin, rotation, and size to the dim stroker
    updateTextBoxFromTextBlock (pAdimProcess, *textBlock);
    
    // make sure the first element is correctly identified
    //pTextDescr->h.appData1 = pAdimProcess->partName;  removed in graphite
    return  pTextDescr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::dimTextBlock_getRequiresTextBlock
(
AdimProcess     *pAdimProcess,
DimMLText       **ppTextOut
)
    {
    DimMLText   *pText           = NULL;
    bool        bReqEnhancedText = false;

    if (NULL == pAdimProcess)
        return  false;

    /*-----------------------------------------------------------------------------------
        Enhanced text is required when:
        - a dimmltext holder is found or
        - enhanced fractions are used or
        - inline dual format is requested
    -----------------------------------------------------------------------------------*/
    mdlDim_overridesGetDimMLText (&pText, pAdimProcess->pOverrides, ADIM_GETSEG (pAdimProcess->partName));
    bReqEnhancedText = (pAdimProcess->flags.textBlockPopulated ||
                        NULL != pText ||
                        pAdimProcess->GetDimElementCP()->frmt.dualFormat);

    if (ppTextOut)
        *ppTextOut = pText;

    return  bReqEnhancedText;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    dimTextBlock_getViewRotation
(
AdimProcess     *pAdimProcess
)
    {
    DimViewBlock    *pViewBlock = (DimViewBlock *) mdlDim_getOptionBlock (pAdimProcess->GetElemHandleCR(), ADBLK_VIEWROT, NULL);

    //The version change handler should have burned a viewRotation into the dimension
    if (NULL == pViewBlock)
        { BeAssert (false); return; }

    if (pAdimProcess->GetDgnModelP()->Is3d())
        pAdimProcess->vuMatrix.InitTransposedFromQuaternionWXYZ (pViewBlock->viewRot);
    else
        pAdimProcess->vuMatrix.InitFromRowValuesXY (pViewBlock->viewRot);
    }
