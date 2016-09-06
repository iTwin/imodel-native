#pragma once

#include <DgnPlatform/ClipVector.h>
#include <ScalableMesh/IScalableMesh.h>
#include <TerrainModel/TerrainModel.h>
#include <Geom/GeomApi.h>
#include <Geom/Polyface.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_BENTLEY_TERRAINMODEL

StatusInt ComputeVolumeForAgenda(/*BENTLEY_NAMESPACE_NAME::DRange3d& elemRange, */PolyfaceHeaderPtr meshData, IScalableMeshPtr smPtr, double& cut, double& fill, double& volume/*, bvector<PolyfaceHeaderPtr>& volumeMeshVector*/);
//StatusInt ComputeVolumeForAgenda(ElementAgendaR agenda, IScalableMeshPtr smPtr, ElementAgendaR agendaGround, double& cut, double& fill, double& volume, bvector<PolyfaceHeaderPtr>& volumeMeshVector);

double ComputeVolumeCutAndFill(DTMPtr smPtr, double& cut, double& fill, PolyfaceHeader& mesh/*, DRange3d& elemRange, bvector<PolyfaceHeaderPtr>& volumeMeshVector*/);
//double ComputeVolumeCutAndFill(DTMPtr smPtr, double& cut, double& fill, PolyfaceHeaderPtr& meshGround, PolyfaceHeader& mesh, DRange3d& elemRange, bvector<PolyfaceHeaderPtr>& volumeMeshVector);