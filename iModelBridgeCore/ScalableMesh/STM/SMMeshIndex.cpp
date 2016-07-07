// SMMeshIndex.cpp

#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"
USING_NAMESPACE_IMAGEPP
#include "Edits/ClipUtilities.hpp"
#include "ScalableMesh/ScalableMeshGraph.h"
#include "ScalableMesh.h"
#include "SMPointIndex.h"
#include "SMMeshIndex.h"
#include "SMMeshIndex.hpp"

//template class SMPointIndex<DPoint3d, ISMStore::Extent3d64f>;

//template void SMMeshIndexNode<DPoint3d, ISMStore::Extent3d64f>::SplitMeshForChildNodes();

//template void BENTLEY_NAMESPACE_NAME::ScalableMesh::ClipMeshToNodeRange<DPoint3d, ISMStore::Extent3d64f>(vector<int>& faceIndexes, vector<DPoint3d>& nodePts, bvector<DPoint3d>& pts, ISMStore::Extent3d64f& contentExtent, DRange3d& nodeRange, ScalableMeshMesh* meshP);

template class SMMeshIndex<DPoint3d, DRange3d>;

template class SMMeshIndexNode<DPoint3d, DRange3d>;

template class ISMPointIndexMesher<DPoint3d, DRange3d>;

template<typename T> size_t GetSizeInMemory(T* item)
    {
    return sizeof(T);
    }


template<> size_t GetSizeInMemory<MTGGraph>(MTGGraph* item)
    {
    size_t count = 0;
    count += sizeof(*item);
    count += (sizeof(MTGLabelMask) + 2 * sizeof(int))*item->GetLabelCount();
    count += sizeof(MTG_Node)*item->GetNodeIdCount();
    count += sizeof(int)*item->GetNodeIdCount()* item->GetLabelCount();
    return count;
    }

template<> size_t GetSizeInMemory<DifferenceSet>(DifferenceSet* item)
    {
    size_t count = sizeof(item) + item->addedFaces.size()*sizeof(DPoint3d) + item->addedVertices.size() * sizeof(int32_t) +
        item->removedFaces.size() * sizeof(int32_t) + item->removedVertices.size() * sizeof(int32_t) + item->addedUvIndices.size() * sizeof(int32_t) +
        item->addedUvs.size() * sizeof(DPoint2d);
    return count;
    }

template<> size_t GetSizeInMemory<BcDTMPtr>(BcDTMPtr* item)
    {
    size_t count = (item->get() == nullptr ? 0 : ((*item)->GetTinHandle() == nullptr ? 0 : sizeof(BC_DTM_OBJ) + (*item)->GetPointCount() *(sizeof(DPoint3d) + sizeof(DTM_TIN_NODE)+ sizeof(DTM_CIR_LIST)*6)));
    return count;
    }

bool TopologyIsDifferent(const int32_t* indicesA, const size_t nIndicesA, const int32_t* indicesB, const size_t nIndicesB)
    {
    MTGGraph graphA, graphB;
    bvector<int> temp;
    CreateGraphFromIndexBuffer(&graphA, (const long*)indicesA, (int)nIndicesA, (int)nIndicesA, temp, nullptr);
    CreateGraphFromIndexBuffer(&graphB, (const long*)indicesB, (int)nIndicesB, (int)nIndicesB, temp, nullptr);

    size_t nFacesA = CountExteriorFaces(&graphA);
    size_t nFacesB = CountExteriorFaces(&graphB);
    return nFacesA != nFacesB;
    }