//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/ImageComparator/ImageComparator.cpp $
//:>    $RCSfile: HTiffInfo.cpp,v $
//:>   $Revision: 1.15 $
//:>       $Date: 2011/07/18 21:12:32 $
//:>     $Author: Donald.Morissette $
//:>
//:>  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <stdio.h>
#include <conio.h>
#include <tchar.h>

#include <windows.h>
#include <Imagepp/h/ImageppAPI.h>
#include <Imagepp/all/h/HTIFFFile.h>

#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCURLFile.h>

#include <Imagepp/all/h/HRFJpegFile.h>
#include <Imagepp/all/h/HRFUtility.h>

#include <ImagePP/all/h/HRFFileFormats.h>
#include <Imagepp/all/h/ImageppLib.h>
#include <ImagePP/all/h/HCDPacket.h>
#include <ImagePP/all/h/HGFHMRStdWorldCluster.h>
#include <ImagePP/all/h/HUTImportFromRasterExportToFile.h>
#include <ImagePP/all/h/HRABitmap.h>
#include <ImagePP/all/h/HGF2DIdentity.h>
#include <ImagePP/all/h/HFCLocalBinStream.h>

#include <BeSQLite/L10N.h>
#include <BeSQLite/BeSQLite.h>

USING_NAMESPACE_IMAGEPP

HFCPtr<HGF2DWorldCluster> gWorldClusterP(new HGFHMRStdWorldCluster());
#define UPPER_LEFT_COORDSYS gWorldClusterP->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD)     // Origin upper-Left 

static void       sDisplayUsage       ();
static void       sDisplayBuffer      (const Byte* pi_pBuffer, uint32_t pi_Count);
static void       sDisplayFileData    (HTIFFFile& pi_rFile, bool pi_ReadOnly);
static HTIFFFile* sGetHTIFFforJpegExifTags(HFCPtr<HFCURL>& pi_prUrl);

struct ImageComparatorImageppLibHost : ImagePP::ImageppLib::Host  
{                                                       
virtual void _RegisterFileFormat() override             
    {                                                       
    REGISTER_SUPPORTED_FILEFORMAT                           
    }                                                       
};    

static WString s_outDir;
//#define OUT_BASE_DIR L"D:\\skyNetTest\\output\\"


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HFCURLFile> BuildBlockUrl(WCharCP extension, HRFRasterFile& rasterFile, uint32_t page, uint32_t resolution, uint32_t blockPositionX, uint32_t blockPositionY)
    {
    BeFileName rasterFilename(static_cast<HFCURLFile*>(rasterFile.GetURL().GetPtr())->GetAbsoluteFileName().c_str());
    //BeFileName baseDir(baseDirCP);

    WString dir, name, ext;
    BeFileName::ParseName(NULL, &dir, &name, &ext, rasterFilename.c_str());

    BeFileName outFilename(s_outDir.c_str());
    outFilename.AppendToPath(dir.c_str());

    WString newName;
    newName.Sprintf(L"%s.%s_page(%d)_resolution(%d)_block(%d,%d).%s", name.c_str(), ext.c_str(), page, resolution, blockPositionX, blockPositionY, extension);
    outFilename.AppendToPath(newName.c_str());
    
    BeFileName::CreateNewDirectory(BeFileName::GetDirectoryName(outFilename).c_str());

    return new HFCURLFile(Utf8String("file://") + Utf8String(outFilename));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.Marchand                2/2018
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T>
void ScanMinMax(vector<double>& minSample, vector<double>& maxSample, HRABitmap const& bitmap)
{
    uint64_t width, height;
    bitmap.GetSize(&width, &height);

    if (bitmap.GetPacket()->GetDataSize() != sizeof(T)*width*height)
        {
        _tprintf(L"ScanMinMax: %zu dataSize mismatch. Expected %zu\n", bitmap.GetPacket()->GetDataSize(), sizeof(T)*width*height);
        return;
        }

    T const* pData = (T const*)bitmap.GetPacket()->GetBufferAddress();

    set<double> ignoredValues;

    T min = 0, max = 0;
    uint64_t i = 0;    
    for (; i < width*height; ++i)
    {
        if (pData[i] <= -32767)   // ignore no data values.
        {
            ignoredValues.insert(pData[i]);
            continue;
        }

        min = max = pData[i];
        break;
    }

    for (; i < width*height; ++i)
        {
        if (pData[i] <= -32767)   // ignore no data values.
            {
            ignoredValues.insert(pData[i]);
            continue;
            }

        if (pData[i] < min)
            min = pData[i];
        if (pData[i] > max)
            max = pData[i];
        }

    if (0 != min || 0 != max)
        {
        minSample.push_back(min);
        maxSample.push_back(max);
        }

    if (ignoredValues.empty())
        {
        _tprintf(L"ScanMinMax ignored values : ");
        for (auto const& ignoredValue : ignoredValues)
            {
            _tprintf(L"%f ", ignoredValue);
            }
        _tprintf(L"\n");
        }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.Marchand                2/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static void SetMinMaxTag(HUTImportFromRasterExportToFile& exporter, HRABitmap const& bitmap)
{   
    vector<double> minSample, maxSample;

    switch (bitmap.GetPixelType()->GetClassID())
        {
        case HRPPixelTypeId_V32Float32:
            ScanMinMax<float>(minSample, maxSample, bitmap);
            break;

        case HRPPixelTypeId_V16Gray16:
            ScanMinMax<uint16_t>(minSample, maxSample, bitmap);
            break;

        case HRPPixelTypeId_V16Int16:
            ScanMinMax<int16_t>(minSample, maxSample, bitmap);
            break;

        default:
            break;
        }
    
    if(!minSample.empty())
        exporter.SetTag(new HRFAttributeMinSampleValue(minSample));

    if (!maxSample.empty())
        exporter.SetTag(new HRFAttributeMaxSampleValue(maxSample));
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static bool CreateTiff(HFCPtr<HFCURLFile> pURL, HFCPtr<HRABitmap>& pBitmap)
    {
    try
        {           
        HUTImportFromRasterExportToFile exporter(pBitmap.GetPtr(), *pBitmap->GetEffectiveShape(), gWorldClusterP);

        // Select destination URL
        exporter.SelectExportFilename(pURL.GetPtr());

        // Select export format
        exporter.SelectExportFileFormat(HRFRasterFileFactory::GetInstance()->GetCreator(HRFTiffFile::CLASS_ID));

        // Select best pixel type
        exporter.SelectBestPixelType(pBitmap->GetPixelType());
        //assert(exporter.GetSelectedPixelType() == pBitmap->GetPixelType()->GetClassID());

        exporter.SelectCodec(HCDCodecId_Identity); 

        uint64_t width, height;
        pBitmap->GetSize(&width, &height);
        exporter.SelectBlockType(HRFBlockType::STRIP);           
        exporter.SetBlockWidth((uint32_t)width);
        exporter.SetBlockHeight((uint32_t)height);
        
        // Prefer single resolution file.
        exporter.SelectEncoding(HRFEncodingType(HRFEncodingType::STANDARD));
        
        SetMinMaxTag(exporter, *pBitmap);

        if(NULL != exporter.StartExport())
            return true;
        }
    catch(...)
        {
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static bool s_exportBlockData(HFCPtr<HFCURLFile>& pOutUrl, HRFResolutionDescriptor& descriptor, Byte* pData)
    {
    HFCPtr<HRABitmap> pBitmap = HRABitmap::Create(descriptor.GetBlockWidth(), descriptor.GetBlockHeight(), NULL, UPPER_LEFT_COORDSYS, descriptor.GetPixelType());

    HFCPtr<HCDPacket> pPacket = new HCDPacket(pData, descriptor.GetBlockSizeInBytes(), descriptor.GetBlockSizeInBytes());
    pBitmap->SetPacket(pPacket);

    return CreateTiff(pOutUrl, pBitmap);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.Marchand                2/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static bool s_writeRawBlockData(HFCPtr<HFCURLFile>& pOutUrl, Byte const* pData, size_t dataSize)
{
    HFCLocalBinStream fileStream(pOutUrl->GetAbsoluteFileName(), HFC_WRITE_AND_CREATE);

    return fileStream.Write(pData, dataSize) == dataSize;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void PrintBlockInfo(uint32_t page, uint32_t resolution , uint64_t positionX, uint64_t positionY)
    {
    _tprintf (_TEXT("\n  Page(%d) at resolution(%d) block position(%lld, %lld)\n"), page, resolution, positionX, positionY);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool CompareRasterFile(HRFRasterFile& raster1, HRFRasterFile& raster2)
    {
    if(raster1.CountPages() != raster2.CountPages())
        {
        _tprintf (_TEXT("CountPages mismatch. %d != %d \n"), raster1.CountPages(), raster2.CountPages());
        return false;
        }

    // Verify Geocoding and Georeference
    //
    HFCPtr<HRFPageDescriptor> pPageDescriptor1 = raster1.GetPageDescriptor(0);
    HFCPtr<HGF2DTransfoModel> pHRFTransfoRaster1;
    if (pPageDescriptor1->HasTransfoModel())
        pHRFTransfoRaster1 = pPageDescriptor1->GetTransfoModel();
    else
        pHRFTransfoRaster1 = new HGF2DIdentity();

    GeoCoordinates::BaseGCSCP pGgcs = pPageDescriptor1->GetGeocodingCP();
    WString WKT_Raster1;
    if (pGgcs != NULL)
        {
        pGgcs->GetWellKnownText(WKT_Raster1, GeoCoordinates::BaseGCS::wktFlavorOGC, true);
        if (WKT_Raster1 == L"")
            pGgcs->GetWellKnownText(WKT_Raster1, GeoCoordinates::BaseGCS::wktFlavorESRI, true);
        }

    HFCPtr<HRFPageDescriptor> pPageDescriptor2 = raster2.GetPageDescriptor(0);
    HFCPtr<HGF2DTransfoModel> pHRFTransfoRaster2;
    if (pPageDescriptor2->HasTransfoModel())
        pHRFTransfoRaster2 = pPageDescriptor2->GetTransfoModel();
    else
        pHRFTransfoRaster2 = new HGF2DIdentity();

    GeoCoordinates::BaseGCSCP pGgcs2 = pPageDescriptor2->GetGeocodingCP();
    WString WKT_Raster2;
    if (pGgcs2 != NULL)
        {
        pGgcs2->GetWellKnownText(WKT_Raster2, GeoCoordinates::BaseGCS::wktFlavorOGC, true);
        if (WKT_Raster1 == L"")
            pGgcs2->GetWellKnownText(WKT_Raster2, GeoCoordinates::BaseGCS::wktFlavorESRI, true);
        }

    double          MeanError;
    double          MaxError;
    HGF2DLiteExtent PrecisionArea(0.0, 0.0, (double)pPageDescriptor1->GetResolutionDescriptor(0)->GetWidth(),
                                            (double)pPageDescriptor1->GetResolutionDescriptor(0)->GetHeight());
    double          Step (min (pPageDescriptor1->GetResolutionDescriptor(0)->GetWidth(), pPageDescriptor2->GetResolutionDescriptor(0)->GetHeight()) / 3.0);
    pHRFTransfoRaster1->GetEquivalenceToOver(*pHRFTransfoRaster2, PrecisionArea, Step, true, &MeanError, &MaxError);
    

    if (HDOUBLE_GREATER_EPSILON(MaxError, 0.0) )
        _tprintf(_TEXT("Transfo model mismatch. "));

    if (((pGgcs != nullptr) && (pGgcs2 != nullptr) && !pGgcs->IsEquivalent(*pGgcs2)) ||
         (pGgcs == nullptr) && (pGgcs2 != nullptr) ||
         (pGgcs != nullptr) && (pGgcs2 == nullptr))
        _tprintf(_TEXT("Geocoding mismatch. "));


    for(uint32_t page=0; page < raster1.CountPages(); ++page)
        {
        HFCPtr<HRFPageDescriptor> pPage1 = raster1.GetPageDescriptor(page);
        HFCPtr<HRFPageDescriptor> pPage2 = raster2.GetPageDescriptor(page);

        if(pPage1->CountResolutions() != pPage2->CountResolutions())
            {
            _tprintf (_TEXT("CountResolutions mismatch. %d != %d \n"), pPage1->CountResolutions(), pPage2->CountResolutions());
            return false;
            }

        for(unsigned short resolution=0; resolution < pPage1->CountResolutions(); ++resolution)
            {
            HFCPtr<HRFResolutionDescriptor> pResolution1 = pPage1->GetResolutionDescriptor(resolution);
            HFCPtr<HRFResolutionDescriptor> pResolution2 = pPage2->GetResolutionDescriptor(resolution);

            if(!pResolution1->GetPixelType()->HasSamePixelInterpretation(*pResolution2->GetPixelType()))
                {
                _tprintf (_TEXT("Pixeltype mismatch.\n"));
                return false;
                }

            if(pResolution1->GetBlockType() != pResolution2->GetBlockType())
                {
                _tprintf (_TEXT("BlockType mismatch: %d != %d\n"), (int)pResolution1->GetBlockType() ,(int)pResolution2->GetBlockType());
                return false;
                }

            if(pResolution1->GetScanlineOrientation() != pResolution2->GetScanlineOrientation())
                {
                _tprintf (_TEXT("ScanlineOrientation mismatch: %d != %d\n"), (int)pResolution1->GetScanlineOrientation(), (int)pResolution2->GetScanlineOrientation());
                return false;
                }

            if(pResolution1->GetWidth() != pResolution2->GetWidth())
                {
                _tprintf (_TEXT("Width mismatch: %lld != %lld\n"), pResolution1->GetWidth(),pResolution2->GetWidth());
                return false;
                }

            if(pResolution1->GetHeight() != pResolution2->GetHeight())
                {
                _tprintf (_TEXT("Height mismatch: %lld != %lld\n"), pResolution1->GetHeight(),pResolution2->GetHeight());
                return false;
                }

            if(pResolution1->GetBitsPerWidth() != pResolution2->GetBitsPerWidth())
                {
                _tprintf (_TEXT("BitsPerWidth mismatch: %lld != %lld\n"), pResolution1->GetBitsPerWidth(),pResolution2->GetBitsPerWidth());
                return false;
                }

            if(pResolution1->GetBytesPerBlockWidth() != pResolution2->GetBytesPerBlockWidth())
                {
                _tprintf (_TEXT("BytesPerBlockWidth mismatch: %ld != %ld\n"), pResolution1->GetBytesPerBlockWidth(),pResolution2->GetBytesPerBlockWidth());
                return false;
                }

             if(pResolution1->GetBlocksPerWidth() != pResolution2->GetBlocksPerWidth())
                {
                _tprintf (_TEXT("BlocksPerWidth mismatch: %lld != %lld\n"), pResolution1->GetBlocksPerWidth(),pResolution2->GetBlocksPerWidth());
                return false;
                }

             if(pResolution1->GetBlocksPerHeight() != pResolution2->GetBlocksPerHeight())
                {
                _tprintf (_TEXT("BlocksPerHeight mismatch: %lld != %lld\n"), pResolution1->GetBlocksPerHeight(),pResolution2->GetBlocksPerHeight());
                return false;
                }

             if(pResolution1->GetBlockWidth() != pResolution2->GetBlockWidth())
                {
                _tprintf (_TEXT("BlockWidth mismatch: %d != %d\n"), pResolution1->GetBlockWidth(),pResolution2->GetBlockWidth());
                return false;
                }

             if(pResolution1->GetBlockHeight() != pResolution2->GetBlockHeight())
                {
                _tprintf (_TEXT("BlockHeight mismatch: %d != %d\n"), pResolution1->GetBlockHeight(), pResolution2->GetBlockHeight());
                return false;
                }

             std::unique_ptr<HRFResolutionEditor> pEditor1(raster1.CreateResolutionEditor(page, resolution, HFC_READ_ONLY));

             std::unique_ptr<HRFResolutionEditor> pEditor2(raster2.CreateResolutionEditor(page, resolution, HFC_READ_ONLY));

             if(pEditor1 == NULL || pEditor2 == NULL)
                {
                _tprintf (_TEXT("Cannot create resolution editor"));
                return false;
                }

             for(uint64_t indexHeight=0; indexHeight < pResolution1->GetBlocksPerHeight(); ++indexHeight)
                 {
                 for(uint64_t indexWidth=0; indexWidth < pResolution1->GetBlocksPerWidth(); ++indexWidth)
                     {
                     HFCPtr<HCDPacket> pPacket1 = new HCDPacket();
                     HFCPtr<HCDPacket> pPacket2 = new HCDPacket();
                     uint32_t blockPositionX = (uint32_t)indexWidth*pResolution1->GetBlockWidth();
                     uint32_t blockPositionY = (uint32_t)indexHeight*pResolution1->GetBlockHeight();
                     HSTATUS status1 = pEditor1->ReadBlock(blockPositionX, blockPositionY, pPacket1);
                     HSTATUS status2 = pEditor2->ReadBlock(blockPositionX, blockPositionY, pPacket2);

                     if(status1 != status2 || status1 != H_SUCCESS)
                        {
                        PrintBlockInfo(page, resolution, blockPositionX, blockPositionY);
                        _tprintf (_TEXT("   ReadBlock Error: First=%d Second:%d\n"), status1, status2);
                        return false;
                        }

                     if(pPacket1->GetDataSize() != pPacket2->GetDataSize() || 0 != memcmp(pPacket1->GetBufferAddress(), pPacket2->GetBufferAddress(), pPacket2->GetDataSize()))
                        {
                        PrintBlockInfo(page, resolution, blockPositionX, blockPositionY);
                        
                        HFCPtr<HCDPacket> pUnCompressedPacket1 = new HCDPacket(new Byte[pResolution1->GetBlockSizeInBytes()], pResolution1->GetBlockSizeInBytes(), pResolution1->GetBlockSizeInBytes());
                        pUnCompressedPacket1->SetBufferOwnership(true);
                        memset(pUnCompressedPacket1->GetBufferAddress(), 0, pUnCompressedPacket1->GetBufferSize());
                        pPacket1->Decompress(pUnCompressedPacket1);

                        HFCPtr<HCDPacket> pUnCompressedPacket2 = new HCDPacket(new Byte[pResolution2->GetBlockSizeInBytes()], pResolution2->GetBlockSizeInBytes(), pResolution2->GetBlockSizeInBytes());
                        pUnCompressedPacket2->SetBufferOwnership(true);
                        memset(pUnCompressedPacket2->GetBufferAddress(), 0, pUnCompressedPacket2->GetBufferSize());
                        pPacket2->Decompress(pUnCompressedPacket2);
                        
                        if(0 != memcmp(pUnCompressedPacket1->GetBufferAddress(), pUnCompressedPacket2->GetBufferAddress(), pResolution2->GetBlockSizeInBytes()))
                            {
                            _tprintf (_TEXT("   Block Data mismatch, size1: %zd size2 %zd\n"), pPacket1->GetDataSize(), pPacket1->GetDataSize());
                            HFCPtr<HFCURLFile> pUrl1 = BuildBlockUrl(L"tif", raster1, page, resolution, blockPositionX, blockPositionY);
                            _tprintf(_TEXT("Block file:%s\n"), WString((pUrl1->GetHost() + "\\" + pUrl1->GetPath()).c_str(), BentleyCharEncoding::Utf8).c_str());

                            if(!s_exportBlockData(pUrl1, *pResolution1, pUnCompressedPacket1->GetBufferAddress()))
                                _tprintf (_TEXT("Error exporting block left data \n"));

                            HFCPtr<HFCURLFile> pUrl2 = BuildBlockUrl(L"tif", raster2, page, resolution, blockPositionX, blockPositionY);
                            if(!s_exportBlockData(pUrl2, *pResolution2, pUnCompressedPacket2->GetBufferAddress()))
                                _tprintf (_TEXT("Error exporting block right data \n"));

                            HFCPtr<HFCURLFile> pUrlRaw1 = BuildBlockUrl(L"raw", raster1, page, resolution, blockPositionX, blockPositionY);
                            if (!s_writeRawBlockData(pUrlRaw1, pUnCompressedPacket1->GetBufferAddress(), pUnCompressedPacket1->GetDataSize()))
                                _tprintf(_TEXT("Error write RAW block left data \n"));

                            HFCPtr<HFCURLFile> pUrlRaw2 = BuildBlockUrl(L"raw", raster2, page, resolution, blockPositionX, blockPositionY);
                            if (!s_writeRawBlockData(pUrlRaw2, pUnCompressedPacket2->GetBufferAddress(), pUnCompressedPacket2->GetDataSize()))
                                _tprintf(_TEXT("Error write RAW block right data \n"));

                            return false;
                            }
                        else
                            {
                            _tprintf (_TEXT("   Warning: Compressed Data mismatch but uncompressed is equal"));
                            }
                        }
                     }
                 }
            }
        }

    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static void s_BuildFileList(std::list<WString>& fileList, WCharCP originalPath, WCharCP subPath=NULL)
{
    if(originalPath == NULL || wcslen(originalPath) == 0)
        return;

    WString folder(originalPath);
    WString subFolder;
    if (subPath != NULL)
        subFolder = subPath;

    WIN32_FIND_DATA FindFileData;
    WString searchPath(folder + subFolder + _TEXT("\\*"));
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &FindFileData);

    if (hFind == INVALID_HANDLE_VALUE) 
        return;        
    do 
    {
        // not . and ..
        if(FindFileData.cFileName[0] != '.')
        {
            if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                // recursively process directories
                WString subSearchPath(subFolder + _TEXT("\\") + FindFileData.cFileName);
                s_BuildFileList(fileList, originalPath, subSearchPath.c_str());
            }
            else
            {
                // Skip Info file generated by ATPs
                if (!Utf8String(FindFileData.cFileName).Contains(Utf8String(".testInfo")))
                    {
                    WString filePath(subFolder + _TEXT("\\") + FindFileData.cFileName);
                    fileList.push_back(filePath);
                    }
            }
        }               
    } 
    while(FindNextFile(hFind, &FindFileData));

    fileList.sort();
    FindClose(hFind);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool AreFileEqual(HFCBinStream& leftFile, HFCBinStream& rightFile)
    {
#define CMP_BUFFER_SIZE_MB 1
#define CMP_BUFFER_SIZE_BYTES CMP_BUFFER_SIZE_MB*1024*1024

    uint64_t fileSize = rightFile.GetSize();
    if(leftFile.GetSize() != fileSize)
        return false;

    std::unique_ptr<Byte[]> pLeftBuffer(new Byte[CMP_BUFFER_SIZE_BYTES]);
    std::unique_ptr<Byte[]> pRightBuffer(new Byte[CMP_BUFFER_SIZE_BYTES]);

    leftFile.SeekToBegin();
    rightFile.SeekToBegin();

    while(!leftFile.EndOfFile() && !rightFile.EndOfFile())
        {
        size_t leftRead = leftFile.Read(pLeftBuffer.get(), CMP_BUFFER_SIZE_BYTES);
        size_t rightRead = rightFile.Read(pRightBuffer.get(), CMP_BUFFER_SIZE_BYTES);
        
        if(leftRead != rightRead || 0 != memcmp(pLeftBuffer.get(), pRightBuffer.get(), leftRead))
            return false;
        }

    return true; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static bool s_ProcessDir(WCharCP leftDir, WCharCP rightDir, bool DeleteOutputDir)
    {
    if (DeleteOutputDir)
        {
        BeFileNameStatus status = BeFileName::EmptyDirectory(s_outDir.c_str());
        if(BeFileNameStatus::FileNotFound != status && BeFileNameStatus::Success != status)
            {
            _tprintf(L"Cannot empty output directory: %s", s_outDir.c_str());
            return false;
            }
        }

    std::list<WString> leftFileList;
    s_BuildFileList(leftFileList, leftDir);

    std::list<WString> rightFileList;
    s_BuildFileList(rightFileList, rightDir);

    std::list<WString> InterFileList;
    std::set_intersection(leftFileList.begin(), leftFileList.end(),
                          rightFileList.begin(), rightFileList.end(),
                           std::back_inserter(InterFileList));

    _tprintf(L"-------------------------------------------------------\n   Left dir : %s \n   Right dir: %s\n\n", leftDir, rightDir);

    // Left item only
    std::list<WString> LeftItemOnly;
    std::set_difference(leftFileList.begin(), leftFileList.end(),
                        InterFileList.begin(), InterFileList.end(),
                        std::inserter(LeftItemOnly, LeftItemOnly.begin()));
    _tprintf(L"--------- Left item only :\n");
    for (auto itr : LeftItemOnly)
        _tprintf(L"   %s \n", itr.c_str());

    // Right item only
    std::list<WString> RightItemOnly;
    std::set_difference(rightFileList.begin(), rightFileList.end(),
        InterFileList.begin(), InterFileList.end(),
        std::inserter(RightItemOnly, RightItemOnly.begin()));
    _tprintf(L"--------- Right item only :\n");
    for (auto itr : RightItemOnly)
        _tprintf(L"   %s \n", itr.c_str());

    _tprintf(L"-------------------------------------------------------\n");


    Utf8String leftDirStr(leftDir);
    Utf8String rightDirStr(rightDir);

    uint32_t proceedFile = 1;
    for(auto fileItr : InterFileList)
        {
        Utf8String curFile(fileItr.c_str());

        HFCPtr<HFCURLFile> pLeftURL= new HFCURLFile(Utf8String("file://") + leftDirStr + curFile);
        HFCPtr<HFCURLFile> pRightURL = new HFCURLFile(Utf8String("file://") + rightDirStr + curFile);

        _tprintf(L"Processing(%d/%d) %s: ", proceedFile, (uint32_t)InterFileList.size(), WString(curFile.c_str(),BentleyCharEncoding::Utf8).c_str());

        try
            {
            std::unique_ptr<HFCBinStream> pLeftFile(HFCBinStream::Instanciate(pLeftURL.GetPtr(), HFC_READ_ONLY | HFC_SHARE_READ_WRITE, 0, true));
            std::unique_ptr<HFCBinStream> pRightFile(HFCBinStream::Instanciate(pRightURL.GetPtr(), HFC_READ_ONLY | HFC_SHARE_READ_WRITE, 0, true));

            if(AreFileEqual(*pLeftFile, *pRightFile))
                {
                _tprintf(L".........OK\n");
                }
            else
                {      
                pLeftFile.release();
                pRightFile.release();

                HFCPtr<HRFRasterFile> pRightRaster = HRFRasterFileFactory::GetInstance()->OpenFile(pRightURL.GetPtr(), true);
                HFCPtr<HRFRasterFile> pLeftRaster = HRFRasterFileFactory::GetInstance()->OpenFile(pLeftURL.GetPtr(), true);
                
                if(CompareRasterFile(*pLeftRaster, *pRightRaster))
                    {
                    _tprintf(L"Not binary equal but could not find difference..........OK\n");                    
                    }
                else
                    {
                    _tprintf(L"*** FAILURE ***\n  %s\n  %s\n", WString(pLeftURL->GetAbsoluteFileName().c_str(), BentleyCharEncoding::Utf8).c_str(),
                             WString(pRightURL->GetAbsoluteFileName().c_str(), BentleyCharEncoding::Utf8).c_str());
                    }
                }
            }
        catch (HFCFileException& e)
            {
            _tprintf(L"Exception %s\n", WString(e.GetExceptionMessage().c_str(), BentleyCharEncoding::Utf8).c_str());
            //_tprintf(L"Exception %s(%s)\n", e.GetExceptionMessage().c_str(), e.GetFileName());
            }
        catch (HFCException& e)
            {
            _tprintf(L"Exception %s\n", WString(e.GetExceptionMessage().c_str(), BentleyCharEncoding::Utf8).c_str());
            //_tprintf(L"Exception: %s(%s)\n", e.GetExceptionMessage().c_str(), pRightURL->GetAbsoluteFileName());
            }      
        catch(...)
            {
            _tprintf(L"Unhanded exception occurs with file %s\n", WString(curFile.c_str(), BentleyCharEncoding::Utf8).c_str());
            }

        ++proceedFile;
        }

    return true;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2010
//---------------------------------------------------------------------------------------
static WString getBaseDirOfExecutingModule()
{
    WChar fullExePath[_MAX_PATH];
    ::GetModuleFileNameW(NULL, fullExePath, _MAX_PATH);

    WChar fullExeDrive[_MAX_DRIVE];
    WChar fullExeDir[_MAX_DIR];
    _wsplitpath_s(fullExePath, fullExeDrive, _MAX_DRIVE, fullExeDir, _MAX_DIR, NULL, 0, NULL, 0);

    WChar baseDirW[_MAX_PATH];
    _wmakepath_s(baseDirW, fullExeDrive, fullExeDir, NULL, NULL);

    return baseDirW;
}

int wmain(int pi_Argc, wchar_t *pi_ppArgv[])
{
     if (pi_Argc < 4)
        sDisplayUsage();

    BeFileName leftDir(pi_ppArgv[2]);
    BeFileName rightDir(pi_ppArgv[3]);

    if (!leftDir.IsDirectory())
        {
        _tprintf(L"Left Dir doesn't exist or is not a directory: %s", leftDir.c_str());
        return 1;
        }

    if (!rightDir.IsDirectory())
        {
        _tprintf(L"Right Dir doesn't exist or is not a directory: %s", rightDir.c_str());
        return 1;
        }

    bool pauseAtEnd = false;
    bool DeleteOutputDir = true;

    // parse others options
    for (int i = 4; i < pi_Argc; ++i)
        {
        if (WString(pi_ppArgv[i]).CompareToI(L"--pause") == 0)
            pauseAtEnd = true;

        if (WString(pi_ppArgv[i]).CompareToI(L"--noDeleteOutputDir") == 0)
            DeleteOutputDir = false;
        } 


    // BeSQLiteLib initialization. Must be called only once. Needed by L10N
    WChar tempPath[MAX_PATH] = L"";
    GetTempPathW(MAX_PATH, tempPath);
    BeFileName tempDir(tempPath);
    BeAssert(tempDir.DoesPathExist());
    BeSQLite::BeSQLiteLib::Initialize(tempDir);

    // L10N initialization
    BeFileName defaultFile(getBaseDirOfExecutingModule());
    defaultFile.AppendToPath(L"Assets/imageppApps_en.sqlang.db3");
    BeAssert(defaultFile.DoesPathExist());
    BentleyStatus l10nStatus = BeSQLite::L10N::Initialize(BeSQLite::L10N::SqlangFiles(defaultFile));
    BeAssert(BentleyStatus::SUCCESS == l10nStatus);

    //Initialize ImagePP host
    ImagePP::ImageppLib::Initialize(*new ImageComparatorImageppLibHost());
                       
    s_outDir = pi_ppArgv[1];

    s_ProcessDir(leftDir.c_str(), rightDir.c_str(), DeleteOutputDir);
                
    if (pauseAtEnd)
        {
        _ftprintf(stderr, _TEXT("\n\nPress any key to exit\n"));
        _getch();
        }

    //Terminate ImagePP lib host
    ImagePP::ImageppLib::GetHost().Terminate(true);

    return 0;
}

static void sDisplayUsage()
{
    _ftprintf (stderr, _TEXT("\nImageComparator V1.1 --- %s\n\n"),  _T(__DATE__));
    _ftprintf(stderr, _TEXT("Usage:\n"));
    _ftprintf(stderr, _TEXT("ImageComparator.exe resultDir dir1 dir2 [--pause]|[--noDeleteOutputDir]\n"));
    exit(-1);
}




