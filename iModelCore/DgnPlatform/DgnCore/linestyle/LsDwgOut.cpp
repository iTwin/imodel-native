/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/LsDwgOut.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/DgnCore/LsLocal.h>

#define     LT_GAP      0
#define     LT_DASH     1
#define     LT_SYMB     2
#define     PREFIX_LENGTH   10
#define     DEFAULT_FONT_NAME       L"dgnlstyle"
#define     DGNLIB_PREFIX           L"dgnlib_"

/* This is the maximum number of segments for a DWG linetype as specified in OpenDWG */
#define     MAX_LineTypeSegments    12

USING_NAMESPACE_BENTLEY_DGNPLATFORM

#ifdef BEIJING_DGNPLATFORM_WIP_LINESTYLES
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
BEGIN_LINESTYLE_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     Chuck Kirschman   01/03
+===============+===============+===============+===============+===============+======*/
struct DecompLineType
    {
    struct DecompLineType*  prev;
    struct DecompLineType*  next;
    double                  start;
    double                  length;
    int                     type;
    SymbolRefP              symbolRef;
    };

/*=================================================================================**//**
* @bsiclass                                                     Chuck Kirschman   01/03
+===============+===============+===============+===============+===============+======*/
struct DecomposedDwgLine
    {
private:
    struct DecompLineType   *m_pLineList;
    double                  m_totalLength;
    Int32                   m_lsUnits;
    WString                 m_shapeFontName;
    WString                 m_shapeFontPath;
    bool                    m_countOnly;
    NameRecordP             m_nameRecord;

    StatusInt       ExportDecomp (struct dwgLineStyleInfo*, char* pStrings, Int32* stringIndex, double length, int type, SymbolRefP, DgnModelP) const;
    StatusInt       ExportSymbol (struct dwgLineStyleInfo*, char* pStrings, Int32& stringIndex, SymbolRefP, double xOffset, DgnModelP) const;

public:

    DecomposedDwgLine ();
    ~DecomposedDwgLine ();

    void        Dump () const;
    void        InsertDecomp (DecompLineType  *curLTP);
    void        UpdateLength (double length);
    void        MergeLineData ();
    void        SetFontName (WCharCP suggestedFontPathName);
    void        SetUnits (NameRecordP nameRec);
    double      GetLength () const {return m_totalLength;}
    void        SetCountOnly (bool value) {m_countOnly = value;}
    void        SetNameRecord (NameRecordP nameRec) {m_nameRecord = nameRec;}
    NameRecordP GetNameRecord () const {return m_nameRecord;}

    StatusInt   ToDwg (struct dwgLineStyleInfo* styleInfo, Int32* pNSegments, char* pStrings, Int32* pUnitMode, DgnModelP modelRef) const;
    };
END_LINESTYLE_NAMESPACE
END_BENTLEY_DGNPLATFORM_NAMESPACE


static StatusInt   extractCompoundComponent
(
DwgLineStyleInfo    *styleInfo,
Int32               *pNSegments,
char                *pStrings,
Int32               *pUnitMode,
CompoundComponentP  compoundComponent,
bool                useUnits,
WCharCP             suggestedFontPathName,
DgnModelP        modelRef,
NameRecordP         nameRec
);

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    ChuckKirschman                  08/06
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    lstyle_validNumDwgSegments (Int32 numSegments)
    {
    return (numSegments > 1 && numSegments <= MAX_LineTypeSegments);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    ChuckKirschman                  10/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void      lstyleElm_initDwgInfo
(
DwgLineStyleInfo    *pStyleInfo
)
    {
    pStyleInfo->length = 0.0;
    pStyleInfo->scale = 1.0;
    pStyleInfo->xOffset = 0.0;
    pStyleInfo->yOffset = 0.0;
    pStyleInfo->rotAng = 0.0;
    pStyleInfo->complexShapeCode = 0;
    pStyleInfo->shapeFlag = 0;
    pStyleInfo->adjustRotation = false;
    pStyleInfo->strOffset = 0;
    pStyleInfo->fontNo = -1;
    pStyleInfo->textStyleId = 0;
    memset (pStyleInfo->fontName, 0, sizeof(pStyleInfo)->fontName);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  05/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void createDecomp
(
DecompLineType      **ppLineList,
double              start,
double              length,
int                 type,
SymbolRefP          symbolRef
)
    {
    DecompLineType  *newLTP = NULL;

    newLTP = (DecompLineType *) memutil_calloc (1, sizeof *newLTP, HEAPSIG_LineStyleExport);

    newLTP->start       = start;
    newLTP->length      = length;
    newLTP->type        = type;
    newLTP->symbolRef   = symbolRef;

    *ppLineList = newLTP;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  08/02
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt      lstyleElm_getDescription
(
WCharP              pDescription,
LsLocation const*   location,
DgnModelP        modelRef
)
    {
    StatusInt   status = SUCCESS;

    pDescription[0] = '\0';

    /* Save the description */
    if (location->IsElement())
        {
        MSElementDescrP edP = location->GetElementDescr (modelRef);

        if (NULL != edP)
            {
            if (CELL_HEADER_ELM == edP->el.GetLegacyType())
                {
                status = mdlCell_extractDescription (pDescription, MAX_CELLDSCR_LENGTH, &edP->el);
                }
            else
                {
                if (SUCCESS == (status = mdlLinkage_extractNamedStringLinkageByIndex (pDescription, MAX_CELLDSCR_LENGTH, &edP->el, STRING_LINKAGE_KEY_Description, 0)) &&
                    0 != pDescription[0])
                    {
                    strutil_wstrpwspc (pDescription);
                    }
                }

            }
        mdlElmdscr_freeAll (&edP);
        }
    else
        {
        void    *pRsc = NULL;
        char    sDescription[MAX_CELLDSCR_LENGTH];

        pRsc = location->GetResource (NULL, true);

        if (NULL == pRsc)
            return (ERROR);

        switch (location->GetRscType())
            {
            case LsElementType::LineCode:
                strncpy (sDescription, ((LineCodeRsc *)pRsc)->descr, LS_MAX_DESCR);
                break;

            case LsElementType::LinePoint:
                strncpy (sDescription, ((LinePointRsc *)pRsc)->descr, LS_MAX_DESCR);
                break;

            case LsElementType::Compound:
                strncpy (sDescription, ((LineStyleRsc *)pRsc)->descr, LS_MAX_DESCR);
                break;

            case LsElementType::PointSymbol:
            case LS_ELEMENT_POINTSYMBOLV7:
                strncpy (sDescription, ((PointSymRsc *)pRsc)->header.descr, LS_MAX_DESCR);
                break;

            case LsElementType::Internal:
                sprintf (sDescription, "Line Code %d", location->GetIdentKey());   /* Comment in line style file; doesn't need translation */
                break;
            }

        sDescription[LS_MAX_DESCR-1] = '\0';
        mdlResource_free (pRsc);
        
        wcscpy (pDescription, WString(sDescription).c_str());
        }
    return (status);
    }
#endif

/*----------------------------------------------------------------------------------*//**
* No strokes with uneven widths
* @bsimethod                                                    ChuckKirschman  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     StrokePattern::ExportableToDwg (bool checkSegmentCount, bool allowNonShxStyles) const
    {
    // Cannot exceed the number of strokes for DWG
    if (GetStrokeCount() > MAX_LineTypeSegments)
        return false;

    // Cannot handle a fixed number of iterations
    if (GetIterationMode())
        return false;

    LsStrokeP   stroke = GetConstStrokePtr(0);
    double      width = 0.0;
    bool        widthFound = false;
    bool        zeroWidthFound = false;

    for (int i=0; i<GetStrokeCount(); i++, stroke++)
        {
        if (!stroke->IsDash())
            continue;

        // Can only have FULL widths
        if (stroke->GetWidthMode () != LCWIDTH_NONE && stroke->GetWidthMode () != LCWIDTH_FULL)
            return false;

        // Open dash caps draw as parallel lines.
        if (stroke->GetCapMode() == LCCAP_OPEN)
            return false;

        // Look for widths changing dash to dash.
        if (stroke->_HasUniformFullWidth())
            {
            if (widthFound && stroke->GetStartWidth() != width)
                return false;

            if (zeroWidthFound)
                return false;

            widthFound = true;
            width = stroke->GetStartWidth();

            }
        else if (!stroke->HasWidth() && widthFound)
            {
            zeroWidthFound = true;

            if (widthFound)
                return false;
            }

        // Don't allow tapers
        if (stroke->GetStartWidth() != stroke->GetEndWidth())
            return  false;
        }

    return  true;
    }

/*----------------------------------------------------------------------------------*//**
* Acad requires breaks to put symbols and a known, fixed length
* @bsimethod                                                    ChuckKirschman  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     InternalComponent::ExportableToDwg (bool checkSegmentCount, bool allowNonShxStyles) const
    {
    if (IsHardwareStyle() && GetHardwareStyle() > 0)
        return false;

    return  true;
    }

/*----------------------------------------------------------------------------------*//**
* No end symbols
* @bsimethod                                                    ChuckKirschman  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool         PointComponent::ExportableToDwg (bool checkSegmentCount, bool allowNonShxStyles) const
    {
    if (!allowNonShxStyles)
        return  false;

    SymbolRefP  symbols = GetSymbols();
    size_t      numSymbols = GetNumSymbols();

    for (int iSymb=0; iSymb<numSymbols; iSymb++)
        {
        if (symbols[iSymb].GetMod1() & (LCPOINT_LINEORG | LCPOINT_LINEEND | LCPOINT_LINEVERT))
            return  false;
        }

    return true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool         CompoundComponent::ExportableToDwg (bool checkSegmentCount, bool allowNonShxStyles) const
    {
    // A compound with no components is always bad
    if (GetNumComponents() == 0)
        return false;

    // This was the first behavior; essentially limits to stuff imported from a lin or dwg file
    if (!allowNonShxStyles)
        {
        /* Currently can maybe handle certain complex, if they came from DWG */
        if (GetNumComponents() != 2)
            return  false;

        int          compType[2];
        compType[0] = GetComponent(0)->GetResourceType();
        compType[1] = GetComponent(1)->GetResourceType();

        int          pointIndex = (LsElementType::LineCode == compType[0] ? 1 : 0);

        if (LsElementType::LineCode == compType[pointIndex?0:1] && LsElementType::LinePoint == compType[pointIndex])
            {
            PointComponentP  point = dynamic_cast <PointComponentP> (GetComponent(pointIndex));
            if (NULL == point)
                return  false;

            for (size_t i=0; i<point->GetNumSymbols(); i++)
                {
                SymbolRefP symbol = point->GetSymbol(i);
                if (NULL == symbol)
                    return  false;

                MSElementDescrP    symbolEdP = symbol->GetElementDescr();
                if (NULL == symbolEdP || symbolEdP->el.GetLegacyType() != TEXT_ELM)
                    return false;

                /* Check to make sure it's already SHX or TrueType */
                if (IS_RSC_FONTNUMBER (symbolEdP->el.text_2d.font))
                    return  false;
                }
            }
        return  true;
        }

    // More lax rules.  No offsets, no dash widths, no end symbols.
    for (int compNum = 0; compNum < GetNumComponents(); compNum++)
        {
        // No offsets
        if (GetOffset (compNum) != 0.0)
            return  false;

        if (!GetComponent (compNum)->ExportableToDwg (false, allowNonShxStyles))
            return  false;
        }

    // Also decompose and check the segment count.  Don't want to do this on recursion, or when called from decomposition.
    if (checkSegmentCount)
        {
        StatusInt   status;
        int         numSegments = 0;
        char        pStrings[256];

        status = extractCompoundComponent (NULL, &numSegments, pStrings, NULL, this, false, "", NULL, NULL);
        if (SUCCESS != status || !lstyle_validNumDwgSegments(numSegments))
            return false;
        }

    return  true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman     01/09
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     LsLineCodeComponent::ExportableToDwg (bool checkSegmentCount, LineStyleDwgExportReason& reason) const
    {
    // Cannot exceed the number of strokes for DWG
    if (GetNumStrokes() > MAX_LineTypeSegments)
        {
        reason = LS_DWG_TooManySegments;
        return false;
        }

    // Cannot handle a fixed number of iterations
    if (HasIterationLimit())
        {
        reason = LS_DWG_IterationLimit;
        return false;
        }

    double      width = 0.0;
    bool        widthFound = false;
    bool        zeroWidthFound = false;

    for (size_t iStroke=0; iStroke<GetNumStrokes(); iStroke++)
        {
        LsStrokeData const * stroke = GetStroke(iStroke);

        if (!stroke->IsDash())
            continue;

        // Can only have FULL widths
        if (stroke->IsHalfWidth())
            {
            reason = LS_DWG_PartialWidthDash;
            return false;
            }

        // Open dash caps draw as parallel lines.
        if (stroke->GetCapMode() == LsCapOpen)
            {
            reason = LS_DWG_OpenDashCap;
            return false;
            }

        // Look for widths changing dash to dash.
        if (stroke->_HasUniformFullWidth())
            {
            if (widthFound && stroke->GetStartWidth() != width)
                {
                reason = LS_DWG_WidthsChange;
                return false;
                }

            if (zeroWidthFound)
                {
                reason = LS_DWG_WidthsChange;
                return false;
                }

            widthFound = true;
            width = stroke->GetStartWidth();
            }
        else if (!stroke->HasWidth() && widthFound)
            {
            zeroWidthFound = true;

            if (widthFound)
                {
                reason = LS_DWG_WidthsChange;
                return false;
                }
            }

        // Don't allow tapers
        if (stroke->GetStartWidth() != stroke->GetEndWidth())
            {
            reason = LS_DWG_WidthTapers;
            return false;
            }
        }

    reason = LS_DWG_Ok;
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ChuckKirschman     02/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool    LsInternalComponent::ExportableToDwg (bool checkSegmentCount, LineStyleDwgExportReason& reason) const
    {
    return true;  // We'll do something here.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ChuckKirschman     01/09
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool         LsLinePointComponent::ExportableToDwg (bool checkSegmentCount, LineStyleDwgExportReason& reason) const
    {
    if (!mdlCapability_isEnabled (CAPABILITY_ALLOW_NON_SHX_STYLES))
        {
        reason = LS_DWG_ShxOnlyCompatSet;
        return  false;
        }

    size_t numSymbols = GetNumSymbols();

    for (int iSymb=0; iSymb<numSymbols; iSymb++)
        {
        // No end symbols
        switch (GetSymbolInfo(iSymb)->GetStrokeLocation ())
            {
            case SymbolLineOrigin:
            case SymbolLineEnd:
            case SymbolLineVertex:
                reason = LS_DWG_SymbolAtStartOrEnd;
                return false;
                break;

            default:
                break;
            }
        }

    reason = LS_DWG_Ok;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ChuckKirschman     01/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LsCompoundComponent::ExportableToDwg (bool checkSegmentCount, LineStyleDwgExportReason& reason) const
    {
    // A compound with no components is always bad
    if (GetNumComponents() == 0)
        {
        reason = LS_DWG_NoComponents;
        return false;
        }

    // This was the first behavior; essentially limits to stuff imported from a lin or dwg file
    if (!mdlCapability_isEnabled (CAPABILITY_ALLOW_NON_SHX_STYLES))
        {
        /* Currently can maybe handle certain complex, if they came from DWG */
        reason = LS_DWG_ShxOnlyCompatSet;

        if (GetNumComponents() != 2)
            return  false;

        int          compType[2];
        compType[0] = GetComponentType(0);
        compType[1] = GetComponentType(1);

        int          pointIndex = (LS_ELEM_LINECODE == compType[0] ? 1 : 0);

        if (LS_ELEM_LINECODE == compType[pointIndex?0:1] && LS_ELEM_LINEPOINT == compType[pointIndex])
            {
            LsLinePointComponent*  point = dynamic_cast <LsLinePointComponent*> (GetComponent(pointIndex));
            if (NULL == point)
                return  false;

            for (size_t i=0; i<point->GetNumSymbols(); i++)
                {
                LsPointSymbolComponent const * symbol = point->GetSymbolInfo(i)->GetSymbol();
                if (NULL == symbol)
                    return  false;

                MSElementDescrCP   symbolEdP = symbol->GetElements();
                if (NULL == symbolEdP || symbolEdP->el.GetLegacyType() != TEXT_ELM)
                    return false;

                /* Check to make sure it's already SHX or TrueType */
                if (IS_RSC_FONTNUMBER (symbolEdP->el.text_2d.font))
                    return  false;
                }
            }
        reason = LS_DWG_Ok;
        return  true;
        }

    // More lax rules.  No offsets, no dash widths, no end symbols.
    for (UInt32 compNum = 0; compNum < GetNumComponents(); compNum++)
        {
        // No offsets
        if (GetComponentOffset(compNum) != 0.0)
            {
            reason = LS_DWG_CompoundHasOffsets;
            return  false;
            }

        if (NULL != GetComponent (compNum) && !GetComponent (compNum)->ExportableToDwg (false, reason))
            return  false;
        }

    // Also decompose and check the segment count.  Don't want to do this on recursion, or when called from decomposition.
    if (checkSegmentCount && HasComponentsFilled())
        {
        StatusInt   status;
        int         numSegments = 0;
        char        pStrings[256];

        status = ExtractCompoundComponent(NULL, &numSegments, pStrings, NULL, false, "", NULL, NULL);
        if (SUCCESS != status || !lstyle_validNumDwgSegments(numSegments))
            {
            reason = LS_DWG_TooManySegments;
            return false;
            }
        }

    reason = LS_DWG_Ok;
    return  true;
    }

#ifdef BEIJING_DGNPLATFORM_WIP_LINESTYLES
/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DecomposedDwgLine::Dump () const
    {
    DecompLineType  *ltP = NULL;
    int             i;

    printf("\nDumping:\n");
    for (ltP=m_pLineList, i=0; NULL != ltP; ltP=ltP->next, i++)
        {
        printf ("line type %2d [%d]: start %12.6f  length %12.6f  end %12.6f\n", i, ltP->type, ltP->start, ltP->length, ltP->start+ltP->length);
        }

    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DecomposedDwgLine::UpdateLength
(
double     length
)
    {
    if (length == fc_hugeVal)  // Internal line; don't want to create a pattern this long.
        return;

    if (length > m_totalLength)
        m_totalLength = length;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DecomposedDwgLine::InsertDecomp
(
DecompLineType*     curLTP
)
    {
    DecompLineType  *ltP = NULL;

    if (NULL == m_pLineList)
        {
        m_pLineList = curLTP;
        }
    else /* Insertion sort by start, type */
        {
        /* Find the last item before this current one. */
        ltP=m_pLineList;
        while (ltP->next && ltP->next->start < curLTP->start)
            ltP = ltP->next;

        if (LT_DASH == curLTP->type)
            {
            /* Inset after current dash; ltP->start <= curLTP->start */
            curLTP->next        = ltP->next;
            curLTP->prev        = ltP;
            ltP->next           = curLTP;
            if (curLTP->next)
                curLTP->next->prev  = curLTP;
            }
        else if (LT_SYMB == curLTP->type)
            {

            /* Before beginning? Have to insert it before dash. */
            if (curLTP->start <= ltP->start)
                {
                curLTP->next    = ltP;
                curLTP->prev    = ltP->prev;
                ltP->prev       = curLTP;
                if (curLTP->prev)
                    curLTP->prev->next  = curLTP;

                if (m_pLineList == ltP)
                    m_pLineList = curLTP;
                }
            /* After end?  Insert after dash */
            else if (curLTP->start >= ltP->start+ltP->length)
                {
                curLTP->next        = ltP->next;
                curLTP->prev        = ltP;
                ltP->next           = curLTP;
                if (curLTP->next)
                    curLTP->next->prev  = curLTP;
                }
            else  /* Need to split */
                {
                DecompLineType  *newLTP = NULL, *nextLTP = ltP->next;

                createDecomp (&newLTP, ltP->start, curLTP->start-ltP->start, ltP->type, NULL);
                ltP->start      = curLTP->start;
                ltP->length    -= newLTP->length;

                newLTP->prev    = ltP->prev;
                newLTP->next    = curLTP;
                curLTP->prev    = newLTP;
                curLTP->next    = ltP;

                if (m_pLineList == ltP)
                    m_pLineList = newLTP;
                else if (ltP->prev)
                    ltP->prev->next = newLTP;

                ltP->prev       = curLTP;
                }
            }
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  05/01
+---------------+---------------+---------------+---------------+---------------+------*/
DecomposedDwgLine::DecomposedDwgLine
(
)
    {
    m_pLineList=NULL;
    m_totalLength=0;
    m_lsUnits = LSATTR_UNITMASTER;
    m_shapeFontName.assign (DEFAULT_FONT_NAME);
    m_shapeFontPath[0] = '\0';
    m_countOnly = false;
    m_nameRecord = NULL;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  05/01
+---------------+---------------+---------------+---------------+---------------+------*/
DecomposedDwgLine::~DecomposedDwgLine()
    {
    DecompLineType      *ltP = NULL, *tmpLTP=NULL;

    for (ltP = m_pLineList; NULL != ltP; ltP = tmpLTP)
        {
        tmpLTP = ltP->next;
        memutil_free (ltP);
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DecomposedDwgLine::SetFontName (WCharCP suggestedFontPathName)
    {
    if (SUCCESS != ConfigurationManager::GetVariable (m_shapeFontName, "MS_DWG_LSTYLE_FONTNAME", MAXFILELENGTH))
        m_shapeFontName.assign (DEFAULT_FONT_NAME);

    if (SUCCESS != ConfigurationManager::GetVariable (m_shapeFontPath, "MS_DWG_LSTYLE_FONTPATH", MAXFILELENGTH))
        {
        if (NULL != suggestedFontPathName)
            m_shapeFontPath.assign (suggestedFontPathName);
        }

    }

/*----------------------------------------------------------------------------------*//**
* Do this after all the lines have been added to the decomposed line, before point symbols
* @bsimethod                                                    ChuckKirschman  05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DecomposedDwgLine::MergeLineData
(
)
    {
    DecompLineType      *ltP = NULL, *tmpLTP=NULL;
    double              end, end2;

    /* Go through each piece.  Merge up lines where possible.*/
    for (ltP = m_pLineList; NULL != ltP && NULL != ltP->next; )
        {
        end = ltP->start + ltP->length;
        if (end >= ltP->next->start)
            {
            end2 = ltP->next->start + ltP->next->length;
            ltP->length = (end > end2 ? end : end2) - ltP->start;
            tmpLTP = ltP->next;
            ltP->next = tmpLTP->next;
            if (NULL != tmpLTP->next)
                tmpLTP->next->prev = ltP;
            memutil_free (tmpLTP);
            }
        else
            {
            ltP = ltP->next;
            }
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  05/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   DecomposedDwgLine::ExportSymbol
(
struct dwgLineStyleInfo*    styleInfoP,
char*                       pStrings,
Int32&                      stringIndex,
SymbolRefP                  symbolInfo,
double                      xOffset,
DgnModelP                modelRef
) const
    {
    StatusInt           status = ERROR;
    MSElementDescr      *pSymbolEdP;
    TextSizeParam       textSizeParam;
    TextParamWide       textParam;
    WString          wBuffer;

    if (NULL == symbolInfo)
        return  ERROR;

    SymbolComponentP    symbol = symbolInfo->GetSymbolComponent();
    BeAssert (NULL != symbol);

    pSymbolEdP = symbol->GetElementDescr();

    styleInfoP->scale       = symbol->GetMuDef();
    styleInfoP->xOffset     = symbolInfo->GetXOffset() - xOffset;
    styleInfoP->yOffset     = symbolInfo->GetYOffset();
    styleInfoP->rotAng      = symbolInfo->GetAngle();
    styleInfoP->adjustRotation = symbolInfo->GetMod1() & LCPOINT_ADJROT ? true : false;

    DgnFontR font = DgnFontManager::ResolveFontNum (pSymbolEdP->el.text_2d.font, modelRef);

    if (TEXT_ELM == pSymbolEdP->el.GetLegacyType() && NULL == pSymbolEdP->h.next)
        {
        DPoint3d        origin;
        bool            oLine=false;
        bool            uLine=false;

        memset (&textParam, 0, sizeof(textParam));
        memset (&textSizeParam, 0, sizeof(textSizeParam));
        textSizeParam.mode = TXT_BY_TILE_SIZE;
        mdlText_directExtractWide (&wBuffer, &origin, NULL, NULL, &textSizeParam, &textParam, NULL, &pSymbolEdP->el, NULL);

        // Over and under lines need special treatment; have to add %%o and %%u to string
        uLine = textParam.flags.underline;
        oLine = textParam.exFlags.overline;

        styleInfoP->xOffset     += origin.x / symbolInfo->GetSymbolComponent()->GetMuDef();
        styleInfoP->yOffset     += origin.y / symbolInfo->GetSymbolComponent()->GetMuDef();

        // Get the text style or font.  If the text element has overrides treat it like no style so one gets created (TR #300202).
        bool hasTextStyleOverrides = (0 != textParam.flags.textStyle && textParam.overridesFromStyle.AreAnyFlagsSet());
        if (0 != textParam.flags.textStyle && !hasTextStyleOverrides)
            {
            LegacyTextStyle    *pStyle = NULL;
            WChar         styleName[MAX_LINKAGE_STRING_LENGTH];

            mdlTextStyle_getById (&pStyle, styleName, _countof(styleName), textParam.textStyleId, modelRef);
            mdlTextStyle_free (pStyle);

            mdlCnv_convertUnicodeToMultibyte (styleName, -1, styleInfoP->fontName, sizeof(styleInfoP)->fontName);
            }
        else
            {
            font.GetName().ConvertToLocaleChars (styleInfoP->fontName);
            }

        if (0 != textSizeParam.size.height)  // Can be zero if it is just a space (which is often used with overline/underline)
            styleInfoP->scale         /= textSizeParam.size.height;

        styleInfoP->textStyleId    = (hasTextStyleOverrides ? 0 : textParam.textStyleId);
        styleInfoP->fontNo         = pSymbolEdP->el.text_2d.font;

        if (DgnFontType::Shx == font.GetType() && font.IsSymbolFont() && font.IsValid())
            {
            styleInfoP->complexShapeCode = wBuffer[0];
            styleInfoP->shapeFlag = SHAPEFLAG_ShapeSymbol;
            styleInfoP->strOffset = 0;
            }
        else
            {
            char   buffer[MAX_TEXT_LENGTH];
            styleInfoP->complexShapeCode = 0;
            styleInfoP->shapeFlag = SHAPEFLAG_TextSymbol;
            styleInfoP->strOffset = stringIndex;

            // WCHAR_T_CHANGE_VENKAT: Please recheck
            mdlCnv_convertUnicodeToMultibyte (wBuffer.c_str (), -1, buffer, MAX_TEXT_LENGTH);
            
            // Add in start of over / under line
            if (uLine)
                {
                strcpy (pStrings+stringIndex, "%%u");
                stringIndex += 3;
                }
            if (oLine)
                {
                strcpy (pStrings+stringIndex, "%%o");
                stringIndex += 3;
                }
            // Add in the string
            strcpy (pStrings+stringIndex, buffer);
            stringIndex += strlen(buffer);
            // Add in end of over / under line
            if (oLine)
                {
                strcpy (pStrings+stringIndex, "%%o");
                stringIndex += 3;
                }
            if (uLine)
                {
                strcpy (pStrings+stringIndex, "%%u");
                stringIndex += 3;
                }
            // Trailing null terminator.
            stringIndex += 1;

            /* DWG uses LB justification, so have to offset to compensate */
            if (TextElementJustification::LeftBaseline != textParam.just && (styleInfoP->shapeFlag == SHAPEFLAG_ShapeSymbol))
                {
                DPoint3d    offset;
                TextLen     textSize;

                /* Need the text by text size for justification changes. */
                memset (&textSizeParam, 0, sizeof(textSizeParam));
                textSizeParam.mode = TXT_BY_TEXT_SIZE;
                mdlText_directExtractWide (&wBuffer, NULL, NULL, NULL, &textSizeParam, NULL, NULL, &pSymbolEdP->el, NULL);

                textSize.h = textSizeParam.size.height;
                textSize.w = textSizeParam.size.width;

                extract_getTxNodeOffset (&offset, &textSize, 0.0, textParam.just, false, pSymbolEdP->el.GetLegacyType(), &font);

                if (LSATTR_UNITMASTER == m_lsUnits)
                    {
                    offset.x /= symbol->GetMuDef();
                    offset.y /= symbol->GetMuDef();
                    }
                styleInfoP->xOffset     -= offset.x;
                styleInfoP->yOffset     -= offset.y;
                }

            }
        }
    else  /* Not text; create in file */
        {
        UShort          shapeNumber = -1;
        char            symbolName[MAX_CELLNAME_BYTES], shapeName[MAX_CELLNAME_BYTES+PREFIX_LENGTH];
        char            qualifiedFontName[MAXFILELENGTH];
        char            *pName = NULL;
        char            fileName[MAXFILELENGTH];

        /* Extract the name */
        symbolName[0] = fileName[0] = '\0';
        memset (shapeName, 0, sizeof(shapeName));
        strcpy (qualifiedFontName, m_shapeFontName);

        if (symbol->GetLocation()->IsElement())
            {
            MSElementDescrP     edP = symbol->GetLocation()->GetElementDescr (modelRef);

            if (NULL != edP)
                {
                // Use the description as the symbol name.  This is correct when coming from RSC files;
                // from LIN files it should be handled in the text case above.
                PointSymRsc     *pRsc = NULL;
#ifdef BEIJING_DGNPLATFORM_WIP_LINESTYLES
                LineStyle_elementToRsc ((void **)&pRsc, NULL, edP, modelRef, false, false);
#endif

                if (NULL != pRsc)
                    strcpy (symbolName, pRsc->header.descr);

                memutil_free (pRsc);
                }

            ElementId           cellId = 0;
            MSElementDescrP     cellEdP = NULL;
            long                idArraySize = 1;

#if defined (BEIJING_DGNPLATFORM_WIP_LINESTYLES)       // *** PROBLEM: expandSharedCells==true!
            ElementId           *pIdArray;
            UShort              linkageKey;

            if (NULL != edP &&
                0 != (cellId = mdlLineStyleElement_getDependency (edP, 0)) &&
                SUCCESS == AssociativePoint::GetElementDescr (&cellEdP, NULL, cellId, modelRef, true) &&
                CELL_HEADER_ELM == cellEdP->el.GetLegacyType() &&
                SUCCESS == mdlLinkage_extractElementIDArrayLinkageByIndex (&linkageKey, &pIdArray, &idArraySize, &cellEdP->el, 0) &&
                ELEMENTID_LINKAGE_KEY_glyphId == linkageKey)
                {
                NameRecordP nameRec = GetNameRecord();
                BeAssert (nameRec != NULL);

                MSElementDescr  *pTable=NULL, *pEntry=NULL;

#ifdef BEIJING_DGNPLATFORM_WIP_LINESTYLES
                if (SUCCESS == lineStyle_findNameTableElement (&pTable, &pEntry, modelRef, nameRec->GetName()))
                    {
                    WChar     wFileName[MAXFILELENGTH];
                    if (SUCCESS == mdlLinkage_extractNamedStringLinkageByIndex (wFileName,
                        MAXFILELENGTH, &pEntry->el, STRING_LINKAGE_KEY_FileName, 0))
                        {
                        mdlCnv_convertUnicodeToMultibyte (wFileName, -1, fileName, sizeof(fileName));

                        strcat (qualifiedFontName, "-");                // Add resource file name as suffix.
                        strcat (qualifiedFontName, fileName);
                        shapeNumber = (UShort) pIdArray[0] + 1;          // Set "shapeNumber" so that the glyph number matches the ResourceId+1.

                        }

                    mdlElmdscr_freeAll (&pTable);
                    }
#endif

                dlmSystem_mdlFree (pIdArray);
                }
#endif

            strncpy (shapeName, fileName, PREFIX_LENGTH);
            strcat (shapeName, "_");
            if ('\0' != symbolName[0])
                {
                strcat (shapeName, symbolName);
                }
            else
                {
                char    num[20];
                sprintf (num, "%d", shapeNumber);
                strcat (shapeName, num);
                }
            pName = shapeName;

            mdlElmdscr_freeAll (&cellEdP);
            mdlElmdscr_freeAll (&edP);
            }
        else  // From resource
            {
            void    *pRsc = NULL;
            int     rscStatus;
            WString rscAlias;
            char    *rscFilenameP=NULL;

            pRsc = symbol->GetLocation()->GetResource (NULL);
            if (NULL != pRsc)
                {
                
                rscStatus = mdlResource_query (&rscAlias, pRsc, RSC_QRY_ALIAS);
                rscStatus = mdlResource_query (&rscFilenameP, pRsc, RSC_QRY_FILENAME);
                
                WString fileNameW = BeFileName::GetFileNameWithoutExtension (UglyUnicodeString (rscFilenameP).GetWCharCP ());
                fileNameW.ConvertToLocaleChars (fileName, _countof (fileName));

                strncpy (shapeName, fileName, PREFIX_LENGTH);
                if (!rscAlias.empty())
                    {
                    strcat (shapeName, "_");
                    strcat (shapeName, UglyAsciiString (rscAlias.c_str()).GetCharCP());
                    }
                else
                    {
                    UInt32  tmpRscId=0;
                    mdlResource_query (&tmpRscId, pRsc, RSC_QRY_ID);
                    char    rscIdString[128];
                    sprintf (rscIdString, "%d", tmpRscId);
                    strcat (shapeName, "_");
                    strcat (shapeName, rscIdString);
                    }
                pName = shapeName;

                // The original method for generating symbols within shape fonts would use a single SHX, "dgnlstyle" - The
                // shapes would then be generated sequentially.  This suffered from mismatched symbol IDs when dgnlstyle.shx
                // was shipped between sites.  In 8.5.1 this was modified (as below) to generate an shx file for each linestyle
                // resource with "dgnlstyle" as the prefix followed by the resource filename.  The shape numbers can then
                // be equated to the symbol Resource Ids which will always be unique for a given resource. This will create
                // more SHX files but should avoid the mismatch problem.
                strcat (qualifiedFontName, "-");                                    // Add resource file name as suffix.
                strcat (qualifiedFontName, fileName);
                shapeNumber = (UShort)symbolInfo->GetSymbolComponent()->GetLocation()->GetRscID();          // Set "shapeNumber" so that the glyph number matches the ResourceId.
                if (0 == shapeNumber)
                    shapeNumber = 255;

                mdlResource_free (pRsc);
                }
            }

        double scale;
        status = DgnRscFont::ExportCellToShapeFile (shapeNumber, scale, pSymbolEdP, pName, qualifiedFontName, m_shapeFontPath);

        if (SUCCESS == status || MDLERR_SHAPEALREADYEXISTS == status)
            {
            strncpy (styleInfoP->fontName, qualifiedFontName, sizeof (styleInfoP->fontName));

            styleInfoP->fontNo              = -1;
            styleInfoP->complexShapeCode    = shapeNumber;
            styleInfoP->shapeFlag           = SHAPEFLAG_ShapeSymbol;
            styleInfoP->strOffset           = 0;
            styleInfoP->scale              *= scale;
            }
        }

    // Special case for non-scaled symbols (S=0 in lin file)
    if (symbolInfo->GetSymbolComponent()->IsNotScaled())
        styleInfoP->scale = 0.0;

    return (SUCCESS);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void   DecomposedDwgLine::SetUnits (NameRecordP nameRec)
    {
    if (NULL == nameRec)
        m_lsUnits = 0;
    else
        m_lsUnits = nameRec->GetAttributes() & LSATTR_UNITMASK;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  05/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   DecomposedDwgLine::ExportDecomp
(
DwgLineStyleInfo*   styleInfoP,
char*               pStrings,
Int32*              stringIndex,
double              length,                  /* For symbols this is the x offset */
int                 type,
SymbolRefP          symbolInfo,
DgnModelP        modelRef
) const
    {
    if (m_countOnly)   // Quick out when we are just counting segments
        return SUCCESS;

    if (LT_SYMB == type) /* These get appended to the end of previous stroke */
        {
        BeAssert (NULL != stringIndex && NULL != pStrings);
        return  ExportSymbol (styleInfoP, pStrings, *stringIndex, symbolInfo, length, modelRef);
        }

    lstyleElm_initDwgInfo (styleInfoP);

    styleInfoP->length = length;
    if (LT_DASH != type)
        styleInfoP->length *= -1;

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  08/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt      DecomposedDwgLine::ToDwg
(
DwgLineStyleInfo*   styleInfo,
Int32              *pNSegments,
char*               pStrings,
Int32              *pUnitMode,
DgnModelP        modelRef
) const
    {
    DecompLineType      *ltP;
    double              start;
    int                 segNum;
    StatusInt           status = SUCCESS;
    Int32               stringIndex = 0;

    /* Everything is in the list to create the line style.  Now just go through and create the bits */
    start  = 0.0;
    segNum = 0;

    // Handle degenerate case of nothing but gap (TR #195140)
    if (NULL == m_pLineList)
        {
        ExportDecomp (styleInfo+segNum, NULL, NULL, m_totalLength, LT_GAP, NULL, modelRef);
        return SUCCESS;
        }

    /* Handle any leading gap */
    if (start < m_pLineList->start)
        {
        ExportDecomp (styleInfo+segNum, NULL, NULL, m_pLineList->start-start, LT_GAP, NULL, modelRef);
        start = m_pLineList->start;
        segNum++;
        }

    for (ltP = m_pLineList; NULL != ltP; ltP = ltP->next, segNum++)
        {
        if (LT_SYMB == ltP->type)
            {
            if (0 == segNum)  /* Have to add a starting gap */
                ExportDecomp (styleInfo+segNum, NULL, NULL, 0.0, LT_GAP, NULL, modelRef);
            else
                segNum--;               /* Appended to end of previous stroke */
            }

        if (segNum < MAX_LineTypeSegments)
            ExportDecomp (styleInfo+segNum, pStrings, &stringIndex, ltP->length, ltP->type, ltP->symbolRef, modelRef);

        start += ltP->length;

        /* Handle gaps */
        if (NULL != ltP->next && (ltP->next->start - start) > mgds_fc_epsilon)
            {
            segNum++;
            if (segNum < MAX_LineTypeSegments)
                ExportDecomp (styleInfo+segNum, NULL, NULL, ltP->next->start-start, LT_GAP, NULL, modelRef);
            start = ltP->next->start;
            }
        }
    /* Handle any trailing gap */
    if (start < m_totalLength)
        {
        if (segNum < MAX_LineTypeSegments)
            ExportDecomp (styleInfo+segNum, NULL, NULL, m_totalLength-start, LT_GAP, NULL, modelRef);
        segNum++;
        }

    if (NULL != styleInfo)
        {
        // if there is one segment, and there is a symbol, then we need to add a trailing gap.  Acad won't take single segments.
        if (1 == segNum && 0 != styleInfo[0].shapeFlag)
            {
            ExportDecomp (styleInfo+segNum, NULL, NULL, 0.0, LT_GAP, NULL, modelRef);
            segNum++;
            }
        }

    *pNSegments = segNum;

    if (NULL != pUnitMode)
        *pUnitMode = m_lsUnits;

    return (status);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  05/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PointComponent::DecomposeForDwgPoint
(
DecomposedDwgLineP pDDLine
) const
    {
    size_t              curPointNum = 0;
    double              start = 0;
    int                 segNum =0;
    StrokePatternP      strokeComp = GetStrokeComponent();

    if (NULL == strokeComp)
        return  ERROR;

    LsStrokeP   strokes = strokeComp->GetConstStrokePtr (0);
    for (int strokeNum=0; strokeNum < strokeComp->GetStrokeCount(); strokeNum++, segNum++)
        {
        for (curPointNum = 0; curPointNum < GetNumSymbols(); curPointNum++)
            {
            SymbolRefP   symbolInfo = GetSymbol(curPointNum);
            if (symbolInfo->GetStrokeNumber() == strokeNum && NULL != symbolInfo->GetSymbolComponent())
                {
                double  pointStart;
                switch  (symbolInfo->GetMod1() & LCPOINT_ONSTROKE)
                    {
                    case LCPOINT_NONE:
                        continue;

                    case LCPOINT_ORIGIN:
                        pointStart = start;
                        break;

                    case LCPOINT_END:
                        pointStart = start + strokes[strokeNum].GetLength();
                        break;

                    case LCPOINT_CENTER:
                        pointStart = start + (strokes[strokeNum].GetLength()) / 2;
                        break;
                    }

                DecompLineType*  tmpLTP;
                createDecomp (&tmpLTP, pointStart, 0.0, LT_SYMB, GetSymbol(curPointNum));
                pDDLine->InsertDecomp (tmpLTP);
                }
            }

        start += strokes[strokeNum].GetLength();
        }

#if defined (DEBUG_LSDWGOUT)
    printf ("\npost-symbols\n");
    DDLine.Dump ();
#endif

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    Chuck.Kirschman                 02/03
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt StrokePattern::_DecomposeForDwg
(
DecomposedDwgLineP  pDDLine
) const
    {
    LsStrokeP           strokes = GetConstStrokePtr(0);
    DecompLineType      *tmpLTP = NULL;
    double              start = 0.0;
    double              totalLength = pDDLine->GetLength ();
    double              length;

    for (int strokeNum=0; strokeNum < GetStrokeCount(); strokeNum++)
        {
        if (strokes[strokeNum].IsDash())
            {
            length = strokes[strokeNum].GetLength();
            if (start+length > totalLength)
                length = totalLength - start;
            createDecomp (&tmpLTP, start, length, LT_DASH, NULL);
            pDDLine->InsertDecomp (tmpLTP);
            }

        start += strokes[strokeNum].GetLength();
        }

    return (SUCCESS);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    Chuck.Kirschman                 02/03
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt InternalComponent::_DecomposeForDwg
(
DecomposedDwgLineP  pDDLine
) const
    {
    DecompLineType      *tmpLTP = NULL;
    // For solid lines, create a dash of total length
    if (!IsHardwareStyle() || GetHardwareStyle() == 0)
        {
        createDecomp (&tmpLTP, 0, pDDLine->GetLength(), LT_DASH, NULL);
        pDDLine->InsertDecomp (tmpLTP);
        }

    return (SUCCESS);
    }

/*----------------------------------------------------------------------------------*//**
* This one is for line code pass; DecomposeForDwgPoint is for point symbol pass.
* @bsimethod                    Chuck.Kirschman                 02/03
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt PointComponent::_DecomposeForDwg
(
DecomposedDwgLineP  pDDLine
) const
    {
    return (SUCCESS);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    Chuck.Kirschman                 02/03
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt CompoundComponent::_DecomposeForDwg
(
DecomposedDwgLineP  pDDLine
) const
    {
    int         compIndex;
    StatusInt   status = SUCCESS;

    /* Run through each piece and get it in terms of its starting point and length */
    for (compIndex=0; compIndex<GetNumComponents(); compIndex++)
        {
        if (GetOffset(compIndex) == 0.0) /* Can't handle anything with an offset */
            {
            if (SUCCESS != (status = GetComponent(compIndex)->_DecomposeForDwg (pDDLine)))
                break;
            }
        }
    return status;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    Chuck.Kirschman                 02/03
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LsLineCodeComponent::_DecomposeForDwg (DecomposedDwgLineP  pDDLine) const
    {
    DecompLineType      *tmpLTP = NULL;
    double              start = 0.0;
    double              totalLength = pDDLine->GetLength ();

    for (size_t strokeNum=0; strokeNum < GetNumStrokes(); strokeNum++)
        {
        LsStrokeData const * pStroke = GetStroke (strokeNum);
        if (pStroke->IsDash())
            {
            double length = pStroke->GetLength();
            if (start+length > totalLength)
                length = totalLength - start;
            createDecomp (&tmpLTP, start, length, LT_DASH, NULL);
            pDDLine->InsertDecomp (tmpLTP);
            }

        start += pStroke->GetLength();
        }

    return (SUCCESS);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    Chuck.Kirschman                 02/03
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef TODO
// Needs to be worked into LsLineCodeComponent?
BentleyStatus InternalComponent::_DecomposeForDwg (DecomposedDwgLineP  pDDLine) const
    {
    DecompLineType      *tmpLTP = NULL;
    // For solid lines, create a dash of total length
    if (!IsHardwareStyle() || GetHardwareStyle() == 0)
        {
        createDecomp (&tmpLTP, 0, pDDLine->GetLength(), LT_DASH, NULL);
        pDDLine->InsertDecomp (tmpLTP);
        }

    return (SUCCESS);
    }
#endif

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  05/01
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LsLinePointComponent::DecomposeForDwgPoint (DecomposedDwgLineP pDDLine) const
    {
    size_t              curPointNum = 0;
    double              start = 0;
    int                 segNum =0;

    LsLineCodeComponent*      strokeComp = GetLineCodeComponent();

    if (NULL == strokeComp)
        return  ERROR;

    for (size_t strokeNum=0; strokeNum < strokeComp->GetNumStrokes(); strokeNum++, segNum++)
        {
        LsLineCodeComponent::LsStrokeData const * pStroke = strokeComp->GetStroke (strokeNum);

        for (curPointNum = 0; curPointNum < GetNumSymbols(); curPointNum++)
            {
            if (GetSymbolInfo(curPointNum)->GetStrokeNumber() == strokeNum && NULL != GetSymbolInfo(curPointNum)->GetSymbolComponent())
                {
                double  pointStart;
                switch  (GetSymbolInfo(curPointNum)->GetStrokeLocation ())
                    {
                    case SymbolStrokeOrigin:
                        pointStart = start;
                        break;

                    case SymbolStrokeEnd:
                        pointStart = start + pStroke->GetLength();
                        break;

                    case SymbolStrokeCenter:
                        pointStart = start + (pStroke->GetLength()) / 2;
                        break;

                    default:
                        continue;
                    }

                DecompLineType*  tmpLTP;
                createDecomp (&tmpLTP, pointStart, 0.0, LT_SYMB, GetSymbolInfo(curPointNum)->GetSymbolRef());
                pDDLine->InsertDecomp (tmpLTP);
                }
            }

        start += pStroke->GetLength();
        }

#if defined (DEBUG_LSDWGOUT)
    printf ("\npost-symbols\n");
    DDLine.Dump ();
#endif

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    Chuck.Kirschman                 02/03
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LsCompoundComponent::_DecomposeForDwg (DecomposedDwgLineP  pDDLine) const
    {
    BentleyStatus   status = SUCCESS;

    // Run through each piece and get it in terms of its starting point and length
    for (UInt32 compIndex=0; compIndex<GetNumComponents(); compIndex++)
        {
        if (GetComponentOffset(compIndex) == 0.0) // Can't handle anything with an offset
            {
            if (SUCCESS != (status = GetComponent(compIndex)->_DecomposeForDwg (pDDLine)))
                break;
            }
        }
    return status;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  05/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   LsCompoundComponent::ExtractCompoundComponent
(
DwgLineStyleInfo*   styleInfo,
Int32*              pNSegments,
char*               pStrings,
Int32*              pUnitMode,
bool                useUnits,
WCharCP             suggestedFontPathName,
DgnModelP        modelRef,
NameRecordP         nameRec
) const
    {
    StatusInt           status = ERROR;
    DecomposedDwgLine   DDLine;

#if defined (DEBUG_LSDWGOUT)
    printf ("\nLine style: %ls\n", pIdRec->pName);
    DDLine.Dump ();
#endif

    DDLine.SetFontName (suggestedFontPathName);
    DDLine.SetUnits (useUnits ? nameRec : NULL);
    DDLine.SetCountOnly (NULL == styleInfo);
    DDLine.SetNameRecord (nameRec);

    /* Run through each piece to get total length */
    for (UInt32 compIndex=0; compIndex<GetNumComponents(); compIndex++)
        {
        if (GetComponentOffset(compIndex) == 0) /* Can't handle anything with an offset */
            DDLine.UpdateLength (GetComponent(compIndex)->GetLength());
        }

    /* Run through each piece and get it in terms of its starting point and length */
    _DecomposeForDwg (&DDLine);

#if defined (DEBUG_LSDWGOUT)
    printf ("\npre-merge\n");
    DDLine.Dump ();
#endif

    /* Merge up the created lines */
    DDLine.MergeLineData();

#if defined (DEBUG_LSDWGOUT)
    printf ("\npost-merge\n");
    DDLine.Dump ();
#endif
    /* Add symbols to list. */
    for (UInt32 compIndex=0; compIndex<GetNumComponents(); compIndex++)
        {
        LsLinePointComponent* pointComp = dynamic_cast <LsLinePointComponent*> (GetComponent(compIndex));
        if (NULL != pointComp)
            status = pointComp->DecomposeForDwgPoint (&DDLine);
        }

    /* Everything is in the list to create the line style.  Now just go through and create the bits */
    status = DDLine.ToDwg (styleInfo, pNSegments, pStrings, pUnitMode, modelRef);

    // DDLine will clean up on exit
    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* Get the data from a line code
* @bsimethod                                                    ChuckKirschman  05/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   extractLineCodeEntry
(
DwgLineStyleInfo    *styleInfo,
Int32               *pNSegments,
Int32               *pUnitMode,
Int32               styleNo,
DgnModelP        modelRef
)
    {
    if (styleNo > MIN_LINECODE && styleNo <= MAX_LINECODE)
        {
        for (int i=0; i<6; i++)
            lstyleElm_initDwgInfo (styleInfo + i);

        switch  (styleNo)
            {
            case 1:
                styleInfo[0].length =  .001;
                styleInfo[1].length =  -.6;
                *pNSegments = 2;
                break;
            case 2:
                styleInfo[0].length =  .6;
                styleInfo[1].length = -.4;
                *pNSegments = 2;
                break;
            case 3:
                styleInfo[0].length =  1.0;
                styleInfo[1].length =  -.4;
                *pNSegments = 2;
                break;
            case 4:
                styleInfo[0].length =  .8;
                styleInfo[1].length = -.4;
                styleInfo[2].length =  .001;
                styleInfo[3].length =  -.4;
                *pNSegments = 4;
                break;
            case 5:
                styleInfo[0].length =  .3;
                styleInfo[1].length = -.4;
                *pNSegments = 2;
                break;
            case 6:
                styleInfo[0].length =   .001;
                styleInfo[1].length =  -.4;
                styleInfo[2].length =   .7;
                styleInfo[3].length =  -.4;
                styleInfo[4].length =   .001;
                styleInfo[5].length =  -.4;
                *pNSegments = 6;
                break;
            case 7:
                styleInfo[0].length =  0.9;
                styleInfo[1].length = -0.4;
                styleInfo[2].length =  0.3;
                styleInfo[3].length = -0.4;
                *pNSegments = 4;
                break;

            default:
                return ERROR;
            }
        if (NULL != pUnitMode)
            *pUnitMode = LSATTR_UNITDEV;

        return  SUCCESS;
        }

    /* ----- Handle normal line codes style ----- */

    if (STYLE_BYLEVEL == styleNo || STYLE_BYCELL == styleNo)
        return  ERROR;

    NameRecordP   nameRec = LsFileInfo::FindInRef (modelRef, styleNo);

    if (NULL == nameRec || !nameRec->IsOfType(LsElementType::LineCode))
        return  ERROR;

    DecomposedDwgLine   DDLine;
    StatusInt           status = ERROR;
    StrokePatternP      strokeComponent = dynamic_cast<StrokePatternP> (nameRec->GetComponent (modelRef));

    if (NULL == strokeComponent || !strokeComponent->ExportableToDwg(true))
        return ERROR;

    DDLine.SetUnits (nameRec);
    DDLine.UpdateLength (strokeComponent->GetLength());

    /* Decompose the strokes */
    strokeComponent->_DecomposeForDwg (&DDLine);

    /* Merge up the created lines */
    DDLine.MergeLineData();

    /* Everything is in the list to create the line style.  Now just go through and create the bits */
    status = DDLine.ToDwg (styleInfo, pNSegments, NULL, pUnitMode, modelRef);

    return status;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  08/02
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   extractLinePoint
(
DwgLineStyleInfo    *styleInfo,
Int32               *pNSegments,
char                *pStrings,
Int32               *pUnitMode,
Int32                styleNo,
WCharCP             suggestedFontPathName,
DgnModelP        modelRef,
NameRecordP         nameRec
)
    {
    PointComponentP     pointComponent;
    StatusInt           status = ERROR;
    DecomposedDwgLine   DDLine;

    if (NULL == nameRec)
        nameRec = LsFileInfo::FindInRef (modelRef, styleNo);

    /* Make sure it's a style we can work with */
    if (STYLE_BYLEVEL == styleNo || STYLE_BYCELL == styleNo ||
        NULL == nameRec ||
        !nameRec->IsOfType(LsElementType::LinePoint) ||
        NULL == (pointComponent = dynamic_cast <PointComponentP> (nameRec->GetComponent (modelRef))))
        {
        return  ERROR;
        }

    if (!pointComponent->ExportableToDwg (true))
        return  ERROR;

    DDLine.SetFontName (suggestedFontPathName);
    DDLine.SetUnits (nameRec);
    DDLine.SetNameRecord (nameRec);

    DDLine.UpdateLength (pointComponent->GetStrokeComponent()->GetLength());

    status = pointComponent->DecomposeForDwgPoint (&DDLine);

    /* Everything is in the list to create the line style.  Now just go through and create the bits */
    status = DDLine.ToDwg (styleInfo, pNSegments, pStrings, pUnitMode, modelRef);

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  05/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   extractCompoundComponent
(
DwgLineStyleInfo    *styleInfo,
Int32               *pNSegments,
char                *pStrings,
Int32               *pUnitMode,
CompoundComponentP  compoundComponent,
bool                useUnits,
WCharCP             suggestedFontPathName,
DgnModelP        modelRef,
NameRecordP         nameRec
)
    {
    StatusInt           status = ERROR;
    DecomposedDwgLine   DDLine;
    int                 compIndex;

#if defined (DEBUG_LSDWGOUT)
    printf ("\nLine style: %ls\n", pIdRec->pName);
    DDLine.Dump ();
#endif

    DDLine.SetFontName (suggestedFontPathName);
    DDLine.SetUnits (useUnits ? nameRec : NULL);
    DDLine.SetCountOnly (NULL == styleInfo);
    DDLine.SetNameRecord (nameRec);

    /* Run through each piece to get total length */
    for (compIndex=0; compIndex<compoundComponent->GetNumComponents(); compIndex++)
        {
        if (compoundComponent->GetOffset(compIndex) == 0) /* Can't handle anything with an offset */
            DDLine.UpdateLength (compoundComponent->GetComponent(compIndex)->GetLength());
        }

    /* Run through each piece and get it in terms of its starting point and length */
    compoundComponent->_DecomposeForDwg (&DDLine);

#if defined (DEBUG_LSDWGOUT)
    printf ("\npre-merge\n");
    DDLine.Dump ();
#endif

    /* Merge up the created lines */
    DDLine.MergeLineData();

#if defined (DEBUG_LSDWGOUT)
    printf ("\npost-merge\n");
    DDLine.Dump ();
#endif
    /* Add symbols to list. */
    for (compIndex=0; compIndex<compoundComponent->GetNumComponents(); compIndex++)
        {
        PointComponentP pointComp = dynamic_cast <PointComponentP> (compoundComponent->GetComponent(compIndex));
        if (NULL != pointComp)
            status = pointComp->DecomposeForDwgPoint (&DDLine);
        }

    /* Everything is in the list to create the line style.  Now just go through and create the bits */
    status = DDLine.ToDwg (styleInfo, pNSegments, pStrings, pUnitMode, modelRef);

    // DDLine will clean up on exit
    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  05/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   extractCompound
(
DwgLineStyleInfo    *styleInfo,
Int32               *pNSegments,
char                *pStrings,
Int32               *pUnitMode,
Int32                styleNo,
WCharCP             suggestedFontPathName,
DgnModelP        modelRef,
NameRecordP         nameRec
)
    {
    CompoundComponentP  compoundComponent;

    if (NULL == nameRec)
        nameRec = LsFileInfo::GetMap (modelRef, true)->Find (styleNo);

    /* Make sure it's a style we can work with */
    if (STYLE_BYLEVEL == styleNo || STYLE_BYCELL == styleNo ||
        NULL == nameRec ||
        !nameRec->IsOfType (LsElementType::Compound) ||
        NULL == (compoundComponent = dynamic_cast <CompoundComponentP> (nameRec->GetComponent (modelRef))))
        {
        return  ERROR;
        }

    if (!compoundComponent->ExportableToDwg (false))
        return  ERROR;

    return extractCompoundComponent(styleInfo, pNSegments, pStrings, pUnitMode, compoundComponent, true, suggestedFontPathName, modelRef, nameRec);
    }

/*----------------------------------------------------------------------------------*//**
* Get the data from a line code
* @bsimethod                                                    ChuckKirschman  05/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt  mdlLineStyle_extractLineStyleEntry
(
DwgLineStyleInfo    *styleInfo,
Int32               *pNSegments,
char                *pStrings,
Int32               *pUnitMode,
WCharP              pDescription,
Int32               styleNo,
WCharCP             suggestedFontPathName,
DgnModelP        modelRef
)
    {
    StatusInt       status = ERROR;
    NameRecordP     nameRec;

    *pNSegments= 0;

    if (styleNo > MIN_LINECODE && styleNo <= MAX_LINECODE)
        {
        status = extractLineCodeEntry (styleInfo, pNSegments, pUnitMode, styleNo, modelRef);
        sprintf (pDescription, "Dgn Style %d", styleNo);
        }
    else if (STYLE_BYLEVEL != styleNo && STYLE_BYCELL != styleNo &&
        NULL != (nameRec = LsFileInfo::FindInRef (modelRef, styleNo)))
        {
        /* Save descpiption */
        lstyleElm_getDescription (pDescription, nameRec->GetLocation(), modelRef);

        switch (nameRec->GetLocation()->GetRscType())
            {
            case LsElementType::LineCode:
                status = extractLineCodeEntry (styleInfo, pNSegments, pUnitMode, styleNo, modelRef);
                break;

            case LsElementType::LinePoint:
                {
                status = extractLinePoint (styleInfo, pNSegments, pStrings, pUnitMode, styleNo, suggestedFontPathName, modelRef,nameRec);
                break;
                }

            case LsElementType::Compound:
                {
                status = extractCompound (styleInfo, pNSegments, pStrings, pUnitMode, styleNo, suggestedFontPathName, modelRef, nameRec);
                break;
                }
            case LsElementType::PointSymbol:
            case LS_ELEMENT_POINTSYMBOLV7:
            case LsElementType::Internal:
                break;

            }
        }

    /* If, somehow, the resulting line code ends up with more than MAX_LineTypeSegments,
       it is one last place to fail.  The only way I can think for this to happen is if there
       are multiple sequential line codes in a compound specifically to get around the 32 segment
       limit. */
    if (!lstyle_validNumDwgSegments (*pNSegments))
        status = ERROR;

    return  status;
    }

#endif
