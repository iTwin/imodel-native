#include <ScalableMeshPCH.h>

#include "ImagePPHeaders.h"
#include "ScalableMesh.h"

#include "ScalableMeshQuadTreeBCLIBFilters.h"
#include "ScalableMeshQuadTreeQueries.h"
#include "ScalableMeshQuadTreeQueries.hpp"

USING_NAMESPACE_BENTLEY_SCALABLEMESH

//template class ScalableMeshQuadTreeBCLIBFilterViewDependent<DPoint3d, IDTMFile::Extent3d64f>;

template class ScalableMeshQuadTreeViewDependentPointQuery<DPoint3d, IDTMFile::Extent3d64f>;

template class ScalableMeshQuadTreeLevelPointIndexQuery<DPoint3d, IDTMFile::Extent3d64f>;

template class ScalableMeshQuadTreeLevelMeshIndexQuery<DPoint3d, IDTMFile::Extent3d64f>;

template class ScalableMeshQuadTreeLevelIntersectIndexQuery<DPoint3d, IDTMFile::Extent3d64f>;

template class ScalableMeshQuadTreeLevelPlaneIntersectIndexQuery<DPoint3d, IDTMFile::Extent3d64f>;

template class ScalableMeshQuadTreeViewDependentMeshQuery<DPoint3d, IDTMFile::Extent3d64f>;








