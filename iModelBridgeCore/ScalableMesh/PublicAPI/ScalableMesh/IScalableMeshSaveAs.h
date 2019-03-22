/*--------------------------------------------------------------------------------------+
|
|
|   $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/IScalableMesh.h>
#include <ScalableMesh/IScalableMeshPublisher.h>
#include <ScalableMesh/IScalableMeshProgress.h>
#include <ScalableMesh/IScalableMeshNodeCreator.h>
/*__PUBLISH_SECTION_END__*/
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
struct IScalableMeshSaveAs
{
public:
    BENTLEY_SM_IMPORT_EXPORT static StatusInt DoSaveAs(const IScalableMeshPtr& source, const WString& destination, ClipVectorPtr clips, IScalableMeshProgressPtr progress);
    BENTLEY_SM_IMPORT_EXPORT static StatusInt DoSaveAs(const IScalableMeshPtr& source, const WString& destination, ClipVectorPtr clips, IScalableMeshProgressPtr progress, const Transform& transform);
    BENTLEY_SM_IMPORT_EXPORT static StatusInt Generate3DTiles(const IScalableMeshPtr& meshP, const WString& outContainerName = L"", const WString& outDatasetName = L"", SMCloudServerType server = SMCloudServerType::LocalDisk, IScalableMeshProgressPtr progress = nullptr, ClipVectorPtr clips = nullptr, uint64_t coverageId = (uint64_t)-1);
    BENTLEY_SM_IMPORT_EXPORT static StatusInt Generate3DTiles(const IScalableMeshPtr& meshP, const WString& outContainerName = L"", const Transform& transform = Transform::FromIdentity(), const WString& outDatasetName = L"", SMCloudServerType server = SMCloudServerType::LocalDisk, IScalableMeshProgressPtr progress = nullptr, ClipVectorPtr clips = nullptr, uint64_t coverageId = (uint64_t)-1);

};
END_BENTLEY_SCALABLEMESH_NAMESPACE
