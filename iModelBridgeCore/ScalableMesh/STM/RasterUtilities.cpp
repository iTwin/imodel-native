#include "ScalableMeshPCH.h"
#include "ImagePPHeaders.h"
#include "RasterUtilities.h"
#include "ReprojectionModel.h"
#include <ImagePP\all\h\HRPPixelTypeV32R8G8B8A8.h>
#include <ImagePP/all/h/HRARaster.h>
#include <ImagePP/all/h/HIMMosaic.h>
#include <ImagePP/all/h/HPMPooledVector.h>
#include <ImagePP/all/h/HRAClearOptions.h>
#include <ImagePP/all/h/HRACopyFromOptions.h>
#include <ImagePP/all/h/HCDCodecIJG.h>
#include <ImagePP/all/h/HCDCodecIdentity.h>
#include <ImagePP\all\h\HCDPacket.h>
#include <ImagePP\all\h\HRFiTiffCacheFileCreator.h>
#include <ImagePP\all\h\HRFUtility.h>
#include <Imagepp/all/h/HRSObjectStore.h>
#include <ImagePP/all/h/HGF2DIdentity.h>
#include <ImagePP/all/h/HCPGCoordUtility.h>
//#include <ImagePP/all/h/HRFVirtualEarthFile.h>
#include <ImagePP/all/h/HRFVirtualEarthFile.h>
#include <ImagePP/all/h/HRFRasterFileCache.h>



BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
HPMPool* RasterUtilities::s_rasterMemPool = nullptr;

static bool s_useMapBox = true;

HFCPtr<HRFRasterFile> RasterUtilities::LoadRasterFile(WString path)
    {
    HFCPtr<HRFRasterFile> pRasterFile;
    HFCPtr<HFCURL> pImageURL(HFCURL::Instanciate(path));

#ifndef VANCOUVER_API
    if (HRFMapBoxCreator::GetInstance()->IsKindOfFile(pImageURL))
        {   
        if (s_useMapBox)
            pRasterFile = HRFMapBoxCreator::GetInstance()->Create(pImageURL, HFC_READ_ONLY);
        else
            pRasterFile = HRFVirtualEarthCreator::GetInstance()->Create(pImageURL, HFC_READ_ONLY);        
        }
    else
#endif
        {
        pRasterFile = HRFRasterFileFactory::GetInstance()->OpenFile(HFCURL::Instanciate(path), TRUE);
        }

    pRasterFile = GenericImprove(pRasterFile, HRFiTiffCacheFileCreator::GetInstance(), true, true);

#ifndef VANCOUVER_API
    if (HRFMapBoxCreator::GetInstance()->IsKindOfFile(pImageURL))
        {
        //pRasterFile = new HRFRasterFileCache(pRasterFile, HRFiTiffCacheFileCreator::GetInstance());
        }
#endif

    return pRasterFile;
    }

HGFHMRStdWorldCluster* RasterUtilities::s_cluster = nullptr;

HGFHMRStdWorldCluster* RasterUtilities::GetWorldCluster()
    {
    if (s_cluster == nullptr)
        s_cluster = new HGFHMRStdWorldCluster();

    return s_cluster;
    }


HFCPtr<HRARASTER> RasterUtilities::LoadRaster(WString path)
    {
    if (s_rasterMemPool == nullptr)
        s_rasterMemPool = new HPMPool(300000, HPMPool::None);
    auto cluster = GetWorldCluster();

    HFCPtr<HGF2DCoordSys>  pLogicalCoordSys;
    HFCPtr<HRSObjectStore> pObjectStore;
    HFCPtr<HRFRasterFile> pRasterFile = LoadRasterFile(path);


    pLogicalCoordSys = cluster->GetWorldReference(pRasterFile->GetPageWorldIdentificator(0));
    pObjectStore = new HRSObjectStore(s_rasterMemPool,
                                      pRasterFile,
                                      0,
                                      pLogicalCoordSys);

    // Get the raster from the store
    HFCPtr<HRARaster> rasterSource = pObjectStore->LoadRaster();
    return rasterSource;
    }


HFCPtr<HRARASTER> RasterUtilities::LoadRaster(WString path, GCSCPTR targetCS, DRange2d extentInTargetCS)
    {
    HFCPtr<HRFRasterFile> rasterFile;

    return LoadRaster(rasterFile, path, targetCS, extentInTargetCS);
    }

HFCPtr<HRARASTER> RasterUtilities::LoadRaster(HFCPtr<HRFRasterFile>& rasterFile, WString path, GCSCPTR targetCS, DRange2d extentInTargetCS)
    {

    if (s_rasterMemPool == nullptr)
        s_rasterMemPool = new HPMPool(300000, HPMPool::None);
    auto cluster = GetWorldCluster();

    HFCPtr<HRSObjectStore> pObjectStore;
    HFCPtr<HRFRasterFile> pRasterFile = LoadRasterFile(path);

	GCSCP pRasterGcs;
#ifndef VANCOUVER_API
     pRasterGcs = pRasterFile->GetPageDescriptor(0)->GetGeocodingCP();
#else
	pRasterGcs = pRasterFile->GetPageDescriptor(0)->GetGeocodingCP()->GetBaseGCS();
#endif

    HFCPtr<HGF2DTransfoModel> pReprojectionModel;
    if (pRasterGcs != nullptr && pRasterGcs->IsValid() && targetCS != nullptr && !targetCS->IsEquivalent(*pRasterGcs))
        {
        pReprojectionModel = new ReprojectionModel(*pRasterGcs, *targetCS);
        }
    else
        {
        pReprojectionModel = new HGF2DIdentity();   // assumed to be coincident        
        }

    HFCPtr<HGF2DTransfoModel> pRasterTransfoModel;
    if (pRasterFile->GetPageDescriptor(0)->HasTransfoModel())
        {
        pRasterTransfoModel = pRasterFile->GetPageDescriptor(0)->GetTransfoModel();
        }
    else
        {
        pRasterTransfoModel = new HGF2DIdentity();
        }

    HFCPtr<HGF2DCoordSys> pRasterWorldCS = cluster->GetCoordSysReference(pRasterFile->GetWorldIdentificator());
    HFCPtr<HGF2DCoordSys> pDgnCS = cluster->GetCoordSysReference(HGF2DWorld_HMRWORLD);
    HFCPtr<HGF2DTransfoModel> pRasterWorldToDgnWorldCS = pRasterWorldCS->GetTransfoModelTo(pDgnCS);

    HFCPtr<HGF2DCoordSys> pReprojCS = new HGF2DCoordSys(*pReprojectionModel, pDgnCS);

    // New CS to Reproj CS
    HFCPtr<HGF2DCoordSys> pRasterLogicalCS = new HGF2DCoordSys(*pRasterWorldToDgnWorldCS, pReprojCS);
    HFCPtr<HGF2DCoordSys> pRasterPhysCS = new HGF2DCoordSys(*pRasterTransfoModel, pRasterLogicalCS);
    // Express the size of one pixel in DgnCS (aka lowerLeft). We use image center.
    double imageCenterX = pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth() / 2.0;
    double imageCenterY = pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight() / 2.0;
    HFCPtr<HVEShape> pPixelShape(new HVEShape(imageCenterX - 0.5, imageCenterY - 0.5, imageCenterX + 0.5, imageCenterY + 0.5, pRasterPhysCS));
    pPixelShape->ChangeCoordSys(pDgnCS);
    HGF2DExtent pixelExtentInDgnCS(pPixelShape->GetExtent());

    HGF2DExtent imageExtent;

	#ifndef VANCOUVER_API
    // If the model doesn't preserve linearity try to simplify it. 
    if (!pReprojectionModel->PreservesLinearity())
        {
        // Compute image extent in source CS
        HVEShape rasterShape(0.0, 0.0, (double)pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth(), (double)pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight(), pRasterPhysCS);
        rasterShape.ChangeCoordSys(pReprojCS);
        imageExtent = rasterShape.GetExtent();

        double rect[10] = { extentInTargetCS.low.x, extentInTargetCS.low.y, extentInTargetCS.high.x, extentInTargetCS.low.y, extentInTargetCS.high.x, extentInTargetCS.high.y, extentInTargetCS.low.x, extentInTargetCS.high.y, extentInTargetCS.low.x, extentInTargetCS.low.y };

        size_t buflen = 10;

        HVEShape filterShape(&buflen, rect, pDgnCS);
        filterShape.ChangeCoordSys(pReprojCS);
        HGF2DExtent filterExtent = filterShape.GetExtent();

        imageExtent.Intersect(filterExtent);

        //intersect with real extent

        HGF2DLiteExtent imageLiteExtent(imageExtent.GetXMin(), imageExtent.GetYMin(), imageExtent.GetXMax(), imageExtent.GetYMax());

        double step = MIN(imageLiteExtent.GetHeight(), imageLiteExtent.GetWidth()) / 5;

        // Compute the expected mean error in destination CS
        double ExpectedMeanError = MIN(pixelExtentInDgnCS.GetWidth() * 0.5, pixelExtentInDgnCS.GetHeight() * 0.5);
        double ExpectedMaxError = MIN(pixelExtentInDgnCS.GetWidth(), pixelExtentInDgnCS.GetHeight());

        HFCPtr<HGF2DTransfoModel> pAdaptedModel = HCPGCoordUtility::CreateAdaptedModel(*pReprojectionModel, imageLiteExtent, step, ExpectedMeanError, ExpectedMaxError, nullptr, nullptr, nullptr, nullptr);
        if (pAdaptedModel != nullptr)
            {
            pReprojectionModel = pAdaptedModel->CreateSimplifiedModel();
            if (pReprojectionModel == nullptr)
                pReprojectionModel = pAdaptedModel; // cannot simplify adapted model.
            }

        // re-cook our CS to use the adapted model.
        pReprojCS = new HGF2DCoordSys(*pReprojectionModel, pDgnCS);
        pRasterLogicalCS = new HGF2DCoordSys(*pRasterWorldToDgnWorldCS, pReprojCS);
        }
    else
#endif
        {
        HVEShape rasterShape(0.0, 0.0, (double)pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth(), (double)pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight(), pRasterPhysCS);
        rasterShape.ChangeCoordSys(pReprojCS);
        imageExtent = rasterShape.GetExtent();
        }

    pObjectStore = new HRSObjectStore(s_rasterMemPool,
                                      pRasterFile,
                                      0,
                                      pRasterLogicalCS);

    // Get the raster from the store
    HFCPtr<HRARaster> rasterSource = pObjectStore->LoadRaster();
    
    HVEShape imageReprojectShape(imageExtent);
    rasterSource->SetShape(imageReprojectShape);

    rasterFile = pRasterFile;




    HFCPtr<HIMMosaic> mosaicPtr = new HIMMosaic(GetWorldCluster()->GetCoordSysReference(HGF2DWorld_HMRWORLD));
    mosaicPtr->Add(rasterSource);
    return mosaicPtr.GetPtr();


    }


StatusInt RasterUtilities::CopyFromArea(bvector<uint8_t>& texData, int width, int height, const DRange2d area, const float* textureResolution, HRARASTER& raster)
    {
    HFCMatrix<3, 3> transfoMatrix;
/*
    if (textureResolution != nullptr)
        transfoMatrix[0][0] = *textureResolution;
    else
*/
        transfoMatrix[0][0] = (area.high.x - area.low.x) / width;


    

    transfoMatrix[0][1] = 0;
    transfoMatrix[0][2] = area.low.x;
    transfoMatrix[1][0] = 0;
/*
    if (textureResolution != nullptr)
        transfoMatrix[1][1] = -*textureResolution;
    else
*/
        transfoMatrix[1][1] = -(area.high.y - area.low.y) / height;

    transfoMatrix[1][2] = area.high.y;
    transfoMatrix[2][0] = 0;
    transfoMatrix[2][1] = 0;
    transfoMatrix[2][2] = 1;

    HFCPtr<HGF2DTransfoModel> pTransfoModel((HGF2DTransfoModel*)new HGF2DProjective(transfoMatrix));

    HFCPtr<HGF2DTransfoModel> pSimplifiedModel = pTransfoModel->CreateSimplifiedModel();

    if (pSimplifiedModel != 0)
        {
        pTransfoModel = pSimplifiedModel;
        }

    HFCPtr<HRABitmap> pTextureBitmap;

    HFCPtr<HRPPixelType> pPixelType(new HRPPixelTypeV32R8G8B8A8());
#ifdef VANCOUVER_API
    HFCPtr<HCDCodec>     pCodec(new HCDCodecIdentity());
#endif
    texData.resize(3 * sizeof(int) + width * height * 3);

#ifdef VANCOUVER_API
    pTextureBitmap = new HRABitmap(width,
                                   height,
                                   pTransfoModel.GetPtr(),
                                   raster.GetCoordSys(),
                                   pPixelType,
                                   8,
                                   HRABitmap::UPPER_LEFT_HORIZONTAL,
                                   pCodec);
#else
    pTextureBitmap = HRABitmap::Create(width,
                                       height,
                                       pTransfoModel.GetPtr(),
                                       raster.GetCoordSys(),
                                       pPixelType,
                                       8);
#endif

    byte* pixelBufferPRGBA = new byte[width * height * 4];
    pTextureBitmap->GetPacket()->SetBuffer(pixelBufferPRGBA, width * height * 4);
    pTextureBitmap->GetPacket()->SetBufferOwnership(false);

    HRAClearOptions clearOptions;

    //green color when no texture is available
    uint32_t green;

    ((uint8_t*)&green)[0] = 0;
    ((uint8_t*)&green)[1] = 0x77;
    ((uint8_t*)&green)[2] = 0;

    clearOptions.SetRawDataValue(&green);

    pTextureBitmap->Clear(clearOptions);

    HRACopyFromOptions copyFromOptions;

    //Rasterlib set this option on the last tile of a row or a column to avoid black lines.     
    copyFromOptions.SetAlphaBlend(true);

#ifdef VANCOUVER_API
    copyFromOptions.SetGridShapeMode(true);
    pTextureBitmap->CopyFrom(&raster, copyFromOptions);
#else
    pTextureBitmap->CopyFrom(raster, copyFromOptions);
#endif


    Byte *pPixel = &texData[0] + 3 * sizeof(int);

    int nChannels = 3;
    memcpy(&texData[0], &width, sizeof(int));
    memcpy(&texData[0] + sizeof(int), &height, sizeof(int));
    memcpy(&texData[0] + 2 * sizeof(int), &nChannels, sizeof(int));


    for (size_t i = 0; i < width*height; ++i)
        {
        *pPixel++ = pixelBufferPRGBA[i * 4];
        *pPixel++ = pixelBufferPRGBA[i * 4 + 1];
        *pPixel++ = pixelBufferPRGBA[i * 4 + 2];
        }
    delete[] pixelBufferPRGBA;
    pTextureBitmap = 0;
    return SUCCESS;
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE
