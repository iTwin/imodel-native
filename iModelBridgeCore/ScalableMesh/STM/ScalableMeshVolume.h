/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshVolume.h $
|    $RCSfile: ScalableMeshVolume.h,v $
|   $Revision: 1.0 $
|       $Date: 2015/04/20 14:27:27 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
        IScalableMesh* m_scmPtr;
        bool hasRestrictions;
        uint64_t m_restrictedId;
        Transform m_transform;
        Transform m_UorsToStorage;

        double _ComputeVolumeCutAndFillForTile(IScalableMeshMeshPtr smTile, double& cut, double& fill, PolyfaceHeader& mesh, bool is2d, DRange3d meshExtent, bvector<PolyfaceHeaderPtr>& volumeMeshVector);
    protected:

        virtual DTMStatusInt _ComputeCutFillVolume(double* cut, double* fill, double* volume, PolyfaceHeaderCP mesh) override;
        virtual DTMStatusInt _ComputeCutFillVolumeClosed(double* cut, double* fill, double* volume, PolyfaceHeaderCP mesh) override;
        virtual bool _RestrictVolumeToRegion(uint64_t regionId) override;
        virtual void _RemoveAllRestrictions() override;
        
        DTMStatusInt _ComputeVolumeCutAndFill(double& cut, double& fill, double& area, PolyfaceHeader& intersectingMeshSurface, DRange3d& meshRange);
        DTMStatusInt _ComputeVolumeCutAndFill(double& cut, double& fill, double& area, PolyfaceHeader& intersectingMeshSurface, DRange3d& meshRange, bvector<PolyfaceHeaderPtr>& volumeMeshVector);
        DTMStatusInt _ComputeVolumeCutAndFill(PolyfaceHeaderPtr& terrainMesh, double& cut, double& fill, PolyfaceHeader& mesh, bool is2d, bvector<PolyfaceHeaderPtr>& volumeMeshVector);
    public:
        ScalableMeshVolume(IScalableMeshPtr scMesh);
        ScalableMeshVolume() : m_transform(Transform::FromIdentity()), m_UorsToStorage(Transform::FromIdentity()) {}

        void SetTransform(TransformR transform)
            {
            m_transform = transform;
#ifndef VANCOUVER_API
             m_UorsToStorage = m_transform.ValidatedInverse();
#else
            m_UorsToStorage.InverseOf(m_transform);
#endif
            }
        //double ComputeVolumeCutAndFillForTile(PolyfaceHeaderPtr terrainMesh, double& cut, double& fill, PolyfaceHeader& mesh, bool is2d, bvector<PolyfaceHeaderPtr>& volumeMeshVector);
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
