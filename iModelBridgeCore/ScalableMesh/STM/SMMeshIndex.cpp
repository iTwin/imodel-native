// SMMeshIndex.cpp

#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"

#include "Edits/ClipUtilities.hpp"
#include "ScalableMesh/ScalableMeshGraph.h"
#include "ScalableMesh.h"
#include "SMPointIndex.h"
#include "SMMeshIndex.h"
#include "SMMeshIndex.hpp"

//template class SMPointIndex<DPoint3d, IDTMFile::Extent3d64f>;

//template void SMMeshIndexNode<DPoint3d, IDTMFile::Extent3d64f>::SplitMeshForChildNodes();

//template void Bentley::ScalableMesh::ClipMeshToNodeRange<DPoint3d, IDTMFile::Extent3d64f>(vector<int>& faceIndexes, vector<DPoint3d>& nodePts, bvector<DPoint3d>& pts, IDTMFile::Extent3d64f& contentExtent, DRange3d& nodeRange, ScalableMeshMesh* meshP);

template class SMMeshIndex<DPoint3d, IDTMFile::Extent3d64f>;

template class SMMeshIndexNode<DPoint3d, IDTMFile::Extent3d64f>;

template class ISMPointIndexMesher<DPoint3d, IDTMFile::Extent3d64f>;