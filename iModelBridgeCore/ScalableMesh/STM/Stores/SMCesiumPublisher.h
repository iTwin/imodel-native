/*--------------------------------------------------------------------------------------+
|
|
|   Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableMesh\IScalableMeshPublisher.h>
#include "SMPublisher.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct SMCesiumPublisher : virtual public IScalableMeshPublisher
    {
    protected:        

        StatusInt _Publish(IScalableMeshPublishParamsPtr params) override { return ERROR; }
        void _Publish(IScalableMeshNodePtr node, const Transform& tranform, bvector<Byte>& outData, bool outputTexture) override;
        void _Publish(IScalableMeshNodePtr nodePtr, ClipVectorPtr clips, const uint64_t& coverageID, bool isClipBoundary, const Transform& transform, bvector<Byte>& outData, bool outputTexture) override;
        void _ExtractPublishNodeHeader(IScalableMeshNodePtr nodePtr, Json::Value& smHeader) override;
        void _ExtractPublishMasterHeader(IScalableMeshPtr smPtr, Json::Value& smMasterHeader) override;
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE