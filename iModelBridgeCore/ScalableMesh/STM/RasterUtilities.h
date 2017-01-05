#pragma once
#include <ScalableMesh/ScalableMeshDefs.h>
#include <Imagepp/all/h/HRSObjectStore.h>

#include <ImagePP/all/h/HRARaster.h>
#include <ImagePP\all\h\HRFMapboxFile.h>


#include <ImagePP\all\h\HRFiTiffCacheFileCreator.h>
#include <ImagePP\all\h\HRFUtility.h>
#include <ImagePP\all\h\HGFHMRStdWorldCluster.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

class RasterUtilities
    {
    private: 

        static ImagePP::HGFHMRStdWorldCluster* s_cluster;

    public:

        static HPMPool* s_rasterMemPool;

        static ImagePP::HGFHMRStdWorldCluster* GetWorldCluster();
        static HFCPtr<HRFRasterFile> LoadRasterFile(WString path);
        static HFCPtr<ImagePP::HRARaster> LoadRaster(WString path);
        static HFCPtr<ImagePP::HRARaster> LoadRaster(WString path, GeoCoordinates::BaseGCSCPtr targetCS, DRange2d extentInTargetCS);

        static StatusInt CopyFromArea(bvector<uint8_t>& texData, int width, int height, const DRange2d area, ImagePP::HRARaster& raster);
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE