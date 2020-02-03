/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/HRFCapabilitiesExporter/TextExporter.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"

#include <Imagepp/all/h/HRFFileFormats.h>
#include <Imagepp/all/h/Imagepplib.h>

IMPLEMENT_DEFAULT_IMAGEPP_LIBHOST(MyImageppLibHost)

#include "TextExporter.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
TextExporter::TextExporter(otstream& out)
: m_out(out)
{
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
TextExporter::~TextExporter(void)
{
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void TextExporter::SetRasterCreator(HRFRasterFileCreator& rasterCreator)
{
    m_creator = &rasterCreator;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
void TextExporter::PrintHeader(void)
{
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
void TextExporter::PrintFooter(void)
{
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
void TextExporter::PrintElementHeader(void)
{
    m_out << _T("\n//============================================================================");
    WString labelW(m_creator->GetLabel().c_str(), BentleyCharEncoding::Utf8);
    m_out << _T("\n//  ") << labelW.c_str();
    WString extensionsW(m_creator->GetExtensions().c_str(), BentleyCharEncoding::Utf8);
    m_out << _T(" (Extensions: ") << extensionsW.c_str() << _T(")");
    m_out << _T("\n//----------------------------------------------------------------------------\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
void TextExporter::PrintElementFooter(void)
{
    m_out << _T("\n\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void TextExporter::PrintElement()
{
    PrintElementHeader();
   
    HFCPtr<HRFRasterFileCapabilities> capabilities = m_creator->GetCapabilities();
    
    for (uint32_t index=0; index < capabilities->CountCapabilities(); index++)
    {
        HFCPtr<HRFCapability> pCapability = capabilities->GetCapability(index);

        if (pCapability->GetClassID() == HRFPixelTypeCapability::CLASS_ID)
            PrintPixelTypeCapability(((HFCPtr<HRFPixelTypeCapability>&)pCapability));
        else
        if (pCapability->GetClassID() == HRFInterlaceCapability::CLASS_ID)
            PrintInterlaceCapability(((HFCPtr<HRFInterlaceCapability>&)pCapability));
        else
        if (pCapability->GetClassID() == HRFScanlineOrientationCapability::CLASS_ID)
            PrintScanlineOrientationCapability(((HFCPtr<HRFScanlineOrientationCapability>&)pCapability));
        else
        if (pCapability->GetClassID() == HRFInterleaveCapability::CLASS_ID)
            PrintInterleaveCapability(((HFCPtr<HRFInterleaveCapability>&)pCapability));
        else
        if (pCapability->GetClassID() == HRFMultiResolutionCapability::CLASS_ID)
            PrintMultiResolutionCapability(((HFCPtr<HRFMultiResolutionCapability>&)pCapability));
        else
        if (pCapability->GetClassID() == HRFSingleResolutionCapability::CLASS_ID)
            PrintSingleResolutionCapability(((HFCPtr<HRFSingleResolutionCapability>&)pCapability));
        else
        if (pCapability->GetClassID() == HRFLineCapability::CLASS_ID)
            PrintLineCapability(((HFCPtr<HRFLineCapability>&)pCapability));
        else
        if (pCapability->GetClassID() == HRFStripCapability::CLASS_ID)
            PrintStripCapability(((HFCPtr<HRFStripCapability>&)pCapability));
        else
        if (pCapability->GetClassID() == HRFTileCapability::CLASS_ID)
            PrintTileCapability(((HFCPtr<HRFTileCapability>&)pCapability));
        else
        if (pCapability->GetClassID() == HRFImageCapability::CLASS_ID)
            PrintImageCapability(((HFCPtr<HRFImageCapability>&)pCapability));
        else
        if (pCapability->GetClassID() == HRFFilterCapability::CLASS_ID)
            PrintFilterCapability(((HFCPtr<HRFFilterCapability>&)pCapability));
        else
        if (pCapability->GetClassID() == HRFClipShapeCapability::CLASS_ID)
            PrintClipShapeCapability(((HFCPtr<HRFClipShapeCapability>&)pCapability));
        else
        if (pCapability->GetClassID() == HRFTransfoModelCapability::CLASS_ID)
            PrintTransfoModelCapability(((HFCPtr<HRFTransfoModelCapability>&)pCapability));
        else
        if (pCapability->GetClassID() == HRFHistogramCapability::CLASS_ID)
            PrintHistogramCapability(((HFCPtr<HRFHistogramCapability>&)pCapability));
        else
        if (pCapability->GetClassID() == HRFThumbnailCapability::CLASS_ID)
            PrintThumbnailCapability(((HFCPtr<HRFThumbnailCapability>&)pCapability));
        else
        if (pCapability->GetClassID() == HRFRepresentativePaletteCapability::CLASS_ID)
            PrintRepresentativePaletteCapability(((HFCPtr<HRFRepresentativePaletteCapability>&)pCapability));
        else
        if (pCapability->GetClassID() == HRFMultiPageCapability::CLASS_ID)
            PrintMultiPageCapability(((HFCPtr<HRFMultiPageCapability>&)pCapability));
        else
        if (pCapability->GetClassID() == HRFTagCapability::CLASS_ID)
            PrintTagCapability(((HFCPtr<HRFTagCapability>&)pCapability));
        else
        if (pCapability->GetClassID() == HRFBlocksDataFlagCapability::CLASS_ID)
            PrintBlocksDataFlagCapability(((HFCPtr<HRFBlocksDataFlagCapability>&)pCapability));
        else
        if (pCapability->GetClassID() == HRFEmbedingCapability::CLASS_ID)
            PrintEmbedingCapability(((HFCPtr<HRFEmbedingCapability>&)pCapability));
        else
        if (pCapability->GetClassID() == HRFStillImageCapability::CLASS_ID)
            PrintStillImageCapability(((HFCPtr<HRFStillImageCapability>&)pCapability));
        else
        if (pCapability->GetClassID() == HRFAnimationCapability::CLASS_ID)
            PrintAnimationCapability(((HFCPtr<HRFAnimationCapability>&)pCapability));
        else
        if (pCapability->GetClassID() == HRFGeocodingCapability::CLASS_ID)
            PrintGeocodingCapability(((HFCPtr<HRFGeocodingCapability>&)pCapability));
    }
    
    PrintElementFooter();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void TextExporter::PrintAccessModeCapability(HFCPtr<HRFCapability>& pi_rpCapability)
{
    // Access Mode information
    WString AccessStr;
    AccessStr += _T("(");
    if (pi_rpCapability->GetAccessMode().m_HasReadAccess)
        AccessStr += _T("Read");
    if (pi_rpCapability->GetAccessMode().m_HasWriteAccess)
        AccessStr += _T(" Write");
        

    AccessStr += _T(")");
    m_out << AccessStr.c_str();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void TextExporter::PrintPixelTypeCapability(HFCPtr<HRFPixelTypeCapability>& pi_rpCapability)
{
    m_out << _T("\nColor Space: ") << ConvertPixelTypeToString(pi_rpCapability->GetPixelTypeClassID()).c_str() << _T("  ");

    PrintAccessModeCapability(((HFCPtr<HRFCapability>&)pi_rpCapability));

    m_out << _T("\nCompression:");
    for (uint32_t index=0; index < pi_rpCapability->CountCodecs(); index++)
    {
        if (index == 0)
        {
            m_out << _T(" ") << ConvertCodecToString(pi_rpCapability->GetCodecCapabilityByIndex(index)->GetCodecClassID()).c_str();
        }
        else
            m_out << _T(", ") << ConvertCodecToString(pi_rpCapability->GetCodecCapabilityByIndex(index)->GetCodecClassID()).c_str();

    }
    m_out << _T("\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void TextExporter::PrintInterlaceCapability(HFCPtr<HRFInterlaceCapability>& pi_rpCapability)
{
    m_out << _T("\nInterlace\n");
    PrintAccessModeCapability(((HFCPtr<HRFCapability>&)pi_rpCapability));
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void TextExporter::PrintScanlineOrientationCapability(HFCPtr<HRFScanlineOrientationCapability>& pi_rpCapability)
{
    // ScanlineOrientation
    m_out << _T("\nScanline Orientation: ");
    m_out << WString(HUTClassIDDescriptor::GetInstance()->GetClassLabelSLO(pi_rpCapability->GetScanlineOrientation()).c_str(), BentleyCharEncoding::Utf8).c_str();
    m_out << _T("  ");

    PrintAccessModeCapability(((HFCPtr<HRFCapability>&)pi_rpCapability));
    m_out << _T("\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void TextExporter::PrintInterleaveCapability(HFCPtr<HRFInterleaveCapability>& pi_rpCapability)
{
    // Interleave Type
    if (pi_rpCapability->GetInterleaveType() == HRFInterleaveType::PIXEL)
        m_out << _T("\nInterleave Type: Pixel  ");
    else
    if (pi_rpCapability->GetInterleaveType() == HRFInterleaveType::LINE)
        m_out << _T("\nInterleave Type: Line  ");
    else
    if (pi_rpCapability->GetInterleaveType() == HRFInterleaveType::PLANE)
        m_out << _T("\nInterleave Type: Plane  ");

    PrintAccessModeCapability(((HFCPtr<HRFCapability>&)pi_rpCapability));
    m_out << _T("\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void TextExporter::PrintMultiResolutionCapability(HFCPtr<HRFMultiResolutionCapability>& pi_rpCapability)
{
    m_out << _T("\nImage encoding: multi-resolution  ");
    PrintAccessModeCapability(((HFCPtr<HRFCapability>&)pi_rpCapability));
    m_out << _T("\n");

    m_out << _T("SmallestResWidth: ") << pi_rpCapability->GetSmallestResWidth() << _T("\n");
    m_out << _T("SmallestResHeight: ") << pi_rpCapability->GetSmallestResHeight() << _T("\n");
    
    if (pi_rpCapability->IsSinglePixelType())
        m_out << _T("Single color space  \n");
    else
        m_out << _T("Multiple color space\n");

    if (pi_rpCapability->IsSingleBlockType())
        m_out << _T("Single block type\n");
    else
        m_out << _T("Multiple block type\n");

    if (pi_rpCapability->IsArbitaryXRatio())
        m_out << _T("Arbitary X ratio\n");
    else
        m_out << _T("Factor X Ratio \n");

    if (pi_rpCapability->IsArbitaryYRatio())
        m_out << _T("Arbitary Y ratio\n");
    else
        m_out << _T("Factor Y Ratio \n");
    
    if (pi_rpCapability->IsXYRatioLocked())
        m_out << _T("XY ratio locked\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void TextExporter::PrintSingleResolutionCapability(HFCPtr<HRFSingleResolutionCapability>& pi_rpCapability)
{
    m_out << _T("\nImage encoding: Standard ");
    PrintAccessModeCapability(((HFCPtr<HRFCapability>&)pi_rpCapability));
    m_out << _T("\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void TextExporter::PrintBlockCapability(HFCPtr<HRFBlockCapability>& pi_rpCapability)
{
    PrintAccessModeCapability(((HFCPtr<HRFCapability>&)pi_rpCapability));
    m_out << _T("\n");
    m_out << _T("MaxSizeInBytes: ") << pi_rpCapability->GetMaxSizeInBytes() << _T("\n");

    // Storage Access
    if (pi_rpCapability->GetBlockAccess() == HRFBlockAccess::RANDOM)
        m_out << _T("Block Access: Random");
    else
    if (pi_rpCapability->GetBlockAccess() == HRFBlockAccess::SEQUENTIAL)
        m_out << _T("Block Access: Sequential");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void TextExporter::PrintLineCapability(HFCPtr<HRFLineCapability>& pi_rpCapability)
{
    m_out << _T("\n\nBlock type: line  ");
    PrintBlockCapability(((HFCPtr<HRFBlockCapability>&)pi_rpCapability));
    m_out << _T("\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void TextExporter::PrintStripCapability(HFCPtr<HRFStripCapability>& pi_rpCapability)
{
    m_out << _T("\n\nBlock type: strip  ");
    PrintBlockCapability(((HFCPtr<HRFBlockCapability>&)pi_rpCapability));
    m_out << _T("\n");

    m_out << _T("Minimum height: ") << pi_rpCapability->GetMinHeight() << _T("\n");
    m_out << _T("Maximum height: ") << pi_rpCapability->GetMaxHeight() << _T("\n");
    m_out << _T("Height increment: ") << pi_rpCapability->GetHeightIncrement() << _T("\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void TextExporter::PrintTileCapability(HFCPtr<HRFTileCapability>& pi_rpCapability)
{
    m_out << _T("\n\nBlock type: tile  ");
    PrintBlockCapability(((HFCPtr<HRFBlockCapability>&)pi_rpCapability));
    m_out << _T("\n");

    m_out << _T("Minimum width: ") << pi_rpCapability->GetMinWidth() << _T("\n");
    m_out << _T("Maximum width: ") << pi_rpCapability->GetMaxWidth() << _T("\n");
    m_out << _T("Width increment: ") << pi_rpCapability->GetWidthIncrement() << _T("\n");

    m_out << _T("Minimum height: ") << pi_rpCapability->GetMinHeight() << _T("\n");
    m_out << _T("Maximum height: ") << pi_rpCapability->GetMaxHeight() << _T("\n");
    m_out << _T("Height increment: ") << pi_rpCapability->GetHeightIncrement() << _T("\n");

    if (pi_rpCapability->IsSquare())
        m_out << _T("Square tiles\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void TextExporter::PrintImageCapability(HFCPtr<HRFImageCapability>& pi_rpCapability)
{
    m_out << _T("\n\nBlock type: image  ");
    PrintBlockCapability(((HFCPtr<HRFBlockCapability>&)pi_rpCapability));
    m_out << _T("\n");

    m_out << _T("Minimum width: ") << pi_rpCapability->GetMinWidth() << _T("\n");
    m_out << _T("Maximum width: ") << pi_rpCapability->GetMaxWidth() << _T("\n");

    m_out << _T("Minimum height: ") << pi_rpCapability->GetMinHeight() << _T("\n");
    m_out << _T("Maximum height: ") << pi_rpCapability->GetMaxHeight() << _T("\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void TextExporter::PrintFilterCapability(HFCPtr<HRFFilterCapability>& pi_rpCapability)
{
    m_out << _T("\nFilter: ") << ConvertFilterToString(pi_rpCapability->GetFilter()).c_str() << _T("  ");
    PrintAccessModeCapability(((HFCPtr<HRFCapability>&)pi_rpCapability));
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void TextExporter::PrintClipShapeCapability(HFCPtr<HRFClipShapeCapability>& pi_rpCapability)
{
    if (pi_rpCapability->GetCoordinateType() == HRFCoordinateType::LOGICAL)
        m_out << _T("\nShape with logical coordinate  ");
    else
        m_out << _T("\nShape with physical coordinate  ");
  
    PrintAccessModeCapability(((HFCPtr<HRFCapability>&)pi_rpCapability));
    m_out << _T("\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void TextExporter::PrintTransfoModelCapability(HFCPtr<HRFTransfoModelCapability>& pi_rpCapability)
{
    m_out << _T("\nTransformation model: ") << ConvertTransfoModelToString(pi_rpCapability->GetTransfoModelClassKey()).c_str() << _T("  ");
    PrintAccessModeCapability(((HFCPtr<HRFCapability>&)pi_rpCapability));
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void TextExporter::PrintHistogramCapability(HFCPtr<HRFHistogramCapability>& pi_rpCapability)
{
    m_out << _T("\nHistogram  ");
    PrintAccessModeCapability(((HFCPtr<HRFCapability>&)pi_rpCapability));
    m_out << _T("\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void TextExporter::PrintThumbnailCapability(HFCPtr<HRFThumbnailCapability>& pi_rpCapability)
{
    m_out << _T("\nThumbnail ");
#if 0 
        void        AddPixelType(HPMClassKey pi_PixelType);
        uint32_t      CountCodecs() const;
        uint32_t      GetMaxHeight() const;
        uint32_t      GetMaxWidth() const;
        HPMClassKey GetPixelType(uint32_t pi_Index) const;
#endif 

    PrintAccessModeCapability(((HFCPtr<HRFCapability>&)pi_rpCapability));
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void TextExporter::PrintRepresentativePaletteCapability(HFCPtr<HRFRepresentativePaletteCapability>& pi_rpCapability)
{
    m_out << _T("\nRepresentative palette  ");
    PrintAccessModeCapability(((HFCPtr<HRFCapability>&)pi_rpCapability));
    m_out << _T("\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void TextExporter::PrintMultiPageCapability(HFCPtr<HRFMultiPageCapability>& pi_rpCapability)
{
    m_out << _T("\nMultiPage  ");
    PrintAccessModeCapability(((HFCPtr<HRFCapability>&)pi_rpCapability));
    m_out << _T("\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void TextExporter::PrintTagCapability(HFCPtr<HRFTagCapability>& pi_rpCapability)
{   
    m_out << "\nTag: " << WString(pi_rpCapability->GetTag()->GetName().c_str(), BentleyCharEncoding::Utf8).c_str();
    PrintAccessModeCapability(((HFCPtr<HRFCapability>&)pi_rpCapability));
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void TextExporter::PrintBlocksDataFlagCapability(HFCPtr<HRFBlocksDataFlagCapability>& pi_rpCapability)
{
    m_out << _T("\nBlocks data flag  ");
    PrintAccessModeCapability(((HFCPtr<HRFCapability>&)pi_rpCapability));
    m_out << _T("\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void TextExporter::PrintEmbedingCapability(HFCPtr<HRFEmbedingCapability>& pi_rpCapability)
{
    m_out << _T("\nEmbedding  ");
    PrintAccessModeCapability(((HFCPtr<HRFCapability>&)pi_rpCapability));
    m_out << _T("\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void TextExporter::PrintStillImageCapability(HFCPtr<HRFStillImageCapability>& pi_rpCapability)
{
    m_out << _T("\nStill image  ");
    PrintAccessModeCapability(((HFCPtr<HRFCapability>&)pi_rpCapability));
    m_out << _T("\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void TextExporter::PrintAnimationCapability(HFCPtr<HRFAnimationCapability>& pi_rpCapability)
{
    m_out << _T("\nAnimation  ");
    PrintAccessModeCapability(((HFCPtr<HRFCapability>&)pi_rpCapability));
    m_out << _T("\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Jonathan.Bernier      01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void TextExporter::PrintGeocodingCapability(HFCPtr<HRFGeocodingCapability>& pi_rpCapability)
{
    m_out << _T("\nGeocoding  ");
    PrintAccessModeCapability(((HFCPtr<HRFCapability>&)pi_rpCapability));
    m_out << _T("\n");
}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString TextExporter::ConvertPixelTypeToString(HCLASS_ID pi_ClassKey)
{
    
    HCLASS_ID pixelTypeID = pi_ClassKey;
    WString Result(HUTClassIDDescriptor::GetInstance()->GetClassLabelPixelType(pixelTypeID).c_str(), BentleyCharEncoding::Utf8);

    return Result;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString TextExporter::ConvertCodecToString(HCLASS_ID pi_ClassKey)
{
    
    HCLASS_ID codecID = pi_ClassKey;
    WString Result(HUTClassIDDescriptor::GetInstance()->GetClassLabelCodec(codecID).c_str(), BentleyCharEncoding::Utf8);

    return Result;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString TextExporter::ConvertFilterToString(HCLASS_ID pi_ClassKey)
{
    HCLASS_ID filterID = pi_ClassKey;
    WString Result(HUTClassIDDescriptor::GetInstance()->GetClassLabelFilter(filterID).c_str(), BentleyCharEncoding::Utf8);

    return Result;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString TextExporter::ConvertTransfoModelToString(HCLASS_ID pi_ClassKey)
{
    HCLASS_ID transfoModelID = pi_ClassKey;
    WString Result(HUTClassIDDescriptor::GetInstance()->GetClassLabelTransfoModel(transfoModelID).c_str(), BentleyCharEncoding::Utf8);

    return Result;
}



