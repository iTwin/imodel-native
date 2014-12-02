/*----------------------------------------------------------------------+
|
|   $Source: DgnHandlers/Dimension/DimAPI.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*----------------------------------------------------------------------+
|                                                                       |
|   Local type definitions                                              |
|                                                                       |
+----------------------------------------------------------------------*/
typedef struct
    {
    byte    nWords;
    byte    type;
    } OptionBlock;

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlDim_getOrCreateOptionBlock                           |
|                                                                       |
| author        petri.niiranen                          10/00           |
|                                                                       |
+----------------------------------------------------------------------*/
DimOptionBlockHeader      *BentleyApi::mdlDim_getOrCreateOptionBlock
(
EditElementHandleR dimElement,      /* <=> */
int             type,       /* => one of ADBLK_ */
int             size,       /* => */
UInt64*      elementID
)
    {
    DimOptionBlockHeader *pOptionBlock = NULL;

    if (NULL == (pOptionBlock = (DimOptionBlockHeader *) mdlDim_getOptionBlock (dimElement, type, elementID)))
        {
        if (NULL != (pOptionBlock = (DimOptionBlockHeader*) calloc (1, size)))
            {
            ((OptionBlock *)pOptionBlock)->nWords     = static_cast <byte> (size / 2);
            ((OptionBlock *)pOptionBlock)->type       = (byte)type;
            UInt64 sharedCellTermId []= {0, 0, 0, 0};//In 8.11 we initialize shared cell Id to zero
            mdlDim_insertOptionBlock (dimElement, pOptionBlock, sharedCellTermId);

            free (pOptionBlock);
            }

        pOptionBlock = (DimOptionBlockHeader *)mdlDim_getEditOptionBlock (dimElement, type, elementID);
        }

    return  pOptionBlock;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void    setShield
(
bool                  setShieldOn,
DimStyleProp          property,
DimStylePropMaskR     shieldFlags
)
    {
    if (setShieldOn)
        shieldFlags.SetPropertyBit (property, true);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlDim_isPointAssociative                               |
|                                                                       |
| author        petri.niiranen                          02/01           |
|                                                                       |
+----------------------------------------------------------------------*/
int   BentleyApi::mdlDim_isPointAssociative
(
bool            *pIsAssociative,    /* <= */
ElementHandleCR dimElm,             /* => */
int             pointNo             /* => */
)
    {
    DgnElementCP pDimElementIn = dimElm.GetElementCP();
    DimensionElm    *pDim = NULL;

    if (NULL == pDimElementIn)
        return  DGNHANDLERS_STATUS_BadElement;

    if (DIMENSION_ELM != pDimElementIn->GetLegacyType())
        return  DGNHANDLERS_STATUS_BadElement;

    if (NULL == pIsAssociative)
        return  DGNHANDLERS_STATUS_BadArg;

    pDim = (DimensionElm *)pDimElementIn;
    if (pointNo < 0 || pointNo >= pDim->nPoints)
        return  DGNHANDLERS_STATUS_BadArg;

    *pIsAssociative = pDim->GetDimTextCP(pointNo)->flags.b.associative;

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool      BentleyApi::mdlDim_partTypeIsAnyDimLine
(
UInt32  partType
)
    {
    return (ADTYPE_DIMLINE == partType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool      BentleyApi::mdlDim_partTypeIsAnyExtension
(
UInt32  partType
)
    {
    return (ADTYPE_EXT_LEFT == partType ||
            ADTYPE_EXT_RIGHT == partType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool      BentleyApi::mdlDim_partTypeIsAnyText
(
UInt32  partType
)
    {
    return (ADTYPE_TEXT_UPPER == partType ||
            ADTYPE_TEXT_LOWER == partType ||
            ADTYPE_TEXT_SINGLE == partType ||
            ADTYPE_TEXT_SYMBOLS == partType ||
            ADTYPE_CHAIN == partType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool      BentleyApi::mdlDim_partTypeIsAnyTerminator
(
UInt32  partType
)
    {
    return (ADTYPE_TERM_LEFT == partType ||
            ADTYPE_TERM_RIGHT == partType);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlDim_createAndInsertOptionBlock                       |
|                                                                       |
| author        MartinWaardenburg                       10/98           |
|                                                                       |
+----------------------------------------------------------------------*/
void     BentleyApi::mdlDim_createAndInsertOptionBlock
(
EditElementHandleR dimElement,
RotMatrix       *rMatrixP
)
    {
    DimViewBlock    dimView;

    memset (&dimView, 0, sizeof (dimView));
    dimView.nWords = sizeof (dimView) / 2;
    dimView.type = ADBLK_VIEWROT;

    adim_packToFourDoubles (dimView.viewRot, rMatrixP, dimElement.GetElementCP()->Is3d());
    mdlDim_insertOptionBlock (dimElement, (DimOptionBlockHeader *)&dimView, NULL);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlDim_setAlignment                                     |
|                                                                       |
| author        KiranHebbar                             12/98           |
|                                                                       |
+----------------------------------------------------------------------*/
void                DimensionHandler::SetAlignment (EditElementHandleR eeh, DimensionAlignment alignment)
    {
    eeh.GetElementP()->ToDimensionElmR().flag.alignment = alignment;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionAlignment            DimensionHandler::GetAlignment (ElementHandleCR eh) const
    {
    return static_cast<DimensionAlignment>(eh.GetElementCP()->ToDimensionElm().flag.alignment);
    }

#define     DIMENSION_DimText               0

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlDim_getDimData                                       |
|                                                                       |
| author        AbheyLamba                              04/99           |
|                                                                       |
+----------------------------------------------------------------------*/
int     BentleyApi::mdlDim_getDimData
(
void        *dataP,
int         *dataSizeP,
ElementHandleCR eh,
int         dataType,
UInt32      dimArg
)
    {
    DgnElementCP elmP = eh.GetElementCP();
    int     status = ERROR;

    switch (dataType)
        {
        case DIMENSION_DimText:
            memcpy (dataP, ((DimensionElm *) elmP)->GetDimTextCP((int) dimArg), sizeof (DimText));
            *dataSizeP = sizeof (DimText);
            status = SUCCESS;
            break;
        }

    return  status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlDim_setDimData                                       |
|                                                                       |
| author        AbheyLamba                              04/99           |
|                                                                       |
+----------------------------------------------------------------------*/
int     BentleyApi::mdlDim_setDimData
(
void        *dataP,
int         *dataSizeP,
EditElementHandleR eh,
int         dataType,
UInt32      dimArg
)
    {
    int     status = ERROR;
    DgnElement   *elmP = eh.GetElementP ();

    switch (dataType)
        {
        case DIMENSION_DimText:
            memcpy (((DimensionElm *) elmP)->GetDimTextP((int) dimArg), dataP, sizeof (DimText));
            status = SUCCESS;
            break;
        }

    return  status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|       Local definition                                                |
|                                                                       |
+----------------------------------------------------------------------*/
#define LINKAGETYPE_Unit    1
#define LINKAGETYPE_Cell    2

/*----------------------------------------------------------------------+
|                                                                       |
| name          getLinkageKey                                           |
|                                                                       |
| author        petri.niiranen                          02/01           |
|                                                                       |
+----------------------------------------------------------------------*/
static UShort  getLinkageKey
(
int         linkageTypeIn,
int         flagIn
)
    {
    UShort  linkageKey = 0;

    if (LINKAGETYPE_Unit == linkageTypeIn)
        {
        switch (flagIn)
            {
            case DIMLABEL_MASTUNIT:
            case DIMLABEL_SUBUNIT:
            case DIMLABEL_SECONDARY_MASTUNIT:
            case DIMLABEL_SECONDARY_SUBUNIT:
                linkageKey = (UShort) flagIn;
                break;

            default:
                break;
            }
        }

    if (LINKAGETYPE_Cell == linkageTypeIn)
        {
        switch (flagIn)
            {
            case DIMCELL_ARROW:
            case DIMCELL_STROKE:
            case DIMCELL_DOT:
            case DIMCELL_ORIGIN:
            case DIMCELL_PREFIX:
            case DIMCELL_SUFFIX:
            case DIMCELL_NOTE:
                linkageKey = (UShort)flagIn;
                break;

            default:
                break;
            }
        }

    return  linkageKey;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          adim_setStringLinkageUsingDescr                         |
|                                                                       |
| author        petri.niiranen                          02/01           |
|                                                                       |
+----------------------------------------------------------------------*/
int      BentleyApi::adim_setStringLinkageUsingDescr
(
MSElementDescr  **ppDescrIn,
WChar         *pStringIn,
int             categoryIn,
int             linkageKeyIn,
int             maxStringSizeIn
)
    {
    int     status = ERROR;
    UShort  linkageKey;

    if (NULL != ppDescrIn &&
        NULL != *ppDescrIn &&
        NULL != pStringIn  &&
        (int) wcslen (pStringIn) <= maxStringSizeIn)
        {
        linkageKey = getLinkageKey (categoryIn, linkageKeyIn);
        if (linkageKey)
            status = LinkageUtil::SetStringLinkageUsingDescr (*ppDescrIn, linkageKey, pStringIn);
        }

    return  status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          adim_setStringLinkage                                   |
|                                                                       |
| author        petri.niiranen                          02/01           |
|                                                                       |
+----------------------------------------------------------------------*/
int      BentleyApi::adim_setStringLinkage
(
DgnElement      *pElementIn,
const WChar  *pStringIn,
int             categoryIn,
int             linkageKeyIn,
int             maxStringSizeIn
)
    {
    int     status = ERROR;
    UShort  linkageKey;

    if (NULL != pElementIn &&
        NULL != pStringIn  &&
        (int) wcslen (pStringIn) <= maxStringSizeIn)
        {
        linkageKey = getLinkageKey (categoryIn, linkageKeyIn);
        if (linkageKey)
            status = LinkageUtil::SetStringLinkage (pElementIn, linkageKey, pStringIn);
        }

    return  status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          adim_getStringLinkage                                   |
|                                                                       |
| author        petri.niiranen                          02/01           |
|                                                                       |
+----------------------------------------------------------------------*/
int      BentleyApi::adim_getStringLinkage
(
WCharP        pStringOut,
const DgnElement *pElementIn,
int             categoryIn,
int             linkageKeyIn,
int             maxStringSizeIn
)
    {
    int     status = ERROR;
    UShort  linkageKey = 0;

    if (NULL != pElementIn &&
        0    != (linkageKey = getLinkageKey (categoryIn, linkageKeyIn)))
        {
        status = LinkageUtil::ExtractNamedStringLinkageByIndex (pStringOut,
                                                              maxStringSizeIn,
                                                              linkageKey,
                                                              0,
                                                              pElementIn);
        }

    return  status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          adim_deleteStringLinkage                                |
|                                                                       |
| author        petri.niiranen                          02/01           |
|                                                                       |
+----------------------------------------------------------------------*/
int      BentleyApi::adim_deleteStringLinkage
(
DgnElement   *pElementIn,    /* <=> */
int         categoryIn,     /* => */
int         linkageKeyIn    /* => */
)
    {
    int     status = ERROR;
    UShort  linkageKey;

    if (NULL != pElementIn &&
        0    != (linkageKey = getLinkageKey (categoryIn, linkageKeyIn)))
        {
        status = LinkageUtil::DeleteStringLinkage (pElementIn, linkageKey, 0);
        }

    return  status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlDim_setUnitLabel                                     |
|                                                                       |
| author        petri.niiranen                          01/01           |
|                                                                       |
+----------------------------------------------------------------------*/
int   BentleyApi::mdlDim_setUnitLabel
(
EditElementHandleR dimElement,     /* <=> */
const WChar  *pLabelIn,       /* => NULL not setting */
int             labelFlag            /* => DIMLABEL_xxx */
)
    {
    DgnV8ElementBlank tmpElement;
    dimElement.GetElementCP()->CopyTo (tmpElement);
    StatusInt status = adim_setStringLinkage (&tmpElement, pLabelIn, LINKAGETYPE_Unit, labelFlag, MAX_UNIT_LABEL_LENGTH);
    if (SUCCESS == status)
        dimElement.ReplaceElement (&tmpElement);
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlDim_getUnitLabel                                     |
|                                                                       |
| author        petri.niiranen                          01/01           |
|                                                                       |
+----------------------------------------------------------------------*/
int   BentleyApi::mdlDim_getUnitLabel
(
WChar         *pUnitLabelOut,     /* <= NULL not interested, must be of MAX_DIMSTRING size */
ElementHandleCR    dimElement,        /* <=> */
int             labelFlag           /* => DIMLABEL_xxx */
)
    {
    return adim_getStringLinkage (pUnitLabelOut, dimElement.GetElementCP(), LINKAGETYPE_Unit, labelFlag, MAX_UNIT_LABEL_LENGTH);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlDim_removeUnitLabel                                  |
|                                                                       |
| author        petri.niiranen                          01/01           |
|                                                                       |
+----------------------------------------------------------------------*/
int   BentleyApi::mdlDim_removeUnitLabel
(
EditElementHandleR dimElement,
int         labelFlag       /* => DIMLABEL_xxx */
)
    {
    return  adim_deleteStringLinkage (dimElement.GetElementP(), LINKAGETYPE_Unit, labelFlag);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlDim_setCellName                                      |
|                                                                       |
| author        petri.niiranen                          02/01           |
|                                                                       |
+----------------------------------------------------------------------*/
Public int      mdlDim_setCellName
(
DgnElement   *pElementIn,        /* <=> */
WChar     *pCellNameIn,       /*  => */
int         cellFlag            /* DIMCELL_xxx */
)
    {
    return  adim_setStringLinkage (pElementIn, pCellNameIn, LINKAGETYPE_Cell, cellFlag, MAX_CELLNAME_LENGTH);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlDim_setCellNameUsingDescr                            |
|                                                                       |
| author        petri.niiranen                          02/01           |
|                                                                       |
+----------------------------------------------------------------------*/
int   mdlDim_setCellNameUsingDescr
(
MSElementDescr  **ppDescrIn,
WChar         *pCellNameIn,       /*  => */
int             cellFlag                /* DIMCELL_xxx */
)
    {
    return adim_setStringLinkageUsingDescr (ppDescrIn, pCellNameIn, LINKAGETYPE_Cell, cellFlag, MAX_CELLNAME_LENGTH);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlDim_getCellName                                      |
|                                                                       |
| author        petri.niiranen                          02/01           |
|                                                                       |
+----------------------------------------------------------------------*/
Public int      mdlDim_getCellName
(
WChar     *pCellNameOut,  /* <= */
DgnElement   *pElementIn,    /* <=> */
int         cellFlag        /* => DIMCELL_xxx */
)
    {
    return  adim_getStringLinkage (pCellNameOut, pElementIn, LINKAGETYPE_Cell, cellFlag, MAX_CELLNAME_LENGTH);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlDim_removeCellName                                   |
|                                                                       |
| author        petri.niiranen                          02/01           |
|                                                                       |
+----------------------------------------------------------------------*/
Public int      mdlDim_removeCellName
(
DgnElement   *pElementIn,
int         cellFlag
)
    {
    return  adim_deleteStringLinkage (pElementIn, LINKAGETYPE_Cell, cellFlag);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlDim_suppressWitnessline                              |
|                                                                       |
| author        petri.niiranen                          04/01           |
|                                                                       |
+----------------------------------------------------------------------*/
BentleyStatus     BentleyApi::mdlDim_suppressWitnessline
(
EditElementHandleR eh,
int         pointNo,
bool        bSuppressFlag
)
    {
    DimensionElm* pDim = &eh.GetElementP()->ToDimensionElmR();
    
    if (pointNo > pDim->nPoints - 1 ||  pointNo < 0)
        return  ERROR;
    
    pDim->GetDimTextP(pointNo)->flags.b.noWitness = bSuppressFlag;
    pDim->GetDimTextP(pointNo)->flags.b.witCtrlLocal = true;
    return  SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlDim_isWitnesslineSuppressed                          |
|                                                                       |
| author        Don.Fu                                  01/04           |
|                                                                       |
+----------------------------------------------------------------------*/
bool    OrdinateDimensionHelper::IsWitnesslineSuppressed (int pointNo) const
    {
    DimensionElmCP pDim = &m_dimension.GetElementCP()->ToDimensionElm();
    if (pointNo > pDim->nPoints - 1 ||  pointNo < 0)
        return  false;

    return pDim->GetDimTextCP(pointNo)->flags.b.noWitness;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
int   BentleyApi::mdlDim_getPointNumber
(
ElementHandleCR dimElement,
int         segment,
int         partType,
int         partSub,
int         closeVertex
)
    {
    int pointNo;
    DgnElementCP pElm = dimElement.GetElementCP();
    if (ADTYPE_EXT_LEFT  == partType ||
        ADTYPE_TERM_LEFT == partType)
        pointNo = segment;
    else
        pointNo = segment + 1;

    if (DIM_ANGULAR (static_cast<DimensionType>(pElm->ToDimensionElm().dimcmd)))
        {
        pointNo++;
        }
    else
    if (DIM_ORDINATE (static_cast<DimensionType>(pElm->ToDimensionElm().dimcmd)))
        {
        pointNo--;
        }
    else
    if (DIM_RADIAL (static_cast<DimensionType>(pElm->ToDimensionElm().dimcmd)))
        {
        if (mdlDim_partTypeIsAnyDimLine (partType))
            pointNo = closeVertex + 1;
        else
        if (mdlDim_partTypeIsAnyText (partType))
            pointNo = pElm->ToDimensionElm().nPoints - 1;
        else
            pointNo = pointNo + 1;
        }
    else
    if (DIM_NOTE (static_cast<DimensionType>(pElm->ToDimensionElm().dimcmd)))
        {
        if (mdlDim_partTypeIsAnyDimLine (partType))
            pointNo = closeVertex;
        else
        if (mdlDim_partTypeIsAnyText (partType))
            pointNo = pElm->ToDimensionElm().nPoints - 1;
        else
            {/*pointNo = pointNo;*/} // removed in graphite 
        }

    return pointNo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_getPartSymbology
(
Symbology   *pSymbology,
ElementHandleCR elementIn,
UInt32       partName
)
    {
    int                 partType = ADIM_GETTYPE (partName);
    DimOverrides       *pOverrides = NULL;
    DimMaterialIndex    material;

    mdlDim_overridesGet (&pOverrides, elementIn);

    if (mdlDim_partTypeIsAnyExtension (partType))
        material = DIM_MATERIAL_Extension;
    else
    if (mdlDim_partTypeIsAnyText (partType))
        material = DIM_MATERIAL_Text;
    else
    if (mdlDim_partTypeIsAnyTerminator (partType))
        material = DIM_MATERIAL_Terminator;
    else
        material = DIM_MATERIAL_DimLine;

    adim_getEffectiveSymbology (pSymbology, elementIn, material, partName, pOverrides);

    if (NULL != pOverrides)
        mdlDim_overridesFreeAll (&pOverrides);

    return ERROR;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          BentleyApi::mdlDim_setWitnessSymbologyFlag                          |
|                                                                       |
| author        DonFu                                   05/02           |
|                                                                       |
+----------------------------------------------------------------------*/
StatusInt    BentleyApi::mdlDim_setWitnessSymbologyFlag
(
EditElementHandleR  eh,
bool                useWitnessSymb,
int                 fromNo,
int                 toNo
)
    {
    if  (fromNo >= 0 && toNo >= 0)
        {
        int     i;

        if (toNo > eh.GetElementP()->ToDimensionElm().nPoints - 1)
            toNo = eh.GetElementP()->ToDimensionElm().nPoints - 1;

        for (i = fromNo; i <= toNo; i++)
            eh.GetElementP()->ToDimensionElmR().GetDimTextP(i)->flags.b.altSymb = useWitnessSymb;

        return  SUCCESS;
        }

    return  ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionHandler::GetTextStyleID (ElementHandleCR dimElement, UInt32*  pTextStyleID) const
    {
    if (DIMENSION_ELM != dimElement.GetLegacyType())
        return DGNHANDLERS_STATUS_BadArg;

    if (pTextStyleID)
        *pTextStyleID = dimElement.GetElementCP()->ToDimensionElm().textStyleId;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionHandler::SetTextStyleID (EditElementHandleR eh, UInt32 textStyleID)
    {
    DgnElement *pDimElm = eh.GetElementP();
    if (DIMENSION_ELM != pDimElm->GetLegacyType())
        return DGNHANDLERS_STATUS_BadArg;

    pDimElm->ToDimensionElmR().textStyleId = textStyleID;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionHandler::GetTextStyle (ElementHandleCR dimElement, MdlTextStyle* pTextStyleOut) const
    {
    if (!dimElement.IsValid() || DIMENSION_ELM != dimElement.GetLegacyType())
        return DGNHANDLERS_STATUS_BadArg;

    if (NULL == pTextStyleOut)
        return ERROR; //MDLERR_NULLOUTPUTBUFFER;
    
    DimensionElm const* dim = &dimElement.GetElementCP()->ToDimensionElm();
    if (SUCCESS != ElementUtil::ExtractTextStyleFromLinkage (*pTextStyleOut, dimElement, KEY_TEXTSTYLE_DIMSTYLE))
        memset (pTextStyleOut, 0, sizeof *pTextStyleOut);

    /* The following properties are stored in the main part of the dimension
       because they existed BEFORE the text style was stored in the dimension */
    pTextStyleOut->fontNo           = dim->text.font;
    pTextStyleOut->height           = dim->text.height;
    pTextStyleOut->width            = dim->text.width;
    pTextStyleOut->color            = dim->text.color;
    pTextStyleOut->flags.underline  = dim->flag.underlineText;
    pTextStyleOut->flags.color      = dim->text.b.useColor;
    pTextStyleOut->flags.fractions  = !dim->flag.stackFract;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* WARNING: It is typically not safe to call this function directly. Use
*          mdlDim_setTextStyle2.
* @bsimethod                                                    JoshSchifter    07/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::adim_setTextStyle
(
EditElementHandleR  dimElm,             /* => */
const LegacyTextStyle  *pTextStyle,        /* => */
bool                sizeChangeAllowed   /* => usually true */
)
    {
    DimensionElmP  pDimElm = &dimElm.GetElementP()->ToDimensionElmR();

    if (DIMENSION_ELM != pDimElm->GetLegacyType())
        return DGNHANDLERS_STATUS_BadArg;

    /* The following properties are stored in the main part of the dimension
       because they existed BEFORE the text style was stored in the dimension */
    pDimElm->text.font          = pTextStyle->fontNo;
    pDimElm->text.height        = pTextStyle->height;
    pDimElm->text.width         = pTextStyle->width;
    pDimElm->text.color         = pTextStyle->color;
    pDimElm->flag.underlineText = (unsigned int) pTextStyle->flags.underline;
    pDimElm->text.b.useColor    = (unsigned int) pTextStyle->flags.color;
    pDimElm->flag.stackFract    = (unsigned int) ! pTextStyle->flags.fractions;

    if ( ! sizeChangeAllowed)
        return SUCCESS;

    // Do not store Style:None textstyle linkage
    if (0 == pDimElm->textStyleId)
        {
        ElementUtil::DeleteTextStyleLinkage (dimElm, KEY_TEXTSTYLE_DIMSTYLE);
        return SUCCESS;
        }

    LegacyTextStyle   textStyle = *pTextStyle;

    textStyle.fontNo = 0;
    textStyle.height = 0.0;
    textStyle.width  = 0.0;
    textStyle.color  = 0;
    textStyle.flags.underline = false;
    textStyle.flags.color     = false;
    textStyle.flags.fractions = false;

    return ElementUtil::AppendTextStyleAsLinkage (dimElm, textStyle, KEY_TEXTSTYLE_DIMSTYLE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionHandler::SetTextStyle
(
EditElementHandleR      dimElm,             /* <=> Requires a full size buffer, unless sizeChangeAllowed is false */
const LegacyTextStyle*     pTextStyle,         /*  => */
bool                    setOverrideFlags,   /*  => */
bool                    sizeChangeAllowed   /*  => usually true */
)
    {
    if (!dimElm.IsValid() || DIMENSION_ELM != dimElm.GetLegacyType())
        return DGNHANDLERS_STATUS_BadArg;

    LegacyTextStyle   localBuff;
    LegacyTextStyle   oldTextStyle;
    GetTextStyle (dimElm, &oldTextStyle);

    // Set the override flags
    if (setOverrideFlags && sizeChangeAllowed)
        {
        localBuff = *pTextStyle;

        // Turn on the TSO for any property that is being changed.
        DgnTextStylePtr dimensionStyle = DgnTextStyle::Create (*dimElm.GetDgnProject());
        DgnTextStylePersistence::Legacy::FromLegacyStyle(*dimensionStyle, *pTextStyle);
        DgnTextStylePtr oldStyle = DgnTextStyle::Create(*dimElm.GetDgnProject());
        DgnTextStylePersistence::Legacy::FromLegacyStyle(*oldStyle, oldTextStyle);
        
        DgnTextStylePropertyMaskPtr diff = dimensionStyle->Compare(*oldStyle);
        DgnTextStylePropertyMaskPtr oldFlags = DgnTextStylePersistence::Legacy::FromLegacyMask(oldTextStyle.overrideFlags);
        diff->LogicalOr(*oldFlags);
        localBuff.overrideFlags = DgnTextStylePersistence::Legacy::ToLegacyMask(*diff);
        
        DimensionHandler&   hdlr            = DimensionHandler::GetInstance();
        DimStylePropMaskPtr shieldFlags     = hdlr.GetOverrideFlags (dimElm);

        // If the value is being changed, set the corresponding shield as well.
        // NOTE: only explicit TSOs have corresponding shields.
        setShield (oldTextStyle.fontNo          != localBuff.fontNo,          DIMSTYLE_PROP_General_Font_FONT,              *shieldFlags);
        setShield (oldTextStyle.height          != localBuff.height,          DIMSTYLE_PROP_Text_Height_DOUBLE,             *shieldFlags);
        setShield (oldTextStyle.width           != localBuff.width,           DIMSTYLE_PROP_Text_Width_DOUBLE,              *shieldFlags);
        setShield (oldTextStyle.flags.underline != localBuff.flags.underline, DIMSTYLE_PROP_Text_Underline_BOOLINT,         *shieldFlags);
        setShield (oldTextStyle.flags.fractions != localBuff.flags.fractions, DIMSTYLE_PROP_Text_StackedFractions_BOOLINT,  *shieldFlags);
        setShield (oldTextStyle.flags.color     != localBuff.flags.color,     DIMSTYLE_PROP_Text_OverrideColor_BOOLINT,     *shieldFlags);
        setShield (oldTextStyle.color           != localBuff.color,           DIMSTYLE_PROP_Text_Color_COLOR,               *shieldFlags);

        mdlDim_setOverridesDirect (dimElm, shieldFlags.get(), false);

        pTextStyle = &localBuff;
        }

    // Change the stored strings to use the new font's encoding
    if (sizeChangeAllowed)
        BentleyApi::adim_changeTextHeapEncoding (dimElm, pTextStyle->fontNo,  pTextStyle->shxBigFont,
                                                    oldTextStyle.fontNo, oldTextStyle.shxBigFont);

    // Store the new data on the dimension element
    return adim_setTextStyle (dimElm, pTextStyle, sizeChangeAllowed);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    SunandSandurkar             07/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_getNoteHorizontalJustification
(
DimStyleProp_MLNote_Justification   *eJust,
ElementHandleCR dimElement
)
    {
    if (!dimElement.IsValid())
        return ERROR;

    DimensionElm const* dim = &dimElement.GetElementCP()->ToDimensionElm();

    // We are storing this value on the dimension element starting in 8.5.1.
    // A value of 0 means it is a pre-8.5.1 note, whose value must be derived from
    // the note cell geometry
    if (0 == dim->extFlag.uNoteHorJust)
        return ERROR;

    if (eJust)
        *eJust = (DimStyleProp_MLNote_Justification) (dim->extFlag.uNoteHorJust - 1);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    SunandSandurkar             07/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BentleyApi::mdlDim_setNoteHorizontalJustification
(
EditElementHandleR dimElement,
DimStyleProp_MLNote_Justification   eJust
)
    {
    DimensionElm * dim = &dimElement.GetElementP()->ToDimensionElmR();

    // We are storing this value on the dimension element starting in 8.5.1.
    // A value of 0 means it is a pre-8.5.1 note, whose value must be derived from
    // the note cell geometry
    dim->extFlag.uNoteHorJust = eJust + 1;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    SunandSandurkar             07/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt               BentleyApi::mdlDim_getNoteFrameType
(
DimStyleProp_MLNote_FrameType * eFrameType,
ElementHandleCR                    dimElement
)
    {
    DimStyleExtensions              extensions;
    DimStyleProp_MLNote_FrameType   frameType = DIMSTYLE_VALUE_MLNote_FrameType_None;

    if (!dimElement.IsValid())
        return ERROR;

    DimensionElm const* dim = &dimElement.GetElementCP()->ToDimensionElm();

    // Note 1:
    // We are storing this value on the dimension element starting in 8.5.1.
    // A value of 0 means it is a pre-8.5.1 note, whose value must be derived from
    // the note cell geometry

    // Note 2:
    // In Mozart the frame type is gathered from two flags.

    if (0 == dim->extFlag.uNoteFrameType)
        return ERROR;

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);

    switch (dim->extFlag.uNoteFrameType)
        {
        case 1:
            frameType = DIMSTYLE_VALUE_MLNote_FrameType_None;
            break;

        case 2:
            frameType = DIMSTYLE_VALUE_MLNote_FrameType_Line;
            break;

        case 3:
            frameType = DIMSTYLE_VALUE_MLNote_FrameType_Box;
            break;

        case 4:
            {
            switch (extensions.flags2.uNoteFrame)
                {
                case 0:
                    frameType = DIMSTYLE_VALUE_MLNote_FrameType_RotatedBox;
                    break;
                case 1:
                    frameType = DIMSTYLE_VALUE_MLNote_FrameType_Circle;
                    break;
                case 2:
                    frameType = DIMSTYLE_VALUE_MLNote_FrameType_Capsule;
                    break;
                case 3:
                    frameType = DIMSTYLE_VALUE_MLNote_FrameType_Hexagon;
                    break;
                case 4:
                    frameType = DIMSTYLE_VALUE_MLNote_FrameType_RotatedHexagon;
                    break;
                case 5:
                    frameType = DIMSTYLE_VALUE_MLNote_FrameType_Triangle;
                    break;
                case 6:
                    frameType = DIMSTYLE_VALUE_MLNote_FrameType_Pentagon;
                    break;
                case 7:
                    frameType = DIMSTYLE_VALUE_MLNote_FrameType_Octagon;
                    break;
                }
            break;
            }
        }

    if (eFrameType)
        *eFrameType = frameType;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    SunandSandurkar             07/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt               BentleyApi::mdlDim_setNoteFrameType
(
EditElementHandleR dimElement,
DimStyleProp_MLNote_FrameType   eFrameType
)
    {
    int                         extensionflag = 0;
    DimensionElm * dim = &dimElement.GetElementP()->ToDimensionElmR();
    DimStyleExtensions  extensions;

    // Note 1:
    // We are storing this value on the dimension element starting in 8.5.1.
    // A value of 0 means it is a pre-8.5.1 note, whose value must be derived from
    // the note cell geometry

    // Note 2:
    // In Mozart the frame type is gathered from two flags - extFlag.uNoteFrameType and
    // extensions.flags2.uNoteFrame.

    switch (eFrameType)
        {
        case DIMSTYLE_VALUE_MLNote_FrameType_None:
            dim->extFlag.uNoteFrameType = 1;
            break;

        case DIMSTYLE_VALUE_MLNote_FrameType_Line:
            dim->extFlag.uNoteFrameType = 2;
            break;

        case DIMSTYLE_VALUE_MLNote_FrameType_Box:
            dim->extFlag.uNoteFrameType = 3;
            break;

        case DIMSTYLE_VALUE_MLNote_FrameType_RotatedBox:
            dim->extFlag.uNoteFrameType = 4;
            break;

        case DIMSTYLE_VALUE_MLNote_FrameType_Circle:
            dim->extFlag.uNoteFrameType = 4;
            extensionflag = 1;
            break;

        case DIMSTYLE_VALUE_MLNote_FrameType_Capsule:
            dim->extFlag.uNoteFrameType = 4;
            extensionflag = 2;
            break;

        case DIMSTYLE_VALUE_MLNote_FrameType_Hexagon:
            dim->extFlag.uNoteFrameType = 4;
            extensionflag = 3;
            break;

        case DIMSTYLE_VALUE_MLNote_FrameType_RotatedHexagon:
            dim->extFlag.uNoteFrameType = 4;
            extensionflag = 4;
            break;

        case DIMSTYLE_VALUE_MLNote_FrameType_Triangle:
            dim->extFlag.uNoteFrameType = 4;
            extensionflag = 5;
            break;

        case DIMSTYLE_VALUE_MLNote_FrameType_Pentagon:
            dim->extFlag.uNoteFrameType = 4;
            extensionflag = 6;
            break;

        case DIMSTYLE_VALUE_MLNote_FrameType_Octagon:
            dim->extFlag.uNoteFrameType = 4;
            extensionflag = 7;
            break;
        }

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);
    extensions.flags2.uNoteFrame = extensionflag;
    mdlDim_setStyleExtension (dimElement, &extensions);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    SunandSandurkar             07/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_getNoteLeaderDisplay
(
bool*           bDisplay,
ElementHandleCR dimElement
)
    {
    if (!dimElement.IsValid())
        return ERROR;

    DimensionElm const* dim = &dimElement.GetElementCP()->ToDimensionElm();

    // We are storing this value on the dimension element starting in 8.5.1.
    // A value of 0 means it is a pre-8.5.1 note, whose value must be derived from
    // the note cell geometry
    if (NULL == dim || 0 == dim->extFlag.uNoteLeaderDisplay)
        return ERROR;

    if (bDisplay)
        *bDisplay = TO_BOOL (dim->extFlag.uNoteLeaderDisplay - 1);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    SunandSandurkar             07/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BentleyApi::mdlDim_setNoteLeaderDisplay
(
EditElementHandleR dimElement,
bool                bDisplay
)
    {
    DimensionElm * dim = &dimElement.GetElementP()->ToDimensionElmR();

    // We are storing this value on the dimension element starting in 8.5.1.
    // A value of 0 means it is a pre-8.5.1 note, whose value must be derived from
    // the note cell geometry
    dim->extFlag.uNoteLeaderDisplay = bDisplay + 1;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    SunandSandurkar             09/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool        BentleyApi::mdlDim_getNoteAllowAutoMode
(
ElementHandleCR dimElement
)
    {
    return (dimElement.IsValid()) ? dimElement.GetElementCP()->ToDimensionElm().extFlag.uNoteAllowAutoMode : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    SunandSandurkar             09/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BentleyApi::mdlDim_setNoteAllowAutoMode
(
EditElementHandleR dimElement,
bool                bAllow
)
    {
    // This flag is intended to distinguish legacy notes that have attachment = auto
    // but the actual attachment side does not correspond to auto OR horJust = dynamic
    // but the actual horizontal justification does not correspond to auto. Such notes
    // were generated pre-8.5.1 when Location = Manual and the user resets on the opposite
    // side of the default auto location.
    // If we do not distinguish them, then mdlNote_create will flip them to correspond
    // to the proper auto attachment or dynamic horizontal justification. Since we cannot
    // make this classification, we are tagging all pre-8.5.1 notes to not allow
    // auto attachment or dynamic horizontal justification.
    DimensionElm * dim = &dimElement.GetElementP()->ToDimensionElmR();
    dim->extFlag.uNoteAllowAutoMode = (bAllow ? 1 : 0);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    SunandSandurkar             07/02
+---------------+---------------+---------------+---------------+---------------+------*/
double          LinearDimensionHelper::GetExtensionHeightDifference (int segNo) const
    {
    /* The extension height is computed for the second segment onwards */
    if (segNo == 0)
        return 0;

    DVec3d      xVec, yVec;
    RotMatrix   rMatrix;
    DgnElementCP dimElement = m_dimension.GetElementCP();

    m_hdlr.GetRotationMatrix (m_dimension, rMatrix);

    rMatrix.GetColumn(xVec,  0); // along dimension line
    rMatrix.GetColumn(yVec,  1); // along extension line

    xVec.Normalize ();
    yVec.Normalize ();

    DPoint3dCR  point0 = dimElement->ToDimensionElm().GetPoint(0);
    DPoint3dCR  point1 = dimElement->ToDimensionElm().GetPoint(segNo);
    DRay3d      ray0 = DRay3d::FromOriginAndVector (point0, xVec);
    DRay3d      ray1 = DRay3d::FromOriginAndVector (point1, yVec);
    DPoint3d    intersect0, intersect1;
    double      dummy0, dummy1;

    bool        retVal = DRay3d::ClosestApproachUnboundedRayUnboundedRay (dummy0, dummy1, intersect0, intersect1, ray0, ray1);    

    BeAssert (true == retVal);
    BeAssert (mgds_fc_epsilon > intersect0.DistanceSquared(intersect1));

    DVec3d      difference;
    difference.DifferenceOf (point1, intersect0);

    BeAssert (mgds_fc_epsilon > difference.magnitudeSquared() || difference.IsParallelTo (yVec));

    return difference.DotProduct (yVec);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    SunandSandurkar             05/03
+---------------+---------------+---------------+---------------+---------------+------*/
void     IDimElementHelper::ResolveTerminators (EditElementHandleR element, int iSegment) const
    {
    BeAssert (GetNumberofSegments() > 1);

    DimStylePropMaskPtr shieldFlags     = m_hdlr.GetOverrideFlags (element);

    DimensionType   dimensionType;
    if (DimensionType::None == (dimensionType = adim_getTemplateNumber (element)))
        {
        // BJB: I added this when changing to use dimensionType. I am not sure whether this is an error condition or even whether it should exit. 
        // Someone knowledgeable in Dimensionology should review if this assert is hit.
        BeAssert (false);
        return;
        }

    /*-------------------------------------------------------------------
      Resolve the Start Terminator
        - First Seg (works out automatically)
            - If first term is available, use it
            - Else, use left term
        - All Other Segs
            - Stacked
                - Use left term
            - NonStacked
                - If joint term is available, use it
                - Else, use left term
    -------------------------------------------------------------------*/
    if (iSegment > 0)
        {
        if (element.GetElementCP()->ToDimensionElm().tmpl.stacked)
            {
            if (element.GetElementCP()->ToDimensionElm().tmpl.first_term)
                {
                element.GetElementP()->ToDimensionElmR().tmpl.first_term = element.GetElementCP()->ToDimensionElm().tmpl.left_term;

                shieldFlags->SetTemplateBit (DIMSTYLE_PROP_Terminator_First_TEMPLATEFLAG, true, dimensionType);
                }
            }
        else
            {
            if (0 != element.GetElementCP()->ToDimensionElm().tmpl.bowtie_symbol)
                {
                element.GetElementP()->ToDimensionElmR().tmpl.left_term  = element.GetElementCP()->ToDimensionElm().tmpl.bowtie_symbol;

                shieldFlags->SetTemplateBit (DIMSTYLE_PROP_Terminator_Left_TEMPLATEFLAG, true, dimensionType);

                if (element.GetElementP()->ToDimensionElm().tmpl.first_term)
                    {
                    element.GetElementP()->ToDimensionElmR().tmpl.first_term = element.GetElementCP()->ToDimensionElm().tmpl.bowtie_symbol;
                    shieldFlags->SetTemplateBit (DIMSTYLE_PROP_Terminator_First_TEMPLATEFLAG, true, dimensionType);
                    }
                }
            else
                {
                if (element.GetElementCP()->ToDimensionElm().tmpl.first_term)
                    {
                    element.GetElementP()->ToDimensionElmR().tmpl.first_term = element.GetElementCP()->ToDimensionElm().tmpl.left_term;

                    shieldFlags->SetTemplateBit (DIMSTYLE_PROP_Terminator_First_TEMPLATEFLAG, true, dimensionType);
                    }
                }
            }
        }

    /*-------------------------------------------------------------------
      Resolve the End Terminator (Only for NonStacked)
        - Last Seg
            - Use right term (works out automatically)
        - All Other Segs
            - If there is a joint term, use it
            - Else, use right term
    -------------------------------------------------------------------*/
    if (!element.GetElementCP()->ToDimensionElm().tmpl.stacked && iSegment < GetNumberofSegments()-1)
        {
        if (0 != element.GetElementCP()->ToDimensionElm().tmpl.bowtie_symbol)
            {
            element.GetElementP()->ToDimensionElmR().tmpl.right_term = element.GetElementCP()->ToDimensionElm().tmpl.bowtie_symbol;

            shieldFlags->SetTemplateBit (DIMSTYLE_PROP_Terminator_Right_TEMPLATEFLAG, true, dimensionType);
            }
        }

    mdlDim_setOverridesDirect (element, shieldFlags.get(), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    JoshSchifter                    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            LinearDimensionHelper::RecalcTextOffsetForLinearSingleDims
(
EditElementHandleR          segmentDim,
ElementHandleCR             origDim,
bvector<double> const&      xTextOffsets,
UInt32                      iSegment
) const
    {
    if (DIMTEXT_RIGHT == segmentDim.GetElementCP()->ToDimensionElm().GetDimTextCP(1)->flags.b.just)
        return;

    if (iSegment >= xTextOffsets.size())
        { BeAssert (0); return; }

    RotMatrix   rMatrix;
    m_hdlr.GetRotationMatrix (origDim, rMatrix);

    DVec3d      dimLineVec;
    rMatrix.GetColumn(dimLineVec,  0);
    dimLineVec.Normalize ();

    // Vector from Point[0] to segment start point
    DVec3d      dStartVec;
    dStartVec.DifferenceOf (origDim.GetElementCP()->ToDimensionElm().GetPoint(iSegment), origDim.GetElementCP()->ToDimensionElm().GetPoint(0));

    // Project the start point bvector onto the dimline bvector
    segmentDim.GetElementP()->ToDimensionElmR().GetDimTextP(1)->flags.b.just = DIMTEXT_OFFSET;
    segmentDim.GetElementP()->ToDimensionElmR().GetDimTextP(1)->offset       = xTextOffsets[iSegment] + dStartVec.DotProduct (dimLineVec);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    SunandSandurkar             07/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionHandler::DropDimensionToSegments (ElementAgendaR droppedDimension, ElementHandleCR dimension)
    {
    IDimElementHelperPtr helper = DimensionHelperFactory::CreateHelper (dimension);
    if (helper.IsNull())
        return ERROR;
    
    if (helper->GetNumberofSegments() < 2)
        return ERROR;

    return helper->DropToSegment (droppedDimension);
    }

/*---------------------------------------------------------------------------------**//**
* Gets referred style unique id.
*
* @param        pUniqueId       <=
* @bsimethod                                                    petri.niiranen  03/01
+---------------+---------------+---------------+---------------+---------------+------*/
UInt64 BentleyApi::mdlDim_getDimStyleID
(
ElementHandleCR dimElement
)
    {
    return (DIMENSION_ELM == dimElement.GetLegacyType()) ? dimElement.GetElementCP()->ToDimensionElm().dimStyleId : 0;
    }

/*---------------------------------------------------------------------------------**//**
* Remove associativity to a style.

* @param        pElementIn      <=>
* @return       assignment status.
* @bsimethod                                                    petri.niiranen  03/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_removeStyle
(
EditElementHandleR dimElement
)
    {
    return  mdlDim_setStyleUniqueId (dimElement, 0);
    }

/*---------------------------------------------------------------------------------**//**
//adopted from                                     SunandSandurkar             07/02
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   LinearDimensionHelper::DropToSegment (ElementAgendaR droppedDimension) const
    {
    BentleyStatus status = ERROR;
    bvector<double>     xTextOffsets;

    EditElementHandle dimElement(m_dimension, true);
    /* For locate single dims gather up the current text offsets, we will need them later */
    if (DimensionType::LocateSingle == static_cast<DimensionType>(dimElement.GetElementCP()->ToDimensionElm().dimcmd))
        {
        for (int iSeg = 0; iSeg < GetNumberofSegments(); iSeg++)
            xTextOffsets.push_back (dimElement.GetElementCP()->ToDimensionElm().GetDimTextCP(iSeg+1)->offset);
        }
    
    /* Create the new dimension elements and display them */
    for (int iSegment = 0; iSegment < GetNumberofSegments(); iSegment++)
        {
        EditElementHandle localElement(dimElement, true);
        
        /* Delete all points beyond the current segment's end point */
        for (int iNext = GetNumberofSegments(); iNext > iSegment+1; iNext--)
            {
            if (SUCCESS != (status = m_hdlr.DeletePoint (localElement, iNext)))
                return status;
            }

        if (dimElement.GetElementCP()->ToDimensionElm().tmpl.stacked || DimensionType::LocateSingle == static_cast<DimensionType>(dimElement.GetElementCP()->ToDimensionElm().dimcmd))
            {
            /* For stacked dimension, delete all points except point[0] since it is always the start point */
            for (int iPrevious = iSegment; iPrevious > 0; iPrevious--)
                {
                if (SUCCESS != (status = m_hdlr.DeletePoint (localElement, iPrevious)))
                    return status;
                }
            }
        else
            {
            /* For chain dimensions, simply delete all points before start */
            for (int iPrevious = iSegment-1; iPrevious >= 0; iPrevious--)
                {
                if (SUCCESS != (status = m_hdlr.DeletePoint (localElement, iPrevious)))
                    return status;
                }
            }

        /* Set the new dimension's offset based on the appropriate rule. */
        if (dimElement.GetElementCP()->ToDimensionElm().tmpl.stacked || DimensionType::LocateSingle == static_cast<DimensionType>(dimElement.GetElementCP()->ToDimensionElm().dimcmd))
            {
            localElement.GetElementP()->ToDimensionElmR().GetDimTextP(0)->offset = dimElement.GetElementCP()->ToDimensionElm().GetDimTextCP(0)->offset +
                                                  mdlDim_getStackHeight (dimElement, iSegment+1);
            }
        else
            {
            localElement.GetElementP()->ToDimensionElmR().GetDimTextP(0)->offset = dimElement.GetElementCP()->ToDimensionElm().GetDimTextCP(0)->offset +
                                                  mdlDim_getStackHeight (dimElement, iSegment+1) -
                                                  GetExtensionHeightDifference (iSegment);
            }

        /* Recalculate the X-offset since it is based on point[0] */
        if (DimensionType::LocateSingle == static_cast<DimensionType>(dimElement.GetElementCP()->ToDimensionElm().dimcmd))
            RecalcTextOffsetForLinearSingleDims (localElement, dimElement, xTextOffsets, iSegment);

        /* Fix up the terminators */
        ResolveTerminators (localElement, iSegment);

        /* Validate the dimension */
        if (SUCCESS != (status = m_hdlr.ValidateElementRange (localElement)))
            return status;

        /* Collect all the new elements */
        droppedDimension.Insert (localElement);
        }
    
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   OrdinateDimensionHelper::DropToSegment (ElementAgendaR droppedDimension) const
    {
    BentleyStatus   status = ERROR;
    bool            b1stHidden = IsWitnesslineSuppressed (0);

    /*-----------------------------------------------------------------------------------
    For an ordinate dimension, the segment number is the same as point number. A dropped
    ordinate dimension should consist of two points/segments: the base and the leader.
    The base segment becomes hidden by suppressing its witness line and text.

    If an ordinate dimension is found to only consist of these two segments (base+datum)
    do not drop it.
    -----------------------------------------------------------------------------------*/
    if (GetNumberofSegments() <= 2 && b1stHidden)
        return  ERROR;

    /*-----------------------------------------------------------------------------------
    Similarly, if the base segment is hidden, keep the base and second segment as one
    dimension that will not be dropped.
    -----------------------------------------------------------------------------------*/
    int iFirst = b1stHidden ? 1 : 0;
    for (int iSegment = iFirst; iSegment < GetNumberofSegments(); iSegment++)
        {
        EditElementHandle localElement(m_dimension, true);

        /* Delete all points after the current point */
        for (int iNext = GetNumberofSegments()-1; iNext > iSegment; iNext--)
            {
            if (SUCCESS != (status = m_hdlr.DeletePoint (localElement, iNext)))
                return status;
            }

        /* Delete all points after the current point except for the first one */
        for (int iPrevious = iSegment-1; iPrevious > 0; iPrevious--)
            {
            if (SUCCESS != (status = m_hdlr.DeletePoint (localElement, iPrevious)))
                return status;
            }

        /* Replace the automatic stacked offset with free location */
        if (iSegment > 0 && m_dimension.GetElementCP()->ToDimensionElm().tmpl.stacked)
            {
            /* remove the stacked flag */
            localElement.GetElementP()->ToDimensionElmR().tmpl.stacked = false;

            /* set the y-offset with the stacked height */
            localElement.GetElementP()->ToDimensionElmR().GetDimTextP(1)->offsetY = mdlDim_getStackHeight (m_dimension, iSegment);

            /* set free location flag */
            if  (fabs(localElement.GetElementCP()->ToDimensionElm().GetDimTextCP(1)->offsetY) > 0.0)
                {
                bool        freeLocation = true;

                mdlDim_setOrdinateFreeLocationFlag (localElement, &freeLocation);
                }
            }

        /* Hide the first segment(base)'s witness line and text */
        if (iSegment > 0)
            status = mdlDim_suppressWitnessline (localElement, 0, true);

        /* Validate the dimension */
        if (SUCCESS != (status = m_hdlr.ValidateElementRange (localElement)))
            return status;

        /* Collect all the new elements */
        droppedDimension.Insert (localElement);
        }
    
    return status;
    }

/*---------------------------------------------------------------------------------**//**
//adopted from                                     SunandSandurkar             07/02
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AngularDimensionHelper::DropToSegment (ElementAgendaR droppedDimension) const
    {
    BentleyStatus   status      = ERROR;

    /* Create the new dimension elements and display them */
    for (int iSegment = 0; iSegment <GetNumberofSegments (); iSegment++)
        {
        EditElementHandle localElement(m_dimension, true);
        /* Delete all points beyond the current segment's end point */
        for (int iNext = GetNumberofSegments() +1; iNext > iSegment+2; iNext--)
            {
            if (SUCCESS != (status = m_hdlr.DeletePoint (localElement, iNext)))
                return status;
            }

        if (localElement.GetElementCP()->ToDimensionElm().tmpl.stacked)
            {
            /* For stacked dimension, delete all points except point[1] since it is always the start point */
            for (int iPrevious = iSegment+1; iPrevious > 1; iPrevious--)
                {
                if (SUCCESS != (status = m_hdlr.DeletePoint (localElement, iPrevious)))
                    return status;
                }
            }
        else
            {
            /* For chain dimensions, simply delete all points before start */
            for (int iPrevious = iSegment; iPrevious > 0; iPrevious--)
                {
                if (SUCCESS != (status = m_hdlr.DeletePoint (localElement, iPrevious)))
                    return status;
                }
            }

        /* For all segments except the first one, the extension line offset needs to be computed */
        if (iSegment>0)
            {
            double firstRadius = localElement.GetElementCP()->ToDimensionElm().GetPoint(0).Distance (m_dimension.GetElementCP()->ToDimensionElm().GetPoint(1));
            double startRadius = localElement.GetElementCP()->ToDimensionElm().GetPoint(0).Distance (m_dimension.GetElementCP()->ToDimensionElm().GetPoint(1));

            localElement.GetElementP()->ToDimensionElmR().GetDimTextP(1)->offset = (firstRadius-startRadius) + m_dimension.GetElementCP()->ToDimensionElm().GetDimTextCP(1)->offset;

            if (m_dimension.GetElementCP()->ToDimensionElm().tmpl.stacked)
                localElement.GetElementP()->ToDimensionElmR().GetDimTextP(1)->offset += mdlDim_getStackHeight (m_dimension, iSegment+1);
            }

        /* Fix up the terminators */
        ResolveTerminators (localElement, iSegment);

        /* Validate the dimension */
        if (SUCCESS != (status = m_hdlr.ValidateElementRange (localElement)))
            return status;
         
        /* Collect all the new elements */
        droppedDimension.Insert (localElement);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
IDimElementHelperPtr DimensionHelperFactory::CreateHelper (ElementHandleCR element)
    {
    DimensionHandler* hdlr = dynamic_cast<DimensionHandler*> (&element.GetHandler());
    if (NULL == hdlr)
        return NULL;
    
    IDimElementHelperPtr    helper;
    DimensionType           dimensionType = static_cast<DimensionType>(element.GetElementCP()->ToDimensionElm().dimcmd);
    if (DIM_LINEAR (dimensionType))
        helper = new LinearDimensionHelper(element, *hdlr);

    if (DIM_ANGULAR (dimensionType))
        helper =  new AngularDimensionHelper (element, *hdlr);
    
    if (DIM_ORDINATE (dimensionType))
        helper =  new OrdinateDimensionHelper (element, *hdlr);

    if (DIM_LABELLINE (dimensionType))
        helper =  new LabelLineHelper (element, *hdlr);

    if (DIM_NOTE (dimensionType))
        helper =  new NoteDimensionHelper (element, *hdlr);

    if (DIM_RADIAL (dimensionType))
        helper =  new RadialDimensionHelper (element, *hdlr);

    return helper;
    }
