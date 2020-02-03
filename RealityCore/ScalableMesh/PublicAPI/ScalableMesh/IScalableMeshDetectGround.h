/*--------------------------------------------------------------------------------------+
|
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
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
    static BENTLEY_SM_IMPORT_EXPORT SMStatus DetectGroundForRegion( IScalableMeshPtr& source, BeFileName& createdTerrain, const BeFileName& coverageTempDataFolder, const bvector<DPoint3d>& coverageData, uint64_t id, IScalableMeshGroundPreviewerPtr groundPreviewer, BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& destinationGcs, bool limitResolution, bool reprojectElevation, const BeFileName& dataSourceDir);

};
END_BENTLEY_SCALABLEMESH_NAMESPACE
