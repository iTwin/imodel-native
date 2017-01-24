/*--------------------------------------------------------------------------------------+
|
|
|   $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableMesh\IScalableMeshPublisher.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct SMCesiumPublisher : virtual public IScalableMeshPublisher
    {
    protected:

        void _Publish(IScalableMeshNodePtr node, const Transform& tranform, bvector<Byte>& outData) override;
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE