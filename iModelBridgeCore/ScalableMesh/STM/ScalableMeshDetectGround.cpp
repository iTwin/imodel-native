#include "ScalableMeshPCH.h"
#include <ScalableMesh/IScalableMeshDetectGround.h>
#include <ScalableMesh/ScalableMeshUtilityFunctions.h>
#include <ScalableMesh/GeoCoords/GCS.h>

using namespace BENTLEY_NAMESPACE_NAME::GeoCoordinates;

SMStatus IScalableMeshDetectGround::DetectGroundForRegion(IScalableMeshPtr& source, BeFileName& createdTerrain, const BeFileName& coverageTempDataFolder, const bvector<DPoint3d>& coverageData, uint64_t id, IScalableMeshGroundPreviewerPtr groundPreviewer, BaseGCSCPtr& destinationGcs, bool limitResolution)
{
    BeFileName terrainAbsName;

    Utf8String coverageName(createdTerrain);

    WString str(source->GetEditFilesBasePath().c_str());
    GetCoverageTerrainAbsFileName(terrainAbsName, str, coverageName);

#ifndef VANCOUVER_API
    assert(!terrainAbsName.DoesPathExist());
#else    
    assert(!BeFileName::DoesPathExist(terrainAbsName.c_str()));
#endif


        IScalableMeshGroundExtractorPtr smGroundExtractor(IScalableMeshGroundExtractor::Create(terrainAbsName, source));

        BaseGCSPtr newDestPtr = (BaseGCS*)destinationGcs.get();
        smGroundExtractor->SetDestinationGcs(newDestPtr);
        smGroundExtractor->SetExtractionArea(coverageData);
        smGroundExtractor->SetGroundPreviewer(groundPreviewer);
        smGroundExtractor->SetLimitTextureResolution(limitResolution);

        StatusInt status = smGroundExtractor->ExtractAndEmbed(coverageTempDataFolder);

        if (status != SUCCESS)
            return status == SUCCESS ? SMStatus::S_SUCCESS : SMStatus::S_ERROR;



    createdTerrain = terrainAbsName;
    return SMStatus::S_SUCCESS;
}
