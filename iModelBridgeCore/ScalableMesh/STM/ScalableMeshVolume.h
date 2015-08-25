/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshVolume.h $
|    $RCSfile: ScalableMeshVolume.h,v $
|   $Revision: 1.0 $
|       $Date: 2015/04/20 14:27:27 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh/IScalableMesh.h>
#include "ScalableMeshQuery.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct ScalableMeshVolume : IDTMVolume
    {
    private:
        IScalableMeshPtr m_scmPtr;
        double _ComputeVolumeCutAndFillForTile(IScalableMeshMeshPtr smTile, double& cut, double& fill, PolyfaceHeader& mesh, bool is2d, DRange3d meshExtent, bvector<PolyfaceHeaderPtr>& volumeMeshVector);
    protected:
        //virtual DTMStatusInt _ComputeVolumeCutAndFill(double& cut, double& fill, double& area, PolyfaceHeader& intersectingMeshSurface, DRange3d& meshRange) override;
        virtual DTMStatusInt _ComputeVolumeCutAndFill(double& cut, double& fill, double& area, PolyfaceHeader& intersectingMeshSurface, DRange3d& meshRange, bvector<PolyfaceHeaderPtr>& volumeMeshVector) override;
        virtual DTMStatusInt _ComputeVolumeCutAndFill(PolyfaceHeaderPtr& terrainMesh, double& cut, double& fill, PolyfaceHeader& mesh, bool is2d, bvector<PolyfaceHeaderPtr>& volumeMeshVector) override;
    public:
        ScalableMeshVolume(IScalableMeshPtr scMesh);
        ScalableMeshVolume(){}
        //double ComputeVolumeCutAndFillForTile(PolyfaceHeaderPtr terrainMesh, double& cut, double& fill, PolyfaceHeader& mesh, bool is2d, bvector<PolyfaceHeaderPtr>& volumeMeshVector);
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
