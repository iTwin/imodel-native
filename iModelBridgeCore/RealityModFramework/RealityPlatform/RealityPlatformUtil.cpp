/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/RealityPlatformUtil.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|   Header File Dependencies
+----------------------------------------------------------------------------*/
#include "stdafx.h"
#include "RealityPlatformUtil.h"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool SessionManager::InitBaseGCS()
    {
    //WIP
    //BeFileName dllFileName;
    //Bentley::BeGetModuleFileName(dllFileName, NULL);

    //GetBaseDirOfExecutingModule

    WChar exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);

    WString geoCoordDir = exePath;
    size_t pos = geoCoordDir.find_last_of(L"/\\");
    geoCoordDir = geoCoordDir.substr(0, pos + 1);
    geoCoordDir.append(L"GeoCoordinateData");

    // Make sure directory exist.
    BeFileName dir(geoCoordDir);
    if (!dir.IsDirectory())
        return false;

    GeoCoordinates::BaseGCS::Initialize(geoCoordDir.c_str());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void GetBaseDirOfExecutingModule (WStringR baseDir)
    {
    WChar fullExePath[_MAX_PATH];
    ::GetModuleFileNameW (NULL, fullExePath, _MAX_PATH);

    WChar fullExeDrive[_MAX_DRIVE];
    WChar fullExeDir[_MAX_DIR];
    _wsplitpath_s (fullExePath, fullExeDrive, _MAX_DRIVE, fullExeDir, _MAX_DIR, NULL, 0, NULL, 0);

    WChar baseDirW[_MAX_PATH];
    _wmakepath_s (baseDirW, fullExeDrive, fullExeDir, NULL, NULL);

    baseDir = baseDirW;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterFacility::ConvertThePixels(size_t pi_Width, size_t pi_Height, const HFCPtr<HRPPixelType>& pi_rpSrcPixelType,
                                            const HFCPtr<HCDPacket>& pi_rpSrcPacket, const HFCPtr<HRPPixelType>& pi_rpDstPixelType,
                                            HFCPtr<HCDPacket>& po_rpDstPacket)
    {
    HPRECONDITION(po_rpDstPacket != 0);

    // calc bytes per row for the source and destination 
    size_t SrcBytesPerRow = (pi_rpSrcPixelType->CountPixelRawDataBits() * pi_Width + 7) / 8;
    size_t DstBytesPerRow = (pi_rpDstPixelType->CountPixelRawDataBits() * pi_Width + 7) / 8;

    // calc the uncompressed data source and destination size
    size_t SrcUncompressedDataSize = SrcBytesPerRow * pi_Height;
    size_t DstUncompressedDataSize = DstBytesPerRow * pi_Height;

    // create a packet to store the source pixels uncompressed
    HArrayAutoPtr<Byte> pSrcPixels(new Byte[SrcUncompressedDataSize]);
    HFCPtr<HCDPacket> pDecompressedSrcPacket(new HCDPacket(pSrcPixels, SrcUncompressedDataSize, SrcUncompressedDataSize));

    // uncompress the source pixels
    if (pi_rpSrcPacket->GetCodec() != 0)
        pi_rpSrcPacket->Decompress(pDecompressedSrcPacket);
    else
        memcpy(pDecompressedSrcPacket->GetBufferAddress(), pi_rpSrcPacket->GetBufferAddress(), SrcUncompressedDataSize);

    // create a pixels converter 
    HFCPtr<HRPPixelConverter> pConverter(pi_rpSrcPixelType->GetConverterTo(pi_rpDstPixelType));

    // Convert the pixels line by line
    Byte* pSrcBuffer     = pSrcPixels;	
    HArrayAutoPtr<Byte>  pDstPixels(new Byte[DstUncompressedDataSize]);
    Byte* pDstBuffer     = pDstPixels;

    if (pi_rpSrcPixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE)
        {
        // set the buffer to white
        memset(pDstPixels, 255, DstUncompressedDataSize);	 	

        for(uint32_t LineIndex = 0; LineIndex < pi_Height; LineIndex++)
            {
            pConverter->Compose(pSrcBuffer, pDstBuffer, pi_Width);
            pDstBuffer += DstBytesPerRow;
            pSrcBuffer += SrcBytesPerRow;
            }
        }
    else
        {
        for(uint32_t LineIndex = 0; LineIndex < pi_Height; LineIndex++)
            {
            pConverter->Convert(pSrcBuffer, pDstBuffer, pi_Width);
            pDstBuffer += DstBytesPerRow;
            pSrcBuffer += SrcBytesPerRow;
            }
        }

    // Compress the pixels 
    HFCPtr<HCDPacket> pDecompressedDstPacket(new HCDPacket(pDstPixels, DstUncompressedDataSize, DstUncompressedDataSize));	

    if (po_rpDstPacket->GetCodec() != 0)
        {
        // test if there is a buffer already defined
        if(po_rpDstPacket->GetBufferSize() == 0)
            po_rpDstPacket->SetBufferOwnership(true);	

        pDecompressedDstPacket->Compress(po_rpDstPacket);
        }
    else
        {
        // test if there is a buffer already defined
        if(po_rpDstPacket->GetBufferSize() == 0)
            {
            po_rpDstPacket->SetBuffer(pDstPixels.release(), DstUncompressedDataSize);
            po_rpDstPacket->SetBufferOwnership(true);
            }
        else
            memcpy(po_rpDstPacket->GetBufferAddress(), pDecompressedDstPacket->GetBufferAddress(), DstUncompressedDataSize);
        }
    }   
	
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterFacility::CreateHBitmapFromHRFThumbnail(HBITMAP* pThumbnailBmp, HFCPtr<HRFThumbnail>&  pThumbnail, HFCPtr<HRPPixelType>& pPixelType)
    {
    // Copy the thumbnail in GDI format GDI
//     RasterFileHandlerLogger::GetLogger()->message (LOG_TRACE, L"Copy the thumbnail in GDI format.");

    HArrayAutoPtr<Byte>  pSrcPixels;
    pSrcPixels = new Byte[pThumbnail->GetSizeInBytes()];

    uint32_t PreferedWidth = pThumbnail->GetWidth();
    uint32_t PreferedHeight = pThumbnail->GetHeight();

    pThumbnail->Read(pSrcPixels);
    
    // Convert the Pixel Type
    uint32_t SrcSizeInBytes = (((pPixelType->CountPixelRawDataBits() * PreferedWidth) + 7) /8) * PreferedHeight;    
    HFCPtr<HRPPixelType> pDstPixelType = new HRPPixelTypeV32B8G8R8X8();
    uint32_t DstBytesPerWidth = ((pDstPixelType->CountPixelRawDataBits() * PreferedWidth) + 7) /8;
    HFCPtr<HCDPacket>    pSourcePacket(new HCDPacket(pSrcPixels, SrcSizeInBytes, SrcSizeInBytes));
    HFCPtr<HCDPacket>    pConvertedPacket = new HCDPacket(0, 0, 0, 0);

    ConvertThePixels(PreferedWidth, PreferedHeight, pPixelType, pSourcePacket, pDstPixelType, pConvertedPacket);

    // Create the HBITMAP 
    BITMAPINFOHEADER BitInfo;

    BitInfo.biSize          = sizeof(BITMAPINFOHEADER);
    BitInfo.biWidth         = PreferedWidth;
    BitInfo.biHeight        = PreferedHeight;
    BitInfo.biPlanes        = 1;
    BitInfo.biBitCount      = 32;
    BitInfo.biCompression   = BI_RGB;
    BitInfo.biSizeImage     = 0;
    BitInfo.biXPelsPerMeter = 0;
    BitInfo.biYPelsPerMeter = 0;
    BitInfo.biClrUsed       = 0;
    BitInfo.biClrImportant  = 0;

    // BITMAP creation   
    void* pDataToCopy;
    *pThumbnailBmp = CreateDIBSection(NULL, (BITMAPINFO*)&BitInfo, DIB_RGB_COLORS, &pDataToCopy, NULL, 0);

    // Buffer allocation for bitmap
    // Each line in the DIB must be padded on a long [(+3)&-4]
    long DIBWidth = (((PreferedWidth*4L) + 3L) & -4L);

    BYTE* pBitmapData = reinterpret_cast<BYTE *>(pDataToCopy);

    // Convert the thumbnail data into the  buffer - due to the padding
    for (uint32_t i=0 ; i<PreferedHeight ; i++)
        memcpy(pBitmapData+(DIBWidth * (PreferedHeight - i -1)), 
        pConvertedPacket->GetBufferAddress() + (DstBytesPerWidth * i), 
        DstBytesPerWidth);

    }   
	