/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshEdit.h $
|       $Date: 2016/08/23 10:24:32 $
|     $Author:Elenie.Godzaridis $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#include <ScalableMesh/ScalableMeshDefs.h>
#include <Geom/Polyface.h>
#include <Bentley/RefCounted.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/ClipVector.h>
#include <ScalableMesh/IScalableMeshQuery.h>

#if defined(VANCOUVER_API) || defined(DGNDB06_API)
    USING_NAMESPACE_BENTLEY_DGNPLATFORM
#else
    USING_NAMESPACE_BENTLEY_DGN
#endif

/*__PUBLISH_SECTION_END__*/
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*
#ifdef VANCOUVER_API
#define CLIP_VECTOR_NAMESPACE BENTLEY_NAMESPACE_NAME
#else
#define CLIP_VECTOR_NAMESPACE BENTLEY_NAMESPACE_NAME::Dgn
#endif
*/


class IScalableMeshEdit  : public RefCountedBase
    {
    protected:
        virtual int _RemoveWithin(CLIP_VECTOR_NAMESPACE::ClipVectorCP clipPlaneSet, const bvector<IScalableMeshNodePtr>& priorityNodes) = 0;

        virtual void _SmoothNode(const DPlane3d& sourceGeometry, const bvector<size_t>& targetedIndices, IScalableMeshNodePtr& node) = 0;

        virtual void _SmoothNode(const DPoint3d& center, double radius, const DVec3d& direction, double height, const bvector<size_t>& targetedIndices, IScalableMeshNodePtr& node) = 0;

        virtual void _Smooth(const DPlane3d& sourceGeometry) = 0;

    public:
        BENTLEY_SM_EXPORT int RemoveWithin(CLIP_VECTOR_NAMESPACE::ClipVectorCP clipPlaneSet);

        BENTLEY_SM_EXPORT int RemoveWithin(CLIP_VECTOR_NAMESPACE::ClipVectorCP clipPlaneSet, const bvector<IScalableMeshNodePtr>& priorityNodes);

        BENTLEY_SM_EXPORT void Smooth(const DPlane3d& sourceGeometry);

        BENTLEY_SM_EXPORT void SmoothNode(const DPlane3d& sourceGeometry, IScalableMeshNodePtr& node);

        BENTLEY_SM_EXPORT void SmoothNode(const DPoint3d& center, double radius, const DVec3d& direction, double height, IScalableMeshNodePtr& node);

        BENTLEY_SM_EXPORT void SmoothNode(const DPlane3d& sourceGeometry, const bvector<size_t>& targetedIndices, IScalableMeshNodePtr& node);
    };

typedef RefCountedPtr<IScalableMeshEdit>                          IScalableMeshEditPtr;

END_BENTLEY_SCALABLEMESH_NAMESPACE
