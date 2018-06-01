/*--------------------------------------------------------------------------------------+
|
|
|   $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/IScalableMesh.h>
#include <ScalableMesh/IScalableMeshProgress.h>
#include <ScalableMesh/IScalableMeshSourceCreator.h>
#include <ScalableMesh/IScalableMeshGroundExtractor.h>
/*__PUBLISH_SECTION_END__*/
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
struct IScalableMeshDetectGround
{
    static BENTLEY_SM_IMPORT_EXPORT SMStatus DetectGroundForRegion( IScalableMeshPtr& source, BeFileName& createdTerrain, const BeFileName& coverageTempDataFolder, const bvector<DPoint3d>& coverageData, uint64_t id, IScalableMeshGroundPreviewerPtr groundPreviewer, BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& destinationGcs, bool limitResolution);

};
END_BENTLEY_SCALABLEMESH_NAMESPACE
