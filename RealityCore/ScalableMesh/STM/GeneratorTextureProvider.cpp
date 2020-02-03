/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ScalableMeshPCH.h"
#include <ScalableMesh/IScalableMeshTextureGenerator.h>
#include "GeneratorTextureProvider.h"
#include "MosaicTextureProvider.h"
#include "ImagePPHeaders.h"
#include <ImagePP/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <ImagePP/all/h/HRARaster.h>
#include <ImagePP/all/h/HRSObjectStore.h>
#include <ImagePP/all/h/HIMMosaic.h>
#include <ImagePP/all/h/HGF2DCoordSys.h>
#include <ImagePP/all/h/HRFiTiffCacheFileCreator.h>
#include <ImagePP/all/h/HRFUtility.h>
#include <ImagePP/all/h/HCDPacket.h>
#include <Bentley/BeDirectoryIterator.h>

static HPMPool* s_rasterGenMemPool = nullptr;

USING_NAMESPACE_BENTLEY_SCALABLEMESH

DPoint2d GeneratorTextureProvider::_GetMinPixelSize()
    {
    DPoint2d minSize = DPoint2d::From(m_minPixelSize, m_minPixelSize);

    return minSize;
    }

DRange2d GeneratorTextureProvider::_GetTextureExtent()
    {
    DRange2d rasterBox = DRange2d::From(m_extent.low.x, m_extent.low.y,
                                        m_extent.high.x, m_extent.high.y);
    return rasterBox;
    }

StatusInt GeneratorTextureProvider::_GetTextureForArea(bvector<uint8_t>& texData, int width, int height, DRange2d& area)
    {
    m_generator->SetPixelSize(std::max(std::min(area.XLength() / width, area.YLength() / height), m_minPixelSize));
    m_generator->SetTextureTempDir(m_dir);
    bvector<DPoint3d> closedPolygonPoints;
    DPoint3d rangePts[5] = { DPoint3d::From(area.low.x, area.low.y, 0), DPoint3d::From(area.low.x, area.high.y, 0), DPoint3d::From(area.high.x, area.high.y, 0),
        DPoint3d::From(area.high.x, area.low.y, 0), DPoint3d::From(area.low.x, area.low.y, 0) };
    closedPolygonPoints.assign(rangePts, rangePts + 5);

    m_generator->GenerateTexture(closedPolygonPoints);

    BeDirectoryIterator directoryIter(m_dir);

    BeFileName currentTextureName;
    bool       isDir;
    if(s_rasterGenMemPool == nullptr) s_rasterGenMemPool = new HPMPool(30000, HPMPool::None);
    auto cluster = new HGFHMRStdWorldCluster();
    HFCPtr<HIMMosaic> pMosaicP = new HIMMosaic(HFCPtr<HGF2DCoordSys>(cluster->GetWorldReference(HGF2DWorld_HMRWORLD).GetPtr()));
    HIMMosaic::RasterList rasterList;
    while (SUCCESS == directoryIter.GetCurrentEntry(currentTextureName, isDir))
        {
#ifdef VANCOUVER_API
        if (0 == BeFileName::GetExtension(currentTextureName.c_str()).CompareToI(L"jpg"))
#else
        if (0 == currentTextureName.GetExtension().CompareToI(L"jpg"))
#endif
            {
            WString path = WString(L"file://") + currentTextureName;            
            
            HFCPtr<HGF2DCoordSys>  pLogicalCoordSys;
            HFCPtr<HRSObjectStore> pObjectStore;
            HFCPtr<HRFRasterFile>  pRasterFile;
            HFCPtr<HRARaster>      pRaster;

#if defined(DGNDB06_API) || defined(VANCOUVER_API)
            pRasterFile = HRFRasterFileFactory::GetInstance()->OpenFile(HFCURL::Instanciate(path), TRUE);
#else
            Utf8String pathUtf8(path);
            pRasterFile = HRFRasterFileFactory::GetInstance()->OpenFile(HFCURL::Instanciate(pathUtf8), TRUE);
#endif

            pRasterFile = GenericImprove(pRasterFile,/* HRFiTiffCacheFileCreator::GetInstance()*/NULL, true, true);

            pLogicalCoordSys = cluster->GetWorldReference(pRasterFile->GetPageWorldIdentificator(0));
            pObjectStore = new HRSObjectStore(s_rasterGenMemPool,
                                              pRasterFile,
                                              0,
                                              pLogicalCoordSys);

            // Get the raster from the store
            pRaster = pObjectStore->LoadRaster();

            HASSERT(pRaster != NULL);

            rasterList.push_back(pRaster.GetPtr());
            pRaster = 0;
            }
        directoryIter.ToNext();
        }
    pMosaicP->Add(rasterList);
    rasterList.clear();
    delete cluster;
    auto provider = new MosaicTextureProvider(pMosaicP);
    StatusInt status = provider->GetTextureForArea(texData, width, height, area);
    pMosaicP = 0;

    // NEEDS_WORK_SM : Do we really want to keep the directory?
#ifdef VANCOUVER_API
    BeFileName::EmptyAndRemoveDirectory(m_dir.c_str());
    BeFileName::CreateNewDirectory(m_dir.c_str());
#else
    // NEEDS_WORK_SM : Doesn't delete directory, only its contents...
    BeFileName::EmptyDirectory(m_dir.c_str());
#endif
    return status;
    }

GeneratorTextureProvider::GeneratorTextureProvider(IScalableMeshTextureGeneratorPtr& generator, DRange3d coveredExtent, double minPixelSize, BeFileName tempDirectory)
    : m_generator(generator), m_extent(coveredExtent), m_minPixelSize(minPixelSize), m_dir(tempDirectory)
    {

    }
