//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFHgtFile.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFHgtFile
//-----------------------------------------------------------------------------
// This class describes an HGT File Raster image.
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
class HRFHgtCapabilities : public HRFRasterFileCapabilities
    {
    public:
        HRFHgtCapabilities();
    };

class HRFHgtFile : public HRFRasterFile
    {
    public:

        //Possible file sizes for HGT files
        static const uint32_t HRFHgtFile::SRTM1_SIZE;
        static const uint32_t HRFHgtFile::SRTM3_SIZE;
        static const uint32_t HRFHgtFile::SRTM1_LINEWIDTH;
        static const uint32_t HRFHgtFile::SRTM3_LINEWIDTH;
        static const uint32_t HRFHgtFile::SRTM1_LINEBYTES;
        static const uint32_t HRFHgtFile::SRTM3_LINEBYTES;
        static const int16_t  HRFHgtFile::HGT_NODATAVALUE;
        static const double   HRFHgtFile::EARTH_RADIUS;
        static const double   HRFHgtFile::SRTM1_RES;
        static const double   HRFHgtFile::SRTM3_RES;

        // Class ID for this class.
        HDECLARE_CLASS_ID(HRFFileId_Hgt, HRFRasterFile)

            friend class HRFHgtLineEditor;
        friend class HRFHgtImageEditor;

        HRFHgtFile(const HFCPtr<HFCURL>&          pi_rpURL,
                   HFCAccessMode                  pi_AccessMode = HFC_READ_ONLY,
                   uint64_t                       pi_Offset = 0);

        virtual                                    ~HRFHgtFile();

        // File capabilities
        virtual const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities() const;

        // File information
        virtual const HGF2DWorldIdentificator GetWorldIdentificator() const;

        // File manipulation
        virtual bool                         AddPage(HFCPtr<HRFPageDescriptor> pi_pPage);

        virtual HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                     uint16_t           pi_Resolution,
                                                                     HFCAccessMode             pi_AccessMode);
    protected:

        virtual bool                        Open();
        virtual void                        CreateDescriptors();

    private:
        HAutoPtr<HFCBinStream>  m_pHgtFile;
        uint32_t   m_Width;        // width and height of image in pixels

        //Disabled
        virtual void                  Save();
        void                          ExtractLatLong(double* latitude, double* longitude) const;
        bool                          IsSRTM1() const;
    };

struct HRFHgtCreator : public HRFRasterFileCreator
    {
    // Opens the file and verifies if it is the right type
    virtual bool                     IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                  uint64_t                 pi_Offset = 0) const;

    // Identification information
    virtual Utf8String                   GetLabel() const;
    virtual Utf8String                   GetSchemes() const;
    virtual Utf8String                   GetExtensions() const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities();

    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t              pi_Offset = 0) const;

    private:
        HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFHgtCreator)

            // Disabled methodes
            HRFHgtCreator();
    };
END_IMAGEPP_NAMESPACE