/*--------------------------------------------------------------------------------------+
|
|
|   $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableMesh\IScalableMeshPublisher.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct SMCesiumPublisher : virtual public IScalableMeshPublisher
    {
    protected:        

        void _Publish(IScalableMeshNodePtr node, const Transform& tranform, bvector<Byte>& outData, bool outputTexture) override;
        void _Publish(IScalableMeshNodePtr nodePtr, ClipVectorPtr clips, const uint64_t& coverageID, bool isClipBoundary, GeoCoordinates::BaseGCSCPtr sourceGCS, GeoCoordinates::BaseGCSCPtr destinationGCS, bvector<Byte>& outData, bool outputTexture) override;
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE