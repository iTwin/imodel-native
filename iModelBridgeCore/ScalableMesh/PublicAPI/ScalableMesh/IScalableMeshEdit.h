/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshEdit.h $
|       $Date: 2016/08/23 10:24:32 $
|     $Author:Elenie.Godzaridis $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#include <ScalableMesh/ScalableMeshDefs.h>
#include <Geom\Polyface.h>
#include <Bentley\RefCounted.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/ClipVector.h>
#include <ScalableMesh/IScalableMeshQuery.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
/*__PUBLISH_SECTION_END__*/
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE



class IScalableMeshEdit abstract : RefCountedBase
    {
    protected:
        virtual int _RemoveWithin(ClipVectorCP clipPlaneSet, const bvector<IScalableMeshNodePtr>& priorityNodes) = 0;

    public:
        BENTLEY_SM_EXPORT int RemoveWithin(ClipVectorCP clipPlaneSet);

        BENTLEY_SM_EXPORT int RemoveWithin(ClipVectorCP clipPlaneSet, const bvector<IScalableMeshNodePtr>& priorityNodes);
    };

typedef RefCountedPtr<IScalableMeshEdit>                          IScalableMeshEditPtr;

END_BENTLEY_SCALABLEMESH_NAMESPACE
