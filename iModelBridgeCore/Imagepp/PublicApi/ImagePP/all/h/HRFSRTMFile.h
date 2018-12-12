//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFSRTMFile.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFSRTMFile
//-----------------------------------------------------------------------------
// This class describes an SRTM File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCURL.h"
#include "HFCMacros.h"
#include "HFCBinStream.h"
#include "HFCAccessMode.h"

#include "HRFMacros.h"
#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"


BEGIN_IMAGEPP_NAMESPACE
class HRFSRTMCapabilities : public HRFRasterFileCapabilities
    {
    public:
        HRFSRTMCapabilities();
    };

class HRFSRTMFile : public HRFRasterFile
    {
    public:

        //Possible file sizes for SRTM files
        static const uint32_t SRTM1_SIZE;
        static const uint32_t SRTM3_SIZE;
        static const uint32_t SRTM1_LINEWIDTH;
        static const uint32_t SRTM3_LINEWIDTH;
        static const uint32_t SRTM1_LINEBYTES;
        static const uint32_t SRTM3_LINEBYTES;
        static const int16_t  SRTM_NODATAVALUE;
        static const double   SRTM1_RES;
        static const double   SRTM3_RES;

        // Class ID for this class.
        HDECLARE_CLASS_ID(HRFFileId_SRTM, HRFRasterFile)

            friend class HRFSRTMLineEditor;
        friend class HRFSRTMImageEditor;

        HRFSRTMFile(const HFCPtr<HFCURL>&          pi_rpURL,
                   HFCAccessMode                  pi_AccessMode = HFC_READ_ONLY,
                   uint64_t                       pi_Offset = 0);

        virtual                                    ~HRFSRTMFile();

        // File capabilities
        const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities() const override;

        // File information
        const HGF2DWorldIdentificator GetWorldIdentificator() const override;

        // File manipulation
        bool                         AddPage(HFCPtr<HRFPageDescriptor> pi_pPage) override;

        HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                     uint16_t           pi_Resolution,
                                                                     HFCAccessMode             pi_AccessMode) override;
    protected:

        virtual bool                        Open();
        virtual void                        CreateDescriptors();

    private:
        HAutoPtr<HFCBinStream>  m_pSRTMFile;
        uint32_t   m_Width;        // width and height of image in pixels

        //Disabled
        void                  Save() override;
        void                          ExtractLatLong(double* latitude, double* longitude) const;
        bool                          IsSRTM1() const;
    };

struct HRFSRTMCreator : public HRFRasterFileCreator
    {
    // Opens the file and verifies if it is the right type
    bool                     IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                  uint64_t                 pi_Offset = 0) const override;

    // Identification information
    Utf8String                   GetLabel() const override;
    Utf8String                   GetSchemes() const override;
    Utf8String                   GetExtensions() const override;

    virtual Utf8String GetShortName() const override { return "SRTM"; }

    // capabilities of Raster file.
    const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities() override;

    // allow to Open an image file READ/WRITE and CREATE
    HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t              pi_Offset = 0) const override;

    private:
        HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFSRTMCreator)

            // Disabled methodes
            HRFSRTMCreator();
    };
END_IMAGEPP_NAMESPACE
