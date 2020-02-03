/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/OffscreenRendering/ImageFile.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
    #pragma once
class ImageFile
    {

    public:

    static bool CreateTiffFromRGBATile(Byte const* pIn, wstring const& filename);

    static bool CreateTiff(HFCPtr<HFCURL> pURL, HFCPtr<HRABitmap> pBitmap);

    static bool ReadRaw_256x256_RGBA(Byte* pOut, string const& filename);

    static bool WriteRaw(Byte const* pIn, size_t size, string const& filename);

        ImageFile(wstring const& filename);

        ~ImageFile(void);

    bool IsValid() const {return m_pRasterFile != NULL && m_pStoredRaster != NULL;}

    uint32_t    GetWidth() const
        {
        if (m_pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetScanlineOrientation().IsScanlineVertical())
            return (uint32_t)m_pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight();

        return (uint32_t)m_pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth();
        }

    uint32_t    GetHeight() const
        {
        if (m_pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetScanlineOrientation().IsScanlineVertical())
            return (uint32_t)m_pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth();

        return (uint32_t)m_pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight();
        }

    HFCPtr<HRFRasterFile> GetRasterFile() {return m_pRasterFile;}

    HFCPtr<HRAStoredRaster> GetRaster() {return m_pStoredRaster;}
    
    private:

        HFCPtr<HRFRasterFile>       m_pRasterFile;
        HFCPtr<HRAStoredRaster>     m_pStoredRaster;
    };

