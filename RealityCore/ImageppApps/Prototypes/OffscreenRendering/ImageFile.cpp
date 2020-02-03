/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/OffscreenRendering/ImageFile.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"
#include "ImageFile.h"


extern HFCPtr<HGF2DWorldCluster> gWorldClusterP;

static HPMPool  s_pool(0/*illimited*/); // Memory pool shared by all rasters. (in KB)

ImageFile::ImageFile(wstring const& filename)
    {

    try
        {
        WString fullURL = HFCURLFile::s_SchemeName() + L"://" + filename.c_str();
        
        HFCPtr<HFCURL> pURL = new HFCURLFile(fullURL);

        m_pRasterFile = HRFRasterFileFactory::GetInstance()->OpenFile(pURL, HFC_READ_ONLY | HFC_SHARE_READ_ONLY);


        // load raster file in memory in a single chunk
        if (!HRFRasterFileBlockAdapter::CanAdapt(m_pRasterFile, HRFBlockType::IMAGE, HRF_EQUAL_TO_RESOLUTION_WIDTH, HRF_EQUAL_TO_RESOLUTION_HEIGHT) &&
            m_pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockType() != HRFBlockType::IMAGE) //Make sure we don't try to adapt something we don't have to adapt
            throw HRFException(HFC_CANNOT_OPEN_FILE_EXCEPTION, m_pRasterFile->GetURL()->GetURL());
                
        HFCPtr<HRFRasterFile> pAdaptedRasterFile(new HRFRasterFileBlockAdapter(m_pRasterFile, HRFBlockType::IMAGE, HRF_EQUAL_TO_RESOLUTION_WIDTH, HRF_EQUAL_TO_RESOLUTION_HEIGHT));

        HFCPtr<HGF2DCoordSys> pLogical = gWorldClusterP->GetCoordSysReference(pAdaptedRasterFile->GetWorldIdentificator());
        HFCPtr<HRSObjectStore> pStore = new HRSObjectStore (&s_pool, pAdaptedRasterFile, 0/*page*/, pLogical);

        // Specify we do not want to use the file's clip shapes if any
        pStore->SetUseClipShape(false);

        m_pStoredRaster = pStore->LoadRaster();
        }
    catch (...)
        {
        m_pRasterFile = NULL;
        }  
    }


ImageFile::~ImageFile(void)
    {
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImageFile::WriteRaw(Byte const* pIn, size_t bufferSize, string const& filename)
    {
    FILE* pFile = 0; 
    if(0 != fopen_s (&pFile, filename.c_str(), "wb+"))
        return false;

    bool success = bufferSize == fwrite(pIn, 1, bufferSize, pFile);

    fclose(pFile);
    return success;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImageFile::ReadRaw_256x256_RGBA(Byte* pOut, string const& filename)
    {
    FILE* pFile = 0; 
    if(0 != fopen_s (&pFile, filename.c_str(), "rb"))
        return false;

    size_t sizeRead = fread(pOut, 1, TILE_SIZE_BYTES, pFile);

    fclose(pFile);
    return TILE_SIZE_BYTES == sizeRead;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImageFile::CreateTiffFromRGBATile(Byte const* pIn, wstring const& filename)
    {
    WString fullURL = HFCURLFile::s_SchemeName() + L"://";

    if(filename.find(L":") == wstring::npos)
        {
        wchar_t defaultPath[MAX_PATH];
        fullURL += _wgetcwd(defaultPath, MAX_PATH);
        fullURL += L"/";
        fullURL += filename.c_str();
        }
    else
        {
        fullURL += filename.c_str();
        }        

    HFCPtr<HRABitmap> pBitmap = new HRABitmap(TILE_SIZE, TILE_SIZE, &HGF2DIdentity(), UPPER_LEFT_COORDSYS, new HRPPixelTypeV32R8G8B8A8);

    HFCPtr<HCDPacket> pPacket = new HCDPacket(const_cast<Byte*>(pIn), TILE_SIZE_BYTES, TILE_SIZE_BYTES);
    pPacket->SetBufferOwnership(false);
    pBitmap->SetPacket(pPacket);
       
    HFCPtr<HFCURL> pURL = new HFCURLFile(fullURL);

    return ImageFile::CreateTiff(pURL, pBitmap);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImageFile::CreateTiff(HFCPtr<HFCURL> pURL, HFCPtr<HRABitmap> pBitmap)
    {
    try
        {           
        HUTImportFromRasterExportToFile exporter(pBitmap.GetPtr(), *pBitmap->GetEffectiveShape(), gWorldClusterP);

        // Select destination URL
        exporter.SelectExportFilename(pURL);

        // Select export format
        exporter.SelectExportFileFormat(HRFRasterFileFactory::GetInstance()->GetCreator(HRFTiffFile::CLASS_ID));

        // Select best pixel type
        exporter.SelectBestPixelType(pBitmap->GetPixelType());
        assert(exporter.GetSelectedPixelType() == pBitmap->GetPixelType()->GetClassID());

        exporter.SelectBlockType(HRFBlockType::TILE);                    
        
        // Prefer single resolution file.
        exporter.SelectEncoding(HRFEncodingType(HRFEncodingType::STANDARD));
        
        if(NULL != exporter.StartExport())
            return true;
        }
    catch(...)
        {
        }

    return false;
    }