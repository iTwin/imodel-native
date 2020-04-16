/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ScalableMeshPCH.h"
#include <ScalableMesh/IScalableMeshDetectGround.h>
#include <ScalableMesh/ScalableMeshUtilityFunctions.h>
#include <ScalableMesh/GeoCoords/GCS.h>

using namespace BENTLEY_NAMESPACE_NAME::GeoCoordinates;
USING_NAMESPACE_BENTLEY_SCALABLEMESH

SMStatus IScalableMeshDetectGround::DetectGroundForRegion(IScalableMeshPtr& source, BeFileName& createdTerrain, const BeFileName& coverageTempDataFolder, const bvector<DPoint3d>& coverageData, uint64_t id, IScalableMeshGroundPreviewerPtr groundPreviewer, BaseGCSCPtr& destinationGcs, bool limitResolution, bool reprojectElevation, const BeFileName& dataSourceDir)
{
    BeFileName terrainAbsName;

    Utf8String coverageName(createdTerrain);

#ifndef VANCOUVER_API
    WString str(source->GetEditFilesBasePath().c_str(), true);
#else
    WString str(source->GetEditFilesBasePath().c_str());
#endif

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

        smGroundExtractor->SetReprojectElevation(reprojectElevation);

        if (!dataSourceDir.empty())
            {
            smGroundExtractor->SetDataSourceDir(dataSourceDir);
            }
        
        SMStatus status = smGroundExtractor->ExtractAndEmbed(coverageTempDataFolder);
        if (status != SMStatus::S_SUCCESS)
            return status;

    createdTerrain = terrainAbsName;
    return SMStatus::S_SUCCESS;
}
