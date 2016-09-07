/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/RealityPlatformUtil.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|   Header File Dependencies
+----------------------------------------------------------------------------*/
#include "stdafx.h"

#include <ImagePP/all/h/HRFPageFileFactory.h>
#include <ImagePP/all/h/HRFRasterFilePageDecorator.h>
#include <RealityPlatform/RealityPlatformUtil.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void GetBaseDirOfExecutingModule(WStringR baseDir)
    {
    WChar fullExePath[_MAX_PATH];
    ::GetModuleFileNameW(NULL, fullExePath, _MAX_PATH);

    WChar fullExeDrive[_MAX_DRIVE];
    WChar fullExeDir[_MAX_DIR];
    _wsplitpath_s(fullExePath, fullExeDrive, _MAX_DRIVE, fullExeDir, _MAX_DIR, NULL, 0, NULL, 0);

    WChar baseDirW[_MAX_PATH];
    _wmakepath_s(baseDirW, fullExeDrive, fullExeDir, NULL, NULL);

    baseDir = baseDirW;
    }

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
    geoCoordDir.append(L"Assets\\");
    geoCoordDir.append(L"DgnGeoCoord");

    // Make sure directory exist.
    BeFileName dir(geoCoordDir);
    if (!dir.IsDirectory())
        return false;

    GeoCoordinates::BaseGCS::Initialize(geoCoordDir.c_str());

    return true;
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HRFRasterFile> RasterFacility::GetRasterFile(Utf8CP inFilename)
    {
    HFCPtr<HRFRasterFile> rasterFile;

    try
        {
        if (Utf8String::IsNullOrEmpty(inFilename))
            return NULL;

        Utf8String filename = inFilename;

        // Create URL
        HFCPtr<HFCURL>  srcFilename(HFCURL::Instanciate(filename));
        if (srcFilename == 0)
            {
            // Open the raster file as a file
            srcFilename = new HFCURLFile(Utf8PrintfString("%s://%s", HFCURLFile::s_SchemeName().c_str(), inFilename));
            }

        // Open Raster file
        {
        // HFCMonitor __keyMonitor(m_KeyByMethod);

        // Create URL
        HFCPtr<HFCURL>  srcFilename(HFCURL::Instanciate(filename));
        if (srcFilename == 0)
            {
            // Open the raster file as a file
            srcFilename = new HFCURLFile(Utf8PrintfString("%s://%s", HFCURLFile::s_SchemeName().c_str(), inFilename));
            }

        // Open Raster file without checking "isKindOfFile"
        rasterFile = HRFRasterFileFactory::GetInstance()->OpenFile((HFCPtr<HFCURL>)srcFilename, true);
        }

        if (rasterFile == 0)
            return rasterFile;

        // Check if we have an internet imaging file
        // DISABLED: We do not support HRFInternetImagingFile
        //         if (RasterFile->IsCompatibleWith(HRFInternetImagingFile::CLASS_ID))
        //             ((HFCPtr<HRFInternetImagingFile>&)RasterFile)->DownloadAttributes();

        // Adapt Scan Line Orientation (1 bit images)
        bool CreateSLOAdapter = false;

        if ((rasterFile->IsCompatibleWith(HRFFileId_Intergraph)) ||
            (rasterFile->IsCompatibleWith(HRFFileId_Cals)))
            {
            if (HRFSLOStripAdapter::NeedSLOAdapterFor(rasterFile))
                {
                // Adapt only when the raster file has not a standard scan line orientation
                // i.e. with an upper left origin, horizontal scan line.
                //pi_rpRasterFile = HRFSLOStripAdapter::CreateBestAdapterFor(pi_rpRasterFile);
                CreateSLOAdapter = true;
                }
            }

            // Add the Decoration HGR, TFW or ERS Page File
            if (HRFPageFileFactory::GetInstance()->HasFor(rasterFile, false))
                {
                const HRFPageFileCreator* pPageFileCreator(HRFPageFileFactory::GetInstance()->FindCreatorFor(rasterFile, false));

                HASSERT(pPageFileCreator != 0);

                HFCPtr<HRFPageFile> pPageFile(pPageFileCreator->CreateFor(rasterFile));

                pPageFile->SetDefaultRatioToMeter(1.0);

                HASSERT(pPageFile != 0);

                rasterFile = new HRFRasterFilePageDecorator(rasterFile, pPageFile);
                }
        }
    catch (HFCException&)
        {
        return NULL;
        }
    catch (exception &e)
        {
        //C++ exception
        ostringstream errorStr;

        errorStr << "Caught " << e.what() << endl;
        errorStr << "Type " << typeid(e).name() << endl;

        return NULL;
        }
    catch (...)
        {
        return NULL;
        }

    return rasterFile;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    12/2015
//-------------------------------------------------------------------------------------
bool RasterFacility::CreateSisterFile(Utf8CP fileName, Utf8CP coordinateSystemKeyName)
    {
    // Get raster file.
    HFCPtr<HRFRasterFile> pRasterFile = RasterFacility::GetRasterFile(fileName);
    if (pRasterFile == 0 || pRasterFile->CountPages() <= 0)
        return false;

    // Get geocoding.
    HFCPtr<HRFPageDescriptor> pPageDescriptor = pRasterFile->GetPageDescriptor(0);
    GeoCoordinates::BaseGCSCP pSrcFileGeocoding = pPageDescriptor->GetGeocodingCP();
    if (pSrcFileGeocoding != nullptr)
        if (pSrcFileGeocoding->IsValid())
            return true;

    // No geocoding, create one.
    WString csKeyNameW;
    BeStringUtilities::Utf8ToWChar(csKeyNameW, coordinateSystemKeyName);
    GeoCoordinates::BaseGCSPtr pDestGeoCoding = GeoCoordinates::BaseGCS::CreateGCS(csKeyNameW.c_str());
    if (pDestGeoCoding == nullptr || !pDestGeoCoding->IsValid())
        {
        // Failed to create GCS from keyname, try with epsg code.
        int epsgCode;
        WString csKeyNameW(coordinateSystemKeyName, BentleyCharEncoding::Utf8);
        size_t pos = csKeyNameW.rfind(L"EPSG:");
        if (Utf8String::npos != pos)
            epsgCode = BeStringUtilities::Wtoi(csKeyNameW.substr(pos + 5).c_str());
        else
            epsgCode = BeStringUtilities::Wtoi(csKeyNameW.c_str());
        
        if (SUCCESS != pDestGeoCoding->InitFromEPSGCode(NULL, NULL, epsgCode))
            return false;
        }        

    // Check if a sister file already exists, if not create one and assign geocoding.
    BeFile sisterFile;
    BeFileName rasterFileName(fileName);
    BeFileName sisterFileName(rasterFileName.GetDevice().c_str(), rasterFileName.GetDirectoryWithoutDevice().c_str(), rasterFileName.GetFileNameWithoutExtension().c_str(), L"prj");
    BeFileStatus status = sisterFile.Create(sisterFileName, false);
    if (BeFileStatus::Success != status)
        return true;

    WString wellKnownText;
    if (SUCCESS != pDestGeoCoding->GetWellKnownText(wellKnownText, GeoCoordinates::BaseGCS::WktFlavor::wktFlavorOGC, false))
        return false;

    Utf8String wellKnownTextUtf8;
    BeStringUtilities::WCharToUtf8(wellKnownTextUtf8, wellKnownText.c_str());

    uint32_t byteCount = 0;
    uint32_t byteCountToCopy = (uint32_t) wellKnownTextUtf8.SizeInBytes();
    status = sisterFile.Write(&byteCount, wellKnownTextUtf8.c_str(), byteCountToCopy);
    sisterFile.Close();
    if (BeFileStatus::Success != status || byteCount != byteCountToCopy)
        return false;

    return true;
    }
	