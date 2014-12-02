/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnFileIO/SheetDef.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#define RETURN_OR_SET_COMPARE_BITMASK(compareBitMask, isEqual, compareBitPosition)  \
        {                                                                           \
        if (NULL != compareBitMask)                                                 \
            {                                                                       \
            isEqual = false;                                                        \
            compareBitMask->Set (compareBitPosition);                               \
            }                                                                       \
        else                                                                        \
            {                                                                       \
            return false;                                                           \
            }                                                                       \
        }

#ifdef DGN_IMPORTER_REORG_WIP
static struct {UInt16 attrid; UInt16 stringLinkageId;} s_propertyStrings[] =
    {
    {SHEET_DEF_FORM_NAME_ATTRIBUTE           , STRING_LINKAGE_KEY_SheetFormName},   
    {SHEET_DEF_PAPER_FORM_NAME_ATTRIBUTE     , STRING_LINKAGE_KEY_PaperFormName},
    {SHEET_DEF_WINDOWS_PRINTER_NAME_ATTRIBUTE, STRING_LINKAGE_KEY_WindowsPrinterName},
    {SHEET_DEF_PLT_FILE_NAME_ATTRIBUTE       , STRING_LINKAGE_KEY_PltFileName},
    {SHEET_DEF_PST_FILE_NAME_ATTRIBUTE       , STRING_LINKAGE_KEY_PstFileName},
    {SHEET_DEF_SHEET_NAME_ATTRIBUTE          , STRING_LINKAGE_KEY_SheetName},
    {SHEET_DEF_PRINT_STYLE_NAME_ATTRIBUTE    , STRING_LINKAGE_KEY_PrintStyleName},
    };
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool      SheetDef::IsEnabled () const
    {
    return properties.isEnabled;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            SheetDef::Enable (bool isEnabledIn)
    {
    properties.isEnabled = isEnabledIn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            SheetDef::GetOrigin (DPoint2d& sheetOriginOut) const
    {
    sheetOriginOut = properties.origin;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            SheetDef::SetOrigin (DPoint2d const& sheetOrigin)
    {
    properties.origin = sheetOrigin;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
double SheetDef::GetRotation () const    
    {
    return properties.rotation;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::SetRotation (double sheetRotationIn)
    {
    properties.rotation = sheetRotationIn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::GetSize
(
double&            sheetWidthOut,
double&            sheetHeightOut
) const
    {
    sheetWidthOut = properties.width;
    sheetHeightOut = properties.height;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::SetSize
(
double              sheetWidthIn,
double              sheetHeightIn
)
    {
    properties.width = sheetWidthIn;
    properties.height = sheetHeightIn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::GetSheetName (WString& sheetNameOut) const
    {
    properties.GetString (sheetNameOut, SHEET_DEF_SHEET_NAME_ATTRIBUTE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::SetSheetName (WChar const* sheetName)
    {
    properties.SetString (SHEET_DEF_SHEET_NAME_ATTRIBUTE, sheetName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::GetFormName (WString& formNameOut) const
    {
    properties.GetString (formNameOut, SHEET_DEF_FORM_NAME_ATTRIBUTE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::SetFormName (WChar const* formNameIn)
    {
    properties.SetString (SHEET_DEF_FORM_NAME_ATTRIBUTE, formNameIn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 SheetDef::GetColor () const    
    {
    return properties.color;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::SetColor (UInt32 sheetColorIn)
    {
    properties.color = sheetColorIn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::GetUnits (UnitDefinitionR unitDefOut) const
    {
    unitDefOut = properties.unitDef;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
void SheetDef::SetUnits (UnitDefinitionCR unitDefIn)
    {
    properties.unitDef = unitDefIn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::GetPaperOrigin (DPoint2d& paperOriginOut) const
    {
    paperOriginOut = properties.paperOrigin;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::SetPaperOrigin (DPoint2d const& paperOrigin)
    {
    properties.paperOrigin = paperOrigin;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
double SheetDef::GetPaperRotation () const    
    { 
    return properties.paperRotation;    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::SetPaperRotation (double paperRotationIn)
    {
    properties.paperRotation = paperRotationIn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::GetPaperSize
(
double&            paperWidthOut,
double&            paperHeightOut
) const
    {
    paperWidthOut = properties.paperWidth;
    paperHeightOut = properties.paperHeight;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::SetPaperSize
(
double              paperWidthIn,
double              paperHeightIn
)
    {
    properties.paperWidth = paperWidthIn;
    properties.paperHeight = paperHeightIn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::GetPaperFormName (WString& formNameOut) const
    {
    properties.GetString (formNameOut, SHEET_DEF_PAPER_FORM_NAME_ATTRIBUTE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::SetPaperFormName (WChar const* formNameIn)
    {
    properties.SetString (SHEET_DEF_PAPER_FORM_NAME_ATTRIBUTE, formNameIn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::GetPaperUnits (UnitDefinitionR paperUnitOut) const
    {
    paperUnitOut = properties.paperUnitDef;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::SetPaperUnits (UnitDefinitionCR paperUnitIn)
    {
    properties.paperUnitDef = paperUnitIn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::GetPaperMargins
(
double&            topMarginOut,
double&            leftMarginOut,
double&            bottomMarginOut,
double&            rightMarginOut
) const
    {
    topMarginOut = properties.topPaperMargin;
    leftMarginOut = properties.leftPaperMargin;
    bottomMarginOut = properties.bottomPaperMargin;
    rightMarginOut = properties.rightPaperMargin;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::SetPaperMargins
(
double              topMarginIn,
double              leftMarginIn,
double              bottomMarginIn,
double              rightMarginIn
)
    {
    properties.topPaperMargin = topMarginIn;
    properties.leftPaperMargin = leftMarginIn;
    properties.bottomPaperMargin = bottomMarginIn;
    properties.rightPaperMargin = rightMarginIn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
double          SheetDef::GetPlotScaleFactor () const
    {
    return properties.plotScale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       SheetDef::SetPlotScaleFactor (double scaleIn)
    {
    if (0.0 >= scaleIn)
        return ERROR;

    properties.plotScale = scaleIn;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::GetWindowsPrinterName (WString& windowsPrinterNameOut) const
    {
    properties.GetString (windowsPrinterNameOut, SHEET_DEF_WINDOWS_PRINTER_NAME_ATTRIBUTE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::SetWindowsPrinterName (WChar const* windowsPrinterNameIn)
    {
    properties.SetString (SHEET_DEF_WINDOWS_PRINTER_NAME_ATTRIBUTE, windowsPrinterNameIn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::GetPltFileName (WString& pltFileNameOut) const
    {
    properties.GetString (pltFileNameOut, SHEET_DEF_PLT_FILE_NAME_ATTRIBUTE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::SetPltFileName (WChar const* pltFileNameIn)
    {
    properties.SetString (SHEET_DEF_PLT_FILE_NAME_ATTRIBUTE, pltFileNameIn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Andrew.Edge     04/05
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::GetPlotStyleTableFileName (WString& pstFileNameOut) const
    {
    properties.GetString (pstFileNameOut, SHEET_DEF_PST_FILE_NAME_ATTRIBUTE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Andrew.Edge     04/05
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::SetPlotStyleTableFileName (WChar const* pstFileNameIn)
    {
    properties.SetString (SHEET_DEF_PST_FILE_NAME_ATTRIBUTE, pstFileNameIn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Andrew.Edge     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::GetPrintStyleName (WString& printStyleNameOut) const
    {
    properties.GetString (printStyleNameOut, SHEET_DEF_PRINT_STYLE_NAME_ATTRIBUTE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Andrew.Edge     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::SetPrintStyleName (WChar const* printStyleNameIn)
    {
    properties.SetString (SHEET_DEF_PRINT_STYLE_NAME_ATTRIBUTE, printStyleNameIn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 04/05
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 SheetDef::GetSheetNumber () const    
    { 
    return properties.sheetNumber;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 04/05
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::SetSheetNumber (UInt32 sheetNumberIn)
    {
    properties.sheetNumber = sheetNumberIn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   04/2005
+---------------+---------------+---------------+---------------+---------------+------*/
ElementId SheetDef::GetBorderAttachmentId () const
    {
    return properties.borderAttachmentId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 04/05
+---------------+---------------+---------------+---------------+---------------+------*/
void        SheetDef::SetBorderAttachmentId (ElementId borderAttachmentIdIn)
    {
    properties.borderAttachmentId = borderAttachmentIdIn;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   SheetDef linkage functions                                          |
|                                                                       |
+----------------------------------------------------------------------*/
#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    DeepakMalkan    10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       SheetDef::SheetProperties::AppendLinkage (DgnElementR elementIn) const
    {
    LinkageHeader*                   header;
    MSSheetPropertiesLinkageData*    data;
    byte* buf;
    size_t allocSize = sizeof (*header) + sizeof (*data) + m_extraLinkage.size();
    buf = (byte*) _alloca (allocSize);
    memset (buf, 0, allocSize);
    header = (LinkageHeader*)buf;
    data = (MSSheetPropertiesLinkageData*) (header + 1);
    /* Set up buf.header */
    header->primaryID = LINKAGEID_SheetProperties;
    header->user      = true;

    LinkageUtil::SetWords (header, LinkageUtil::CalculateSize ((int)allocSize) / sizeof (short));

    data->version = version;
    if (data->version < SHEET_PROPERTIES_LINKAGE_LATEST_VERSION)
        data->version = SHEET_PROPERTIES_LINKAGE_LATEST_VERSION;

    /* Set up strings of buf.data */
    data->isEnabled = isEnabled ? 1 : 0;
    data->reservedBits = reservedBits;
    data->reservedInt = reservedInt;
    data->origin = origin;
    data->rotation = rotation;
    data->width =  width;
    data->height =  height;
    data->color = color;
    data->reservedInt2 = reservedInt2;
    data->unitInfo = unitDef.ToUnitInfo();

    data->paperOrigin = paperOrigin;
    data->paperRotation = paperRotation;
    data->paperWidth = paperWidth;
    data->paperHeight = paperHeight;
    UnitInfo::ToStoredUnitInfo (data->paperUnitInfo, paperUnitDef.ToUnitInfo());
    data->topPaperMargin = topPaperMargin;
    data->leftPaperMargin = leftPaperMargin;
    data->bottomPaperMargin = bottomPaperMargin;
    data->rightPaperMargin = rightPaperMargin;

    data->plotScale = plotScale;

    /* Copy any extra data stored on "sheetPropertiesIn" */
    std::copy (m_extraLinkage.begin(), m_extraLinkage.end(), (byte*)(data + 1 ));

    return linkage_appendToElement (&elementIn, header, data, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    DeepakMalkan    10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       SheetDef::SheetProperties::SetLinkage (DgnElementR elementIn) const
    {
    if ((elementIn.GetSizeWords() - elementIn.GetAttributeOffset()) > 0 &&
        NULL != linkage_extractLinkageByIndex (NULL, &elementIn, LINKAGEID_SheetProperties, NULL, 0))
        {
        if (1 != linkage_deleteLinkageByIndex (&elementIn, LINKAGEID_SheetProperties, 0))
            return ERROR;
        }

    return AppendLinkage (elementIn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    DeepakMalkan    12/03
+---------------+---------------+---------------+---------------+---------------+------*/
void    SheetDef::SheetProperties::ExtractLinkageV0 (MSSheetPropertiesLinkageData const* sheetPropertiesLinkageDataIn)
    {
    reservedBits = sheetPropertiesLinkageDataIn->reservedBits;
    reservedInt = sheetPropertiesLinkageDataIn->reservedInt;
    origin = sheetPropertiesLinkageDataIn->origin;
    rotation = sheetPropertiesLinkageDataIn->rotation;
    width = sheetPropertiesLinkageDataIn->width;
    height = sheetPropertiesLinkageDataIn->height;
    color = sheetPropertiesLinkageDataIn->color;
    reservedInt2 = sheetPropertiesLinkageDataIn->reservedInt2;
    isEnabled = sheetPropertiesLinkageDataIn->isEnabled;
    unitDef.Init (sheetPropertiesLinkageDataIn->unitInfo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    DeepakMalkan    12/03
+---------------+---------------+---------------+---------------+---------------+------*/
void    SheetDef::SheetProperties::ExtractLinkageV1 (MSSheetPropertiesLinkageData const* sheetPropertiesLinkageDataIn)
    {
    ExtractLinkageV0 (sheetPropertiesLinkageDataIn);

    paperOrigin = sheetPropertiesLinkageDataIn->paperOrigin;
    paperRotation = sheetPropertiesLinkageDataIn->paperRotation;
    paperWidth = sheetPropertiesLinkageDataIn->paperWidth;
    paperHeight = sheetPropertiesLinkageDataIn->paperHeight;
    UnitInfo u;
    UnitInfo::FromStoredUnitInfo (u, sheetPropertiesLinkageDataIn->paperUnitInfo);
    paperUnitDef.Init (u);
    topPaperMargin = sheetPropertiesLinkageDataIn->topPaperMargin;
    leftPaperMargin = sheetPropertiesLinkageDataIn->leftPaperMargin;
    bottomPaperMargin = sheetPropertiesLinkageDataIn->bottomPaperMargin;
    rightPaperMargin = sheetPropertiesLinkageDataIn->rightPaperMargin;

    plotScale = sheetPropertiesLinkageDataIn->plotScale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    DeepakMalkan    10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SheetDef::SheetProperties::ExtractLinkage (DgnElementCR elementIn)
    {
    LinkageHeader *                 linkageHeader;
    MSSheetPropertiesLinkageData *  sheetPropertiesLinkageData;
    int                             baseSize;
    int                             actualSize;
    int                             extraLinkageSize;
    byte *                          extraLinkageBuffer;

    if (NULL == (linkageHeader = (LinkageHeader *) linkage_extractLinkageByIndex (NULL, &elementIn, LINKAGEID_SheetProperties, NULL, 0)))
        return DGNHANDLERS_STATUS_LinkageNotFound;
    sheetPropertiesLinkageData = (MSSheetPropertiesLinkageData *) (linkageHeader + 1);

    version = sheetPropertiesLinkageData->version;

    /* Set up fields from sheetPropertiesLinkage.data */
    switch (sheetPropertiesLinkageData->version)
        {
        case 0:
            ExtractLinkageV0 (sheetPropertiesLinkageData);
            break;
        case 1:
        default:
            ExtractLinkageV1 (sheetPropertiesLinkageData);
            break;
        }

    baseSize = sizeof (*linkageHeader) + sizeof (*sheetPropertiesLinkageData);
    actualSize = LinkageUtil::GetWords (linkageHeader) * 2;
    if (actualSize <= baseSize)
        return SUCCESS;

    /* Maintain any extra data that we find on the linkage */
    extraLinkageSize = actualSize - baseSize;

    extraLinkageBuffer = (byte*) linkageHeader;
    extraLinkageBuffer += baseSize;

    m_extraLinkage.resize (extraLinkageSize);
    std::copy (extraLinkageBuffer, extraLinkageBuffer + extraLinkageSize,  m_extraLinkage.begin());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 04/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   SheetDef::SheetProperties::ExtractLinkageEx (DgnElementCR elementIn)
    {
    // NOTICE: AppendLinkageEx is incorrect - it calculates the size incorrectly.
    // Being consistent here.
    // Also if you fix this - check AppendLinkageEx as well as determine what to do in 08.11*

    LinkageHeader *                     linkageHeader = NULL;
    MSSheetPropertiesLinkageDataEx *    sheetPropertiesLinkageDataEx = NULL;
    if (NULL == (linkageHeader = (LinkageHeader *) linkage_extractLinkageByIndex (NULL, &elementIn, LINKAGEID_SheetPropertiesEx, NULL, 0)))
        return DGNHANDLERS_STATUS_LinkageNotFound;

    sheetPropertiesLinkageDataEx    = (MSSheetPropertiesLinkageDataEx *)    (linkageHeader + 1);

    /* Set up fields from sheetPropertiesLinkageEx.data */
    sheetNumber = sheetPropertiesLinkageDataEx->sheetNumber;
    borderAttachmentId = sheetPropertiesLinkageDataEx->borderAttachmentId;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 04/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       SheetDef::SheetProperties::AppendLinkageEx (DgnElementR elementIn) const
    {
    MSSheetPropertiesLinkageEx    sheetPropertiesLinkageEx;
    int                           linkageSize;

    memset (&sheetPropertiesLinkageEx, 0, sizeof(sheetPropertiesLinkageEx));

    /* Set up sheetPropertiesLinkageEx.header */
    sheetPropertiesLinkageEx.header.primaryID = LINKAGEID_SheetPropertiesEx;
    sheetPropertiesLinkageEx.header.user      = true;

    // NOTICE: this is wrong - There is padding between linkage header and linkage data.
    // You cannot calculate the size based on this. Leaving the same as 08.11*
    // Also if you fix this - check ExtractLinkageEx as well as determine what to do in 08.11*
    linkageSize = LinkageUtil::CalculateSize (sizeof (sheetPropertiesLinkageEx));
    LinkageUtil::SetWords (&sheetPropertiesLinkageEx.header, linkageSize / sizeof (short));

    /* Set up strings of sheetPropertiesLinkage.data */
    sheetPropertiesLinkageEx.data.sheetNumber = sheetNumber;
    sheetPropertiesLinkageEx.data.borderAttachmentId = borderAttachmentId;

    return linkage_appendToElement (&elementIn, &sheetPropertiesLinkageEx.header, &sheetPropertiesLinkageEx.data, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 04/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       SheetDef::SheetProperties::SetLinkageEx (DgnElementR elementIn) const
    {
    if ((elementIn.GetSizeWords() - elementIn.GetAttributeOffset()) > 0 &&
        NULL != linkage_extractLinkageByIndex (NULL, &elementIn, LINKAGEID_SheetPropertiesEx, NULL, 0))
        {
        if (1 != linkage_deleteLinkageByIndex (&elementIn, LINKAGEID_SheetPropertiesEx, 0))
            return ERROR;
        }

    return AppendLinkageEx (elementIn);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void SheetDef::SheetProperties::SetString (UInt16 attrid, WChar const* str)
    {
    if (NULL != str && '\0' != *str)
        m_strings[attrid] = str;
    else
       m_strings.erase (attrid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/09
+---------------+---------------+---------------+---------------+---------------+------*/
WString const* SheetDef::SheetProperties::GetString (UInt16 attrid) const
    {
    std::map<UInt16,WString>::const_iterator i = m_strings.find (attrid);
    return i == m_strings.end()? NULL: &i->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void SheetDef::SheetProperties::GetString (WString& str, UInt16 attrid) const
    {
    WString const* s = GetString (attrid);
    if (NULL != s)
        str = *s;   
    }

/*---------------------------------------------------------------------------------**//**
* Return true is any sheet property to be stored on the sheet property linkage is set
* @bsimethod                                                    DeepakMalkan    09/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool SheetDef::SheetProperties::AnySet () const
    {
    if (0 != origin.x)
        return true;
    if (0 != origin.y)
        return true;
    if (0 != rotation)
        return true;
    if (0 != width)
        return true;
    if (0 != height)
        return true;
    if (INVALID_COLOR != color)
        return true;
    if (0 != paperOrigin.x)
        return true;
    if (0 != paperOrigin.y)
        return true;
    if (0 != paperRotation)
        return true;
    if (0 != paperWidth)
        return true;
    if (0 != paperHeight)
        return true;
    if (0 != topPaperMargin)
        return true;
    if (0 != leftPaperMargin)
        return true;
    if (0 != bottomPaperMargin)
        return true;
    if (0 != rightPaperMargin)
        return true;
    if (1.0 != plotScale)
        return true;
    if (0 != sheetNumber)
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/09
+---------------+---------------+---------------+---------------+---------------+------*/
SheetDef::SheetProperties::SheetProperties ()
    :
    version (SHEET_PROPERTIES_LINKAGE_LATEST_VERSION),
    isEnabled (true),
    reservedBits (0),
    rotation (0),
    width (0),
    height (0),
    color (INVALID_COLOR),
    reservedInt (0),
    reservedInt2 (0),
    paperRotation (0),
    paperWidth (0),
    paperHeight (0),
    topPaperMargin (0),
    leftPaperMargin (0),
    bottomPaperMargin (0),
    rightPaperMargin (0),
    plotScale (1.0),
    sheetNumber (0)
    {
    memset (&origin, 0, sizeof(origin));
    memset (&paperOrigin, 0, sizeof(paperOrigin));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/09
+---------------+---------------+---------------+---------------+---------------+------*/
SheetDef::SheetProperties::~SheetProperties ()
    {
    }

#ifdef DGN_IMPORTER_REORG_WIP
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   SheetDef::SheetProperties::FromElement (DgnElementCR elementIn)
    {
    StatusInt linkageStatus = ExtractLinkage (elementIn);
    StatusInt linkageExStatus = ExtractLinkageEx (elementIn);

    bool anyString = false;
    for (size_t iString = 0; iString < _countof(s_propertyStrings); ++iString)
        {
        WChar linkageString[MAX_LINKAGE_STRING_LENGTH];
        if (SUCCESS == mdlLinkage_extractNamedStringLinkageByIndex (linkageString, MAX_LINKAGE_STRING_LENGTH, &elementIn, s_propertyStrings[iString].stringLinkageId, 0))
            {
            SetString (s_propertyStrings[iString].attrid, linkageString);
            anyString = true;
            }
        }

    if (linkageStatus != SUCCESS && linkageExStatus != SUCCESS && !anyString)
        return ERROR;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SheetDef::SheetProperties::ToElement (DgnElementR elementIn) const
    {
    StatusInt   status;

    if (AnySet())
        {
        if (SUCCESS != (status = SetLinkage (elementIn)))
            return status;

        if (SUCCESS != (status = SetLinkageEx (elementIn)))
            return status;
        }

    for (size_t iString = 0; iString < _countof(s_propertyStrings); ++iString)
        {
        WString const* str = GetString (s_propertyStrings[iString].attrid);
        if (NULL != str)
            {
            status = mdlLinkage_setStringLinkage (&elementIn, s_propertyStrings[iString].stringLinkageId, str->c_str());
            if (SUCCESS != status)
                return status;
            }
        }

    return SUCCESS;
    }

#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
SheetDef::SheetDef ()
    {
    m_borderElementRef = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
SheetDef::~SheetDef ()
    {
    DELETE_AND_CLEAR (m_borderElementRef);
    }

#ifdef DGN_IMPORTER_REORG_WIP
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   SheetDef::FromElement (DgnElementCR elementIn)
    {
    return properties.FromElement (elementIn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    SheetDef::SaveProperties (DgnElementR elementIn) const
    {
    return properties.ToElement (elementIn);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
SheetDef::SheetProperties::SheetProperties (SheetDef::SheetProperties const& rhs)
    {
    CopyFrom (rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
SheetDef::SheetProperties& SheetDef::SheetProperties::CopyFrom (SheetDef::SheetProperties const& rhs)
    {
    origin              = rhs.origin;
    rotation            = rhs.rotation;
    width               = rhs.width;
    height              = rhs.height;
    color               = rhs.color;
    unitDef             = rhs.unitDef;
    isEnabled           = rhs.isEnabled;
    reservedBits        = rhs.reservedBits;

    paperOrigin         = rhs.paperOrigin;
    paperRotation       = rhs.paperRotation;
    paperWidth          = rhs.paperWidth;
    paperHeight         = rhs.paperHeight;
    paperUnitDef        = rhs.paperUnitDef;
    topPaperMargin      = rhs.topPaperMargin;
    leftPaperMargin     = rhs.leftPaperMargin;
    bottomPaperMargin   = rhs.bottomPaperMargin;
    rightPaperMargin    = rhs.rightPaperMargin;

    plotScale           = rhs.plotScale;

    sheetNumber         = rhs.sheetNumber;
    borderAttachmentId  = rhs.borderAttachmentId;

    m_strings           = rhs.m_strings;
    m_extraLinkage      = rhs.m_extraLinkage;

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
SheetDef::SheetProperties& SheetDef::SheetProperties::operator= (SheetDef::SheetProperties const& from)
    {
    if (this == &from)
        return *this;

    return CopyFrom (from);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
SheetDef& SheetDef::CopyFrom (SheetDef const& rhs)
    {
    properties          = rhs.properties;
    m_borderElementRef  = NULL;

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
SheetDef& SheetDef::operator= (SheetDef const& from)
    {
    if (this == &from)
        return *this;

    DELETE_AND_CLEAR (m_borderElementRef);

    return CopyFrom (from);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
SheetDef::SheetDef (SheetDef const& rhs)
    {
    CopyFrom (rhs);
    }
#ifdef DGN_IMPORTER_REORG_WIP

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/07
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     areDoublesDifferent
(
double          dist1,
double          dist2,
double          tolerance
)
    {
    return (fabs (dist1 - dist2) > tolerance);
    }

// this class makes sure that a temporary bit mask is always freed.
struct  TempBitMaskHolder
    {
    BitMaskP    m_bitMask;

    TempBitMaskHolder () { m_bitMask = NULL; }
    ~TempBitMaskHolder() { BitMask::FreeAndClear (&m_bitMask); }

    BitMaskP    SetBitMask (BitMaskP bitMask) { return m_bitMask = bitMask;}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan  05/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool  SheetDef::CompareData
(
BitMaskP            compareBitMaskOut,
SheetDef const&     rhs,
BitMaskCP           compareAttributesMaskIn,
double              distTol
)   const
    {
    bool                isEqual = true;

    if (this == &rhs)
        {
        if (NULL != compareBitMaskOut)
            compareBitMaskOut->SetAll ();
        return true;
        }

    BitMaskCP           compareAttributesMask = NULL;
    TempBitMaskHolder   holder;
    if (NULL == compareAttributesMaskIn)
        compareAttributesMask = holder.SetBitMask (BitMask::Create (true));
    else
        compareAttributesMask = compareAttributesMaskIn;

    if (compareAttributesMask->Test (SHEET_DEF_ORIGIN_ATTRIBUTE) &&
        (areDoublesDifferent (properties.origin.x, rhs.properties.origin.x, distTol) ||
         areDoublesDifferent (properties.origin.y, rhs.properties.origin.y, distTol)))
        RETURN_OR_SET_COMPARE_BITMASK (compareBitMaskOut, isEqual, SHEET_DEF_ORIGIN_ATTRIBUTE)

    if (compareAttributesMask->Test (SHEET_DEF_ROTATION_ATTRIBUTE) && areDoublesDifferent (properties.rotation, rhs.properties.rotation, distTol))
        RETURN_OR_SET_COMPARE_BITMASK (compareBitMaskOut, isEqual, SHEET_DEF_ROTATION_ATTRIBUTE)

    if (compareAttributesMask->Test (SHEET_DEF_WIDTH_ATTRIBUTE) && areDoublesDifferent (properties.width, rhs.properties.width, distTol))
        RETURN_OR_SET_COMPARE_BITMASK (compareBitMaskOut, isEqual, SHEET_DEF_WIDTH_ATTRIBUTE)

    if (compareAttributesMask->Test (SHEET_DEF_HEIGHT_ATTRIBUTE) && areDoublesDifferent (properties.height, rhs.properties.height, distTol))
        RETURN_OR_SET_COMPARE_BITMASK (compareBitMaskOut, isEqual, SHEET_DEF_HEIGHT_ATTRIBUTE)

    if (compareAttributesMask->Test (SHEET_DEF_COLOR_ATTRIBUTE) && properties.color != rhs.properties.color)
        RETURN_OR_SET_COMPARE_BITMASK (compareBitMaskOut, isEqual, SHEET_DEF_COLOR_ATTRIBUTE)

    if (compareAttributesMask->Test (SHEET_DEF_UNIT_ATTRIBUTE) && properties.unitDef.GetNumerator() != rhs.properties.unitDef.GetNumerator())
        RETURN_OR_SET_COMPARE_BITMASK (compareBitMaskOut, isEqual, SHEET_DEF_UNIT_ATTRIBUTE)
    if (compareAttributesMask->Test (SHEET_DEF_UNIT_ATTRIBUTE) && properties.unitDef.GetDenominator() != rhs.properties.unitDef.GetDenominator())
        RETURN_OR_SET_COMPARE_BITMASK (compareBitMaskOut, isEqual, SHEET_DEF_UNIT_ATTRIBUTE)
    if (compareAttributesMask->Test (SHEET_DEF_UNIT_ATTRIBUTE) && 0 != wcscmp (properties.unitDef.GetLabelCP(), rhs.properties.unitDef.GetLabelCP()))
        RETURN_OR_SET_COMPARE_BITMASK (compareBitMaskOut, isEqual, SHEET_DEF_UNIT_ATTRIBUTE)

    if (compareAttributesMask->Test (SHEET_DEF_ENABLED_ATTRIBUTE) && properties.isEnabled != rhs.properties.isEnabled)
        RETURN_OR_SET_COMPARE_BITMASK (compareBitMaskOut, isEqual, SHEET_DEF_ENABLED_ATTRIBUTE)

    if (compareAttributesMask->Test (SHEET_DEF_PAPER_ORIGIN_ATTRIBUTE) && areDoublesDifferent (properties.paperOrigin.x, rhs.properties.paperOrigin.x, distTol))
        RETURN_OR_SET_COMPARE_BITMASK (compareBitMaskOut, isEqual, SHEET_DEF_PAPER_ORIGIN_ATTRIBUTE)
    if (compareAttributesMask->Test (SHEET_DEF_PAPER_ORIGIN_ATTRIBUTE) && areDoublesDifferent (properties.paperOrigin.y, rhs.properties.paperOrigin.y, distTol))
        RETURN_OR_SET_COMPARE_BITMASK (compareBitMaskOut, isEqual, SHEET_DEF_PAPER_ORIGIN_ATTRIBUTE)

    if (compareAttributesMask->Test (SHEET_DEF_PAPER_ROTATION_ATTRIBUTE) && areDoublesDifferent (properties.paperRotation, rhs.properties.paperRotation, distTol))
        RETURN_OR_SET_COMPARE_BITMASK (compareBitMaskOut, isEqual, SHEET_DEF_PAPER_ROTATION_ATTRIBUTE)

    if (compareAttributesMask->Test (SHEET_DEF_PAPER_WIDTH_ATTRIBUTE) && areDoublesDifferent (properties.paperWidth, rhs.properties.paperWidth, distTol))
        RETURN_OR_SET_COMPARE_BITMASK (compareBitMaskOut, isEqual, SHEET_DEF_PAPER_WIDTH_ATTRIBUTE)

    if (compareAttributesMask->Test (SHEET_DEF_PAPER_HEIGHT_ATTRIBUTE) && areDoublesDifferent (properties.paperHeight, rhs.properties.paperHeight, distTol))
        RETURN_OR_SET_COMPARE_BITMASK (compareBitMaskOut, isEqual, SHEET_DEF_PAPER_HEIGHT_ATTRIBUTE)

    if (compareAttributesMask->Test (SHEET_DEF_PAPER_UNIT_ATTRIBUTE) && properties.paperUnitDef.GetNumerator() != rhs.properties.paperUnitDef.GetNumerator())
        RETURN_OR_SET_COMPARE_BITMASK (compareBitMaskOut, isEqual, SHEET_DEF_PAPER_UNIT_ATTRIBUTE)
    if (compareAttributesMask->Test (SHEET_DEF_PAPER_UNIT_ATTRIBUTE) && properties.paperUnitDef.GetDenominator() != rhs.properties.paperUnitDef.GetDenominator())
        RETURN_OR_SET_COMPARE_BITMASK (compareBitMaskOut, isEqual, SHEET_DEF_PAPER_UNIT_ATTRIBUTE)
    if (compareAttributesMask->Test (SHEET_DEF_PAPER_UNIT_ATTRIBUTE) && 0 != wcscmp (properties.paperUnitDef.GetLabelCP(), rhs.properties.paperUnitDef.GetLabelCP()))
        RETURN_OR_SET_COMPARE_BITMASK (compareBitMaskOut, isEqual, SHEET_DEF_PAPER_UNIT_ATTRIBUTE)

    if (compareAttributesMask->Test (SHEET_DEF_TOP_PAPER_MARGIN_ATTRIBUTE) && areDoublesDifferent (properties.topPaperMargin, rhs.properties.topPaperMargin, distTol))
        RETURN_OR_SET_COMPARE_BITMASK (compareBitMaskOut, isEqual, SHEET_DEF_TOP_PAPER_MARGIN_ATTRIBUTE)
    if (compareAttributesMask->Test (SHEET_DEF_LEFT_PAPER_MARGIN_ATTRIBUTE) && areDoublesDifferent (properties.leftPaperMargin, rhs.properties.leftPaperMargin, distTol))
        RETURN_OR_SET_COMPARE_BITMASK (compareBitMaskOut, isEqual, SHEET_DEF_LEFT_PAPER_MARGIN_ATTRIBUTE)
    if (compareAttributesMask->Test (SHEET_DEF_BOTTOM_PAPER_MARGIN_ATTRIBUTE) && areDoublesDifferent (properties.bottomPaperMargin, rhs.properties.bottomPaperMargin, distTol))
        RETURN_OR_SET_COMPARE_BITMASK (compareBitMaskOut, isEqual, SHEET_DEF_BOTTOM_PAPER_MARGIN_ATTRIBUTE)
    if (compareAttributesMask->Test (SHEET_DEF_RIGHT_PAPER_MARGIN_ATTRIBUTE) && areDoublesDifferent (properties.rightPaperMargin, rhs.properties.rightPaperMargin, distTol))
        RETURN_OR_SET_COMPARE_BITMASK (compareBitMaskOut, isEqual, SHEET_DEF_RIGHT_PAPER_MARGIN_ATTRIBUTE)

    if (compareAttributesMask->Test (SHEET_DEF_PLOT_SCALE_ATTRIBUTE) && properties.plotScale != rhs.properties.plotScale)
        RETURN_OR_SET_COMPARE_BITMASK (compareBitMaskOut, isEqual, SHEET_DEF_PLOT_SCALE_ATTRIBUTE)

    if (compareAttributesMask->Test (SHEET_DEF_SHEET_NUMBER_ATTRIBUTE) && properties.sheetNumber != rhs.properties.sheetNumber)
        RETURN_OR_SET_COMPARE_BITMASK (compareBitMaskOut, isEqual, SHEET_DEF_SHEET_NUMBER_ATTRIBUTE)

    if (compareAttributesMask->Test (SHEET_DEF_BORDER_ATTACHMENT_ID_ATTRIBUTE) && properties.borderAttachmentId != rhs.properties.borderAttachmentId)
        RETURN_OR_SET_COMPARE_BITMASK (compareBitMaskOut, isEqual, SHEET_DEF_BORDER_ATTACHMENT_ID_ATTRIBUTE)

    for (size_t iString = 0; iString < _countof(s_propertyStrings); ++iString)
        {
        switch (s_propertyStrings[iString].attrid)
            {
            case SHEET_DEF_FORM_NAME_ATTRIBUTE:
                if (! compareAttributesMask->Test (SHEET_DEF_FORM_NAME_ATTRIBUTE)) continue;
                break;

            case SHEET_DEF_PAPER_FORM_NAME_ATTRIBUTE:
                if (! compareAttributesMask->Test (SHEET_DEF_PAPER_FORM_NAME_ATTRIBUTE)) continue;
                break;

            case SHEET_DEF_WINDOWS_PRINTER_NAME_ATTRIBUTE:
                if (! compareAttributesMask->Test (SHEET_DEF_WINDOWS_PRINTER_NAME_ATTRIBUTE)) continue;
                break;

            case SHEET_DEF_PLT_FILE_NAME_ATTRIBUTE:
                if (! compareAttributesMask->Test (SHEET_DEF_PLT_FILE_NAME_ATTRIBUTE)) continue;
                break;

            case SHEET_DEF_PST_FILE_NAME_ATTRIBUTE:
                if (! compareAttributesMask->Test (SHEET_DEF_PST_FILE_NAME_ATTRIBUTE)) continue;
                break;

            case SHEET_DEF_SHEET_NAME_ATTRIBUTE:
                if (! compareAttributesMask->Test (SHEET_DEF_SHEET_NAME_ATTRIBUTE)) continue;
                break;

            case SHEET_DEF_PRINT_STYLE_NAME_ATTRIBUTE:
                if (! compareAttributesMask->Test (SHEET_DEF_PRINT_STYLE_NAME_ATTRIBUTE)) continue;
                break;
            }

        WString const* str    =     properties.GetString (s_propertyStrings[iString].attrid);
        WString const* rhsstr = rhs.properties.GetString (s_propertyStrings[iString].attrid);
        if (NULL != str || NULL != rhsstr)
            {
            if (NULL == str || NULL == rhsstr || *str != *rhsstr)
                RETURN_OR_SET_COMPARE_BITMASK (compareBitMaskOut, isEqual, static_cast<UInt32> (iString))
            }
        }

    return isEqual;
    }
#endif

