/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshAnalysis.h $
|       $Date: 2016/08/23 10:33:32 $
|     $Author:Stephane.Nullans $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once


#include <ScalableMesh/IScalableMeshAnalysis.h>
#include <ScalableMesh/IScalableMesh.h>
#include "ImagePPHeaders.h"
#include "SMMeshIndex.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


class ScalableMeshAnalysis : public IScalableMeshAnalysis
    {
    private:
        IScalableMesh* m_scmPtr;

        void _CreateCutVolumeRanges(SMVolumeSegment& segment, bvector<BENTLEY_NAMESPACE_NAME::TerrainModel::DTMRayIntersection>& _IPoints, DPoint3d& median, DVec3d& direction);
        void _CreateFillVolumeRanges(SMVolumeSegment& segment, bvector<BENTLEY_NAMESPACE_NAME::TerrainModel::DTMRayIntersection>& _IPoints, DPoint3d& median, DVec3d& direction);

    protected:
        virtual DTMStatusInt _ComputeDiscreteVolume(const bvector<DPoint3d>& polygon, double resolution, ISMGridVolume& grid) override;
        virtual DTMStatusInt _ComputeDiscreteVolume(const bvector<DPoint3d>& polygon, IScalableMeshNodePtr anotherMesh, double resolution, ISMGridVolume& grid) override;

    public:
        ScalableMeshAnalysis(IScalableMesh* scmPtr);
        ~ScalableMeshAnalysis();

        static ScalableMeshAnalysis* Create(IScalableMesh* scmPtr);
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE