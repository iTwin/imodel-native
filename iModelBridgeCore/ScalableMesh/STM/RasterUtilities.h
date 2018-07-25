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
#include "ImagePPHeaders.h"

#ifndef VANCOUVER_API
#define HMRWORLDCLUSTER ImagePP::HGFHMRStdWorldCluster
#define HRARASTER ImagePP::HRARaster
#define GCSCPTR GeoCoordinates::BaseGCSCPtr
#define GCSCP GeoCoordinates::BaseGCSCP
#define HPMPOOL ImagePP::HPMPool 
#define HRFRASTERFILE ImagePP::HRFRasterFile
#else
#define HMRWORLDCLUSTER HGFHMRStdWorldCluster
#define HRARASTER HRARaster
#define GCSCPTR GeoCoordinates::BaseGCSPtr
#define GCSCP GeoCoordinates::BaseGCSP
#define HPMPOOL HPMPool 
#define HRFRASTERFILE HRFRasterFile
#endif

USING_NAMESPACE_IMAGEPP
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

class RasterUtilities
    {
    private: 

        static HMRWORLDCLUSTER* s_cluster;

    public:

        BENTLEY_SM_EXPORT static HPMPOOL* s_rasterMemPool;

        BENTLEY_SM_EXPORT static HMRWORLDCLUSTER* GetWorldCluster();
        static HFCPTR<HRFRASTERFILE> LoadRasterFile(WString path);
        BENTLEY_SM_EXPORT static HFCPTR<HRARASTER> LoadRaster(WString path);
        BENTLEY_SM_EXPORT static HFCPTR<HRARASTER> LoadRaster(WString path, GCSCPTR targetCS, DRange2d extentInTargetCS, GCSCPTR replacementGcsPtr = GCSCPTR());
        BENTLEY_SM_EXPORT static HFCPTR<HRARASTER> LoadRaster(HFCPTR<HRFRASTERFILE>& rasterFile, WString path, GCSCPTR targetCS, DRange2d extentInTargetCS, bool forceProjective = false, GCSCPTR replacementGcsPtr = GCSCPTR());

        static StatusInt CopyFromArea(bvector<uint8_t>& texData, int width, int height, const DRange2d area, const float* textureResolution, HRARASTER& raster, bool isRGBA = false, bool addHeader = true);
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE