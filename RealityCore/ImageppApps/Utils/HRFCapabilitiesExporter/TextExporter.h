/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/HRFCapabilitiesExporter/TextExporter.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*=================================================================================**//**
* @bsiclass                                                     JeanLalande    06/2010
+===============+===============+===============+===============+===============+======*/
class TextExporter
    {
    private:
        otstream& m_out;
        HRFRasterFileCreator* m_creator;
        
        TextExporter():m_out(_tcout) {};
        
        void PrintElementHeader();
        void PrintElementFooter();
        
        // Print specific capability
        void PrintAccessModeCapability(HFCPtr<HRFCapability>& pi_rpCapability);
        void PrintPixelTypeCapability(HFCPtr<HRFPixelTypeCapability>& pi_rpCapability);
        void PrintInterlaceCapability(HFCPtr<HRFInterlaceCapability>& pi_rpCapability);
        void PrintScanlineOrientationCapability(HFCPtr<HRFScanlineOrientationCapability>& pi_rpCapability);
        void PrintInterleaveCapability(HFCPtr<HRFInterleaveCapability>& pi_rpCapability);
        void PrintMultiResolutionCapability(HFCPtr<HRFMultiResolutionCapability>& pi_rpCapability);
        void PrintSingleResolutionCapability(HFCPtr<HRFSingleResolutionCapability>& pi_rpCapability);
        void PrintBlockCapability(HFCPtr<HRFBlockCapability>& pi_rpCapability);
        void PrintLineCapability(HFCPtr<HRFLineCapability>& pi_rpCapability);
        void PrintStripCapability(HFCPtr<HRFStripCapability>& pi_rpCapability);
        void PrintTileCapability(HFCPtr<HRFTileCapability>& pi_rpCapability);
        void PrintImageCapability(HFCPtr<HRFImageCapability>& pi_rpCapability);
        void PrintFilterCapability(HFCPtr<HRFFilterCapability>& pi_rpCapability);
        void PrintClipShapeCapability(HFCPtr<HRFClipShapeCapability>& pi_rpCapability);
        void PrintTransfoModelCapability(HFCPtr<HRFTransfoModelCapability>& pi_rpCapability);
        void PrintHistogramCapability(HFCPtr<HRFHistogramCapability>& pi_rpCapability);
        void PrintThumbnailCapability(HFCPtr<HRFThumbnailCapability>& pi_rpCapability);
        void PrintRepresentativePaletteCapability(HFCPtr<HRFRepresentativePaletteCapability>& pi_rpCapability);
        void PrintMultiPageCapability(HFCPtr<HRFMultiPageCapability>& pi_rpCapability);
        void PrintTagCapability(HFCPtr<HRFTagCapability>& pi_rpCapability);
        void PrintBlocksDataFlagCapability(HFCPtr<HRFBlocksDataFlagCapability>& pi_rpCapability);
        void PrintEmbedingCapability(HFCPtr<HRFEmbedingCapability>& pi_rpCapability);
        void PrintStillImageCapability(HFCPtr<HRFStillImageCapability>& pi_rpCapability);
        void PrintAnimationCapability(HFCPtr<HRFAnimationCapability>& pi_rpCapability);
        void PrintGeocodingCapability(HFCPtr<HRFGeocodingCapability>& pi_rpCapability);

        // Conversion to string
        WString ConvertPixelTypeToString(HCLASS_ID pi_ClassKey);
        WString ConvertCodecToString(HCLASS_ID pi_ClassKey);
        WString ConvertFilterToString(HCLASS_ID pi_ClassKey);
        WString ConvertTransfoModelToString(HCLASS_ID pi_ClassKey);

        
    public:
        TextExporter(otstream& out);
        ~TextExporter(void);
        
        void SetRasterCreator(HRFRasterFileCreator& rasterCreator);
        void PrintHeader(void);
        void PrintElement(void);
        void PrintFooter(void);
    };
