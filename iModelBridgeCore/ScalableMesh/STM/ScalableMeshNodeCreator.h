/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/IScalableMeshNodeCreator.h>

#include "ScalableMeshCreator.h"


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*---------------------------------------------------------------------------------**//**
 * @description
 * @bsiclass                                                  Elenie.Godzaridis   07/2015
 +---------------+---------------+---------------+---------------+---------------+------*/
struct IScalableMeshNodeCreator::Impl : public IScalableMeshCreator::Impl
    {
    private:
        friend struct                       IScalableMeshNodeCreator;

    public:
        explicit                            Impl(const WChar*                          scmFileName);
        explicit                            Impl(const IScalableMeshPtr&                        iDTMFilePtr);

        ~Impl();
        IScalableMeshNodeEditPtr AddChildNode(const IScalableMeshNodePtr& parentNode,
                                          DRange3d& childExtent,
                                                       SMStatus&   status,
                                                       bool computeNodeID = true,
                                                       uint64_t nodeId = 0);

        IScalableMeshNodeEditPtr AddNode(StatusInt&   status,
                                         bool computeNodeID = true,
                                         uint64_t nodeId = 0);

        void NotifyAllChildrenAdded(const IScalableMeshNodePtr& parentNode,
                                    StatusInt&                  status,
                                    bool computeNeighbors);

        int64_t AddTextureCompressed(int width, int height, int nOfChannels, const byte* texData, size_t compressedSize);

        int64_t AddTexture(int width, int height, int nOfChannels, const byte* texData);
        
        void AddTexture(int width, int height, int nOfChannels, const byte* texData, int64_t texID);

        void SetTextured(SMTextureType textured);

        void SetIsSingleFile(bool isSingleFile);

        void SetIsTerrain(bool isTerrain);

        void SetDataResolution(float resolution);

        virtual StatusInt                           CreateScalableMesh(bool isSingleFile = true, bool restrictLevelForPropagation= false, bool doPartialUpdate = false) override;

    };

END_BENTLEY_SCALABLEMESH_NAMESPACE