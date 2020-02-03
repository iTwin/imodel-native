/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/HRFCapabilitiesExporter/HTMLExporter.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

// Constants
static const int        MAX_LINE_LENGTH         = 1024;    /* Used to read template files */
static const WString    HMR_CURRENT_VERSION     = _T("2");
static const TCHAR      TEMPLATE_TAG_HEADER[13] = _T("###HEADER###");
static const TCHAR      TEMPLATE_TAG_CONTENT[14]= _T("###CONTENT###");
static const TCHAR      TEMPLATE_TAG_FOOTER[13] = _T("###FOOTER###");
static const WString    FRAMESET_FILENAME       = _T("index.html");
static const WString    HEADER_FILENAME         = _T("header.html");
static const WString    TOC_FILENAME            = _T("toc.html");
static const WString    DEFAULT_CONTENT_FILENAME= _T("content.html");
static const WString    CONTENT_PATH            = _T("formats");
static const WString    FOOTER_FILENAME             = _T("footer.html");

/*=================================================================================**//**
* @bsiclass                                                     JeanLalande    06/2010
+===============+===============+===============+===============+===============+======*/
class HTMLExporter
    {
    private:
        // Printing helpers
        void ConvertBlockObjectToString         (const BlockObject& blockObject, 
                                                 WString& output);
        void ConvertAccessModesToString         (const BooleanMap& accessModes, 
                                                 WString& output);
        void ConvertDownSamplingMethodToString  (HRFDownSamplingMethod::DownSamplingMethod method, 
                                                 WString& output);
        void GetColorSpaceCode                  (long const colorSpaceID, 
                                                 WString& output);
        void GetCompressionCode                 (long const codecID, 
                                                 WString& output);
        void GetOrganizationCode                (BlockObject const& blockType, 
                                                 WString& output);
        void GetResolutionCode                  (BlockObject const& blockType, 
                                                 WString& output);
        void GetDownSamplingCode                (long const downSamplingMethodID, 
                                                 WString& output);
        void GetScanlineOrientation             (ImageFormat const& image, 
                                                 WString& output);
        void BuildProfileID                     (ImageFormat const&            image,
                                                 ColorSpaceObject const&       colorSpace,
                                                 CodecObject const&            codecObject,
                                                 BlockObject const&            blockType,
                                                 DownSamplingObject const&     downSamplingMethod,
                                                 WString&                      profileID);
        
        // Printing methods
        void PrintFrameset                      (otstream& out);
        void PrintDefaultContent                (otstream& out);
        void PrintGenericHeader                 (otstream& out);
        void PrintGenericFooter                 (otstream& out);
        void PrintTableOfContent                (ImageFormatMap const& imageFormats, 
                                                 otstream& out, 
                                                 bool usingFrames);
        void PrintContentHeader                 (otstream& out);
        void PrintContentFooter                 (otstream& out);
        void PrintContent                       (ImageFormatMap const& imageFormats, otstream& out);
        void PrintContent                       (ImageFormatMap const& imageFormats, WString pathToDirectory);
        void PrintHeader                        (otstream& out);
        void PrintFooter                        (otstream& out);
        void PrintImageFormat                   (ImageFormatMap::const_iterator& itr, 
                                                 otstream& out, 
                                                 bool usingFrames);
        void PrintPixelTypeCapabilities         (otstream& out, ImageFormatMap::const_iterator& itr);
        void PrintTransfoModel                  (otstream& out, ImageFormatMap::const_iterator& itr);
        void PrintImageFormatProperties         (otstream& out, ImageFormatMap::const_iterator& itr);
        void PrintTags                          (otstream& out, ImageFormatMap::const_iterator& itr);
        void PrintGeocodingCapabilities         (otstream& out, ImageFormatMap::const_iterator& itr);        
                
    public:
        HTMLExporter(void);
        ~HTMLExporter(void);
        
        void Export(ImageFormatMap const& imageFormats, otstream& out);
        void Export(ImageFormatMap const& imageFormats, itstream& templateInput, otstream& output);
        void Export(ImageFormatMap const& imageFormats, const WString& exportPath);
        

    };
