/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshNodeCreator.h $
|    $RCSfile: ScalableMeshNodeCreator.h,v $
|   $Revision: 1.45 $
|       $Date: 2015/07/15 22:03:24 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

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