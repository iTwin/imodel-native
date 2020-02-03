/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/HRFCapabilitiesExporter/CreatorParser.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

class CreatorParser
    {
    private:
        ImageFormatMap m_fileFormats;
        
        // Parsing helpers
        void SetAccessMode                      (BooleanMap& accessList, 
                                                 const HFCPtr<HRFCapability>& pi_rpCapability);
        
        // Parsing methods
        void ParseImageFormatCapabilities       (HRFRasterFileCreator& rasterCreator, 
                                                 ImageFormat& imageFormat);
        void ParseTransfoModelCapabilities      (HRFRasterFileCreator& rasterCreator, 
                                                 ImageFormat& imageFormat);
        void ParsePixelTypeCapabilities         (HRFRasterFileCreator& rasterCreator, 
                                                 ImageFormat& imageFormat);
        void ParseTagCapabilities               (HRFRasterFileCreator& rasterCreator, 
                                                 ImageFormat& imageFormat);
        void ParseGeocodingCapabilities         (HRFRasterFileCreator& rasterCreator, 
                                                 ImageFormat& imageFormat);
        void ParseBlockTypeCapabilities         (HRFRasterFileCreator& rasterCreator, 
                                                 const HFCPtr<HRFBlockCapability>& pBlockCapability, 
                                                 BlockObject& BlockObject);

        void ConvertDownSamplingMethodToString  (HRFDownSamplingMethod::DownSamplingMethod method, 
                                                 WString& output);

    public:
        CreatorParser(void);
        ~CreatorParser(void);
        
        void ScanFileCreator                    (HRFRasterFileCreator& rasterCreator);
        ImageFormatMap const& GetImageFormats   (void);
    };
