#pragma once
#include <ScalableMesh/ScalableMeshDefs.h>
#include <ImagePP/all/h/HRSObjectStore.h>

#include <ImagePP/all/h/HRARaster.h>
#ifndef VANCOUVER_API
#include <ImagePP/all/h/HRFMapboxFile.h>
#endif


#include <ImagePP/all/h/HRFiTiffCacheFileCreator.h>
#include <ImagePP/all/h/HRFUtility.h>
#include <ImagePP/all/h/HGFHMRStdWorldCluster.h>

#ifndef VANCOUVER_API
#define HMRWORLDCLUSTER ImagePP::HGFHMRStdWorldCluster
#define HRARASTER ImagePP::HRARaster
#define GCSCPTR GeoCoordinates::BaseGCSCPtr
#define GCSCP GeoCoordinates::BaseGCSCP
#else
#define HMRWORLDCLUSTER HGFHMRStdWorldCluster
#define HRARASTER HRARaster
#define GCSCPTR GeoCoordinates::BaseGCSPtr
#define GCSCP GeoCoordinates::BaseGCSP
#endif

USING_NAMESPACE_IMAGEPP
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

class RasterUtilities
    {
    private: 

        static HMRWORLDCLUSTER* s_cluster;

    public:

        BENTLEY_SM_EXPORT static ImagePP::HPMPool* s_rasterMemPool;

        BENTLEY_SM_EXPORT static HMRWORLDCLUSTER* GetWorldCluster();
        static ImagePP::HFCPtr<ImagePP::HRFRasterFile> LoadRasterFile(WString path);
        BENTLEY_SM_EXPORT static ImagePP::HFCPtr<HRARASTER> LoadRaster(WString path);
        BENTLEY_SM_EXPORT static ImagePP::HFCPtr<HRARASTER> LoadRaster(WString path, GCSCPTR targetCS, DRange2d extentInTargetCS);
        BENTLEY_SM_EXPORT static ImagePP::HFCPtr<HRARASTER> LoadRaster(HFCPtr<HRFRasterFile>& rasterFile, WString path, GCSCPTR targetCS, DRange2d extentInTargetCS, bool forceProjective = false);

        static StatusInt CopyFromArea(bvector<uint8_t>& texData, int width, int height, const DRange2d area, const float* textureResolution, HRARASTER& raster, bool isRGBA = false, bool addHeader = true);
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE