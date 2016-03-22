#include <ImagePP\all\h\HRPPixelTypeV32R8G8B8A8.h>

#include <ImagePP/all/h/HFCException.h>
#include <ScalableMesh\ScalableMeshUtilityFunctions.h>
#include <ScalableMesh\IScalableMeshQuery.h>
#include <ImagePP/all/h/HCDCodecIdentity.h>
//#include <eigen\Eigen\Dense>
//#include <PCLWrapper\IDefines.h>
//#include <PCLWrapper\INormalCalculator.h>
#include "ScalableMeshQuery.h"
//#include "MeshingFunctions.h"
#include <ScalableMesh\ScalableMeshUtilityFunctions.h>
#include <Mtg/MtgStructs.h>
#include <Geom/bsp/bspbound.fdf>
#include "ScalableMesh\ScalableMeshGraph.h"
#include <string>
#include <queue>
#include <ctime>
#include <fstream>
#include "Edits/ClipUtilities.h"
#include "vuPolygonClassifier.h"
#include "LogUtils.h"
#include "Edits\Skirts.h"

USING_NAMESPACE_BENTLEY_SCALABLEMESH
#define SM_OUTPUT_MESHES_GRAPH 0
template <class POINT, class EXTENT> SMMeshIndexNode<POINT,EXTENT>::SMMeshIndexNode(size_t pi_SplitTreshold,
                 const EXTENT& pi_rExtent,
                 HFCPtr<HPMCountLimitedPool<POINT>> pool,
                 HFCPtr<SMPointTileStore<POINT, EXTENT> > store,
                 SMMeshIndex<POINT, EXTENT>* meshIndex,
                 ISMPointIndexFilter<POINT, EXTENT>* filter,
                 bool balanced,
                 bool textured,
                 bool propagateDataDown,
                 ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                 ISMPointIndexMesher<POINT, EXTENT>* mesher3d,
                 CreatedNodeMap*                      createdNodeMap)
                 : SMPointIndexNode<POINT, EXTENT>(pi_SplitTreshold, pi_rExtent, pool, store, filter, balanced, propagateDataDown, createdNodeMap),
                 m_graphVec(meshIndex->GetGraphPool(), meshIndex->GetGraphStore()),
                 m_ptsIndiceVec(1, *(new LinkedStoredPooledVector<int32_t>(meshIndex->GetPtsIndicesPool(), meshIndex->GetPtsIndicesStore()))),
                 m_uvVec(meshIndex->GetUVPool(), meshIndex->GetUVStore()),
                 m_uvsIndicesVec(1, *(new HPMStoredPooledVector<int32_t>(meshIndex->GetUVsIndicesPool(), meshIndex->GetUVsIndicesStore()))),
                 m_textureVec(1, *(new HPMStoredPooledVector<Byte>(meshIndex->GetTexturesPool(), meshIndex->GetTexturesStore())))
    {
    m_SMIndex = meshIndex;
    m_mesher2_5d = mesher2_5d;
    m_mesher3d = mesher3d;
    m_isGraphLoaded = false;
    m_graphVec.SetDirty(false);
    m_graphVec.SetDiscarded(true);
    m_isPtsIndiceLoaded = false;            
    m_isUVLoaded = false;
    m_isUVsIndicesLoaded = false;
    m_ptsIndiceVec[0].SetDirty(false);
    m_uvVec.SetDirty(false);
    m_uvsIndicesVec[0].SetDirty(false);
    m_textureVec[0].SetDirty(false);

    m_nodeHeader.m_graphID = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_ptsIndiceID.resize(1);
    m_nodeHeader.m_ptsIndiceID[0] = IDTMFile::GetNullNodeID();

    m_nodeHeader.m_uvsIndicesID.resize(1);
    m_nodeHeader.m_uvsIndicesID[0] = IDTMFile::GetNullNodeID();

    m_nodeHeader.m_uvID = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_textureID.resize(1);
    m_nodeHeader.m_textureID[0] = IDTMFile::GetNullNodeID();

    m_nbClips = 0;
    m_differenceSets.SetDirty(false);
//#ifdef SM_BESQL_FORMAT
    m_nodeHeader.m_ptsIndiceID[0] = GetBlockID();
    m_ptsIndiceVec[0].SetBlockID(GetBlockID());
    m_graphVec.SetBlockID(GetBlockID());
    m_differenceSets.SetBlockID(GetBlockID());
//#endif
    }

template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::SMMeshIndexNode(size_t pi_SplitTreshold,
                const EXTENT& pi_rExtent,
                const HFCPtr<SMMeshIndexNode<POINT, EXTENT> >& pi_rpParentNode)
                : SMPointIndexNode<POINT, EXTENT>(pi_SplitTreshold, pi_rExtent, dynamic_cast<SMPointIndexNode*>(pi_rpParentNode.GetPtr())),
                m_graphVec(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetGraphPool(), dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetGraphStore()),
                m_ptsIndiceVec(1, *(new LinkedStoredPooledVector<int32_t>(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetPtsIndicesPool(), dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetPtsIndicesStore()))),
                m_uvVec(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetUVPool(), dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetUVStore()),
                m_uvsIndicesVec(1, *(new HPMStoredPooledVector<int32_t>(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetUVsIndicesPool(), dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetUVsIndicesStore()))),
                m_textureVec(1, *(new HPMStoredPooledVector<Byte>(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetTexturesPool(), dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetTexturesStore())))
    {
    m_SMIndex = pi_rpParentNode->m_SMIndex;
    m_mesher2_5d = pi_rpParentNode->GetMesher2_5d();
    m_mesher3d = pi_rpParentNode->GetMesher3d();
    m_isGraphLoaded = false;    
    m_graphVec.SetDirty(false);
    m_graphVec.SetDiscarded(true);
    m_isPtsIndiceLoaded = false;
    m_isUVLoaded = false;
    m_isUVsIndicesLoaded = false;    
    m_ptsIndiceVec[0].SetDirty(false);
    m_uvVec.SetDirty(false);
    m_uvsIndicesVec[0].SetDirty(false);
    m_textureVec[0].SetDirty(false);

    m_nbClips = 0;
    m_differenceSets.SetDirty(false);
    m_differenceSets.SetStore(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetClipStore());
    m_differenceSets.SetPool(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetClipPool());
//    m_indiceVec.SetDiscarded(true);
    m_nodeHeader.m_graphID = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_ptsIndiceID.resize(1);
    m_nodeHeader.m_ptsIndiceID[0] = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_uvsIndicesID.resize(1);
    m_nodeHeader.m_uvsIndicesID[0] = IDTMFile::GetNullNodeID();
//    m_nodeHeader.m_uvID.resize(1);
    m_nodeHeader.m_uvID = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_textureID.resize(1);
    m_nodeHeader.m_textureID[0] = IDTMFile::GetNullNodeID();
//#ifdef SM_BESQL_FORMAT
    m_nodeHeader.m_ptsIndiceID[0] = GetBlockID();
    m_ptsIndiceVec[0].SetBlockID(GetBlockID());
    m_graphVec.SetBlockID(GetBlockID());
    m_differenceSets.SetBlockID(GetBlockID());
//#endif
    }

template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::SMMeshIndexNode(size_t pi_SplitTreshold,
                                                                                     const EXTENT& pi_rExtent,
                                                                                     const HFCPtr<SMMeshIndexNode<POINT, EXTENT> >& pi_rpParentNode,
                                                                                     bool IsUnsplitSubLevel)
                                                                                     : SMPointIndexNode<POINT, EXTENT>(pi_SplitTreshold, pi_rExtent, dynamic_cast<SMPointIndexNode*>(pi_rpParentNode.GetPtr()), IsUnsplitSubLevel),
                                                                                     m_graphVec(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetGraphPool(), dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetGraphStore()),
                                                                                     m_ptsIndiceVec(1, *(new LinkedStoredPooledVector<int32_t>(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetPtsIndicesPool(), dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetPtsIndicesStore()))),
                  m_uvVec(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetUVPool(), dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetUVStore()),
                  m_uvsIndicesVec(1, *(new HPMStoredPooledVector<int32_t>(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetUVsIndicesPool(), dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetUVsIndicesStore()))),
                 m_textureVec(1, *(new HPMStoredPooledVector<Byte>(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetTexturesPool(), dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetTexturesStore())))
    {
    m_SMIndex = pi_rpParentNode->m_SMIndex;
    m_mesher2_5d = pi_rpParentNode->GetMesher2_5d();
    m_mesher3d = pi_rpParentNode->GetMesher3d();
    m_isGraphLoaded = false;
    m_graphVec.SetDirty(false);
    m_graphVec.SetDiscarded(true);
    m_isPtsIndiceLoaded = false;
    m_isUVLoaded = false;
    m_isUVsIndicesLoaded = false;

    m_ptsIndiceVec[0].SetDirty(false);
    m_uvVec.SetDirty(false);
    m_uvsIndicesVec[0].SetDirty(false);
    m_textureVec[0].SetDirty(false);

    m_nbClips = 0;
    m_differenceSets.SetDirty(false);
    m_differenceSets.SetStore(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetClipStore());
    m_differenceSets.SetPool(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetClipPool());
//    m_indiceVec.SetDiscarded(true);
    m_nodeHeader.m_graphID = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_ptsIndiceID.resize(1);
    m_nodeHeader.m_ptsIndiceID[0] = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_uvsIndicesID.resize(1);
    m_nodeHeader.m_uvsIndicesID[0] = IDTMFile::GetNullNodeID();
//    m_nodeHeader.m_uvID.resize(1);
    m_nodeHeader.m_uvID = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_textureID.resize(1);
    m_nodeHeader.m_textureID[0] = IDTMFile::GetNullNodeID();
//#ifdef SM_BESQL_FORMAT
    m_nodeHeader.m_ptsIndiceID[0] = GetBlockID();
    m_ptsIndiceVec[0].SetBlockID(GetBlockID());
    m_graphVec.SetBlockID(GetBlockID());
    m_differenceSets.SetBlockID(GetBlockID());
//#endif
    }

/* SM_NEEDS_WORK : Did we use it ?
template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::SMMeshIndexNode(const SMMeshIndexNode<POINT, EXTENT>& pi_rNode)
    :SMPointIndexNode<POINT, EXTENT>(pi_rNode), 
    m_graphVec(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rNode.m_SMIndex).GetGraphPool(), dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rNode.m_SMIndex).GetGraphStore()),
    m_ptsIndiceVec(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rNode.m_SMIndex).GetPtsIndicesPool(), dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rNode.m_SMIndex).GetPtsIndicesStore()),
    m_uvVec(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rNode.m_SMIndex).GetUVPool(), dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rNode.m_SMIndex).GetUVStore()),
    m_uvIndiceVec(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rNode.m_SMIndex).GetUVsIndicesPool(), dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rNode.m_SMIndex).GetUVsIndicesStore()),
    m_textureVec(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rNode.m_SMIndex).GetTexturesPool(), dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rNode.m_SMIndex).GetTexturesStore())
    {
    m_SMIndex = pi_rNode.m_SMIndex;
    m_mesher2_5d = pi_rNode.GetMesher2_5d();
    m_mesher3d = pi_rNode.GetMesher3d();
    m_isGraphLoaded = false;
    m_graphVec.SetDirty(false);
    m_graphVec.SetDiscarded(true);
    m_isPtsIndiceLoaded = false;
    m_isUVLoaded = false;
    m_isUVsIndicesLoaded = false;
    m_ptsIndiceVec[0].SetDirty(false);
    m_uvVec.SetDirty(false);
    m_uvsIndicesVec[0].SetDirty(false);
    m_textureVec[0].SetDirty(false);

    m_nodeHeader.m_graphID = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_ptsIndiceID.resize(1);
    m_nodeHeader.m_ptsIndiceID[0] = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_uvsIndicesID.resize(1);
    m_nodeHeader.m_uvsIndicesID[0] = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_uvID = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_textureID.resize(1);
    m_nodeHeader.m_textureID[0] = IDTMFile::GetNullNodeID();
#ifdef SM_BESQL_FORMAT
    m_nodeHeader.m_ptsIndiceID[0] = GetBlockID();
    m_ptsIndiceVec[0].SetBlockID(GetBlockID());
    m_graphVec.SetBlockID(GetBlockID());
#endif
    }*/
/* SM_NEEDS_WORK : Did we use it ?
template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::SMMeshIndexNode(const SMPointIndexNode<POINT, EXTENT>& pi_rNode)
    : SMPointIndexNode<POINT, EXTENT>(pi_rNode)
    {
    m_mesher2_5d = pi_rNode.GetMesher2_5d();
    m_mesher3d = pi_rNode.GetMesher3d();
    m_isGraphLoaded = false;
    m_graphVec.SetDirty(false);
    m_graphVec.SetDiscarded(true);
    m_isPtsIndiceLoaded = false;
    m_isUVLoaded = false;
    m_isUVsIndicesLoaded = false;
    m_ptsIndiceVec[0].SetDirty(false);
    m_uvVec.SetDirty(false);
    m_uvsIndicesVec[0].SetDirty(false);
    m_textureVec[0].SetDirty(false);

    m_nodeHeader.m_graphID = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_ptsIndiceID.resize(1);
    m_nodeHeader.m_ptsIndiceID[0] = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_uvsIndicesID.resize(1);
    m_nodeHeader.m_uvsIndicesID[0] = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_uvID = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_textureID.resize(1);
    m_nodeHeader.m_textureID[0] = IDTMFile::GetNullNodeID();
#ifdef SM_BESQL_FORMAT
    m_nodeHeader.m_ptsIndiceID[0] = GetBlockID();
    m_ptsIndiceVec[0].SetBlockID(GetBlockID());
    m_graphVec.SetBlockID(GetBlockID());
#endif
    }*/


/* SM_NEEDS_WORK : did we use it ?
template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::SMMeshIndexNode(const SMMeshIndexNode<POINT, EXTENT>& pi_rNode,
                                                                                       const HFCPtr<SMMeshIndexNode>& pi_rpParentNode)
                                                                                       : SMPointIndexNode<POINT, EXTENT>(pi_rNode, pi_rpParentNode),
    m_graphVec(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetGraphPool(), dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetGraphStore()),
    m_ptsIndiceVec(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetPtsIndicesPool(), dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetPtsIndicesStore()),
    m_uvVec(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetUVPool(), dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetUVStore()),
    m_uvIndicesVec(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetUVsIndicesPool(), dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetUVsIndicesStore()),
    m_texturesVec(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetTexturesPool(), dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(pi_rpParentNode->m_SMIndex)->GetTexturesStore())
    {
    m_SMIndex = pi_rpParentNode->m_SMIndex;
    m_mesher2_5d = pi_rpParentNode->GetMesher2_5d();
    m_mesher3d = pi_rpParentNode->GetMesher3d();
    m_isGraphLoaded = false;
    m_graphVec.SetDirty(false);
    m_graphVec.SetDiscarded(true);
    m_isPtsIndiceLoaded = false;
    m_isUVLoaded = false;
    m_isUVsIndicesLoaded = false;
    m_ptsIndiceVec[0].SetDirty(false);
    m_uvVec.SetDirty(false);
    m_uvsIndicesVec[0].SetDirty(false);
    m_textureVec[0].SetDirty(false);
    m_differenceSets.SetDirty(false);
    m_differenceSets.SetStore(m_clipStore);
    m_differenceSets.SetPool(m_clipPool);

    m_nodeHeader.m_graphID = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_ptsIndiceID.resize(1);
    m_nodeHeader.m_ptsIndiceID[0] = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_uvsIndicesID.resize(1);
    m_nodeHeader.m_uvsIndicesID[0] = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_uvID = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_textureID.resize(1);
    m_nodeHeader.m_textureID[0] = IDTMFile::GetNullNodeID();

#ifdef SM_BESQL_FORMAT
    m_nodeHeader.m_ptsIndiceID[0] = GetBlockID();
    m_ptsIndiceVec[0].SetBlockID(GetBlockID());
    m_graphVec.SetBlockID(GetBlockID());
#endif
    }
    */
template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::SMMeshIndexNode(HPMBlockID blockID,
                                                                                       HFCPtr<SMMeshIndexNode<POINT, EXTENT> > parent,
                 HFCPtr<HPMCountLimitedPool<POINT> > pool,
                 HFCPtr<SMPointTileStore<POINT, EXTENT> > store,
                 SMMeshIndex<POINT, EXTENT>* meshIndex,
                 ISMPointIndexFilter<POINT, EXTENT>* filter,
                 bool balanced,
                 bool textured,
                 bool propagateDataDown,
                 ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                 ISMPointIndexMesher<POINT, EXTENT>* mesher3d,
                 CreatedNodeMap*                      createdNodeMap)
                 : SMPointIndexNode<POINT, EXTENT>(blockID, static_pcast<SMPointIndexNode<POINT, EXTENT>, SMMeshIndexNode<POINT, EXTENT>>(parent), pool, store, filter, balanced, propagateDataDown, createdNodeMap),
                  m_graphVec(meshIndex->GetGraphPool(), meshIndex->GetGraphStore()),
                  m_ptsIndiceVec(1, *(new LinkedStoredPooledVector<int32_t>(meshIndex->GetPtsIndicesPool(), meshIndex->GetPtsIndicesStore()))),
                  m_uvVec(meshIndex->GetUVPool(), meshIndex->GetUVStore()),
                  m_uvsIndicesVec(1, *(new HPMStoredPooledVector<int32_t>(meshIndex->GetUVsIndicesPool(), meshIndex->GetUVsIndicesStore()))),
                  m_textureVec(1, *(new HPMStoredPooledVector<Byte>(meshIndex->GetTexturesPool(), meshIndex->GetTexturesStore())))
                 
    {
    m_SMIndex = meshIndex;
    m_mesher2_5d = mesher2_5d;
    m_mesher3d = mesher3d;
    m_isGraphLoaded = false;
    m_graphVec.SetDirty(false);
    m_graphVec.SetDiscarded(true);
    m_isPtsIndiceLoaded = false;
    m_isUVLoaded = false;

    m_ptsIndiceVec[0].SetDirty(false);
    m_uvVec.SetDirty(false);
    m_isUVsIndicesLoaded = false;
    m_uvsIndicesVec[0].SetDirty(false);
    m_textureVec[0].SetDirty(false);

    m_nbClips = 0;
    m_differenceSets.SetDirty(false);
    m_differenceSets.SetStore(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetClipStore());
    m_differenceSets.SetPool(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetClipPool());
//    m_indiceVec.SetDiscarded(true);
    m_nodeHeader.m_graphID = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_ptsIndiceID.resize(1);
    m_nodeHeader.m_ptsIndiceID[0] = IDTMFile::GetNullNodeID();

    m_nodeHeader.m_uvsIndicesID.resize(1);
    m_nodeHeader.m_uvsIndicesID[0] = IDTMFile::GetNullNodeID();
//    m_nodeHeader.m_uvID.resize(1);
    m_nodeHeader.m_uvID = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_textureID.resize(1);
    m_nodeHeader.m_textureID[0]= IDTMFile::GetNullNodeID();
//#ifdef SM_BESQL_FORMAT
    m_nodeHeader.m_ptsIndiceID[0] = GetBlockID();
    m_ptsIndiceVec[0].SetBlockID(GetBlockID());
    m_graphVec.SetBlockID(GetBlockID());
    m_differenceSets.SetBlockID(GetBlockID());

//#endif
    }

template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::SMMeshIndexNode(HPMBlockID blockID,
                 HFCPtr<HPMCountLimitedPool<POINT> > pool,
                 HFCPtr<SMPointTileStore<POINT, EXTENT> > store,
                 SMMeshIndex<POINT, EXTENT>* meshIndex,
                 ISMPointIndexFilter<POINT, EXTENT>* filter,
                 bool balanced,
                 bool textured,
                 bool propagateDataDown,
                 ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                 ISMPointIndexMesher<POINT, EXTENT>* mesher3d,
                 CreatedNodeMap* createdNodeMap)
                 : SMPointIndexNode<POINT, EXTENT>(blockID, pool, store, filter, balanced, propagateDataDown, createdNodeMap),
                 m_graphVec(meshIndex->GetGraphPool(), meshIndex->GetGraphStore()),
                 m_ptsIndiceVec(1, *(new LinkedStoredPooledVector<int32_t>(meshIndex->GetPtsIndicesPool(), meshIndex->GetPtsIndicesStore()))),
                 m_uvVec(meshIndex->GetUVPool(), meshIndex->GetUVStore()),
                 m_uvsIndicesVec(1, *(new HPMStoredPooledVector<int32_t>(meshIndex->GetUVsIndicesPool(), meshIndex->GetUVsIndicesStore()))),
                 m_textureVec(1, *(new HPMStoredPooledVector<Byte>(meshIndex->GetTexturesPool(), meshIndex->GetTexturesStore())))

    {
    m_SMIndex = meshIndex;
    m_mesher2_5d = mesher2_5d;
    m_mesher3d = mesher3d;
    m_isGraphLoaded = false;
    m_graphVec.SetDirty(false);
    m_graphVec.SetDiscarded(true);
    m_isPtsIndiceLoaded = false;
    m_isUVLoaded = false;
    m_isUVsIndicesLoaded = false;
    m_ptsIndiceVec[0].SetDirty(false);
    m_textureVec[0].SetDirty(false);
    m_uvsIndicesVec[0].SetDirty(false);
    m_uvVec.SetDirty(false);

    m_nbClips = 0;
    m_differenceSets.SetDirty(false);
    m_differenceSets.SetStore(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetClipStore());
    m_differenceSets.SetPool(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetClipPool());
    //m_indiceVec.SetDiscarded(true);
    m_nodeHeader.m_graphID = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_ptsIndiceID.resize(1);
    m_nodeHeader.m_ptsIndiceID[0] = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_uvsIndicesID.resize(1);
    m_nodeHeader.m_uvsIndicesID[0] = IDTMFile::GetNullNodeID();
//    m_nodeHeader.m_uvID.resize(1);
    m_nodeHeader.m_uvID = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_textureID.resize(1);
    m_nodeHeader.m_textureID[0] = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_ptsIndiceID[0] = GetBlockID();
    m_ptsIndiceVec[0].SetBlockID(GetBlockID());
    m_graphVec.SetBlockID(GetBlockID());
    m_differenceSets.SetBlockID(GetBlockID());
    }

template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::SMMeshIndexNode(size_t pi_SplitTreshold,
                 const EXTENT& pi_rExtent,
                 HFCPtr<HPMCountLimitedPool<POINT>> pool,
                 HFCPtr<SMPointTileStore<POINT, EXTENT> > store,
                 SMMeshIndex<POINT, EXTENT>* meshIndex,
                 ISMPointIndexFilter<POINT, EXTENT>* filter,
                 bool balanced,
                 bool propagateDataDown,
                 ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                 ISMPointIndexMesher<POINT, EXTENT>* mesher3d,
                 CreatedNodeMap*                      createdNodeMap)
                 : SMPointIndexNode<POINT, EXTENT>(pi_SplitTreshold, pi_rExtent, pool, store, filter, balanced, propagateDataDown, createdNodeMap)
    {
    m_SMIndex = meshIndex;
    m_mesher2_5d = mesher2_5d;
    m_mesher3d = mesher3d;
    m_isGraphLoaded = false;
    m_graphVec.SetDirty(false);
    m_graphVec.SetDiscarded(true);
    m_isUVLoaded = false;
    m_isUVsIndicesLoaded = false;
    m_ptsIndiceVec[0].SetDirty(false);
    m_uvVec.SetDirty(false);
    m_uvsIndicesVec[0].SetDirty(false);
    m_textureVec[0].SetDirty(false);
    m_nodeHeader.m_graphID = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_ptsIndiceID.resize(1);
    m_nodeHeader.m_ptsIndiceID[0] = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_uvID = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_uvsIndicesID.resize(1);
    m_nodeHeader.m_uvsIndicesID[0] = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_textureID.resize(1);
    m_nodeHeader.m_textureID[0] = IDTMFile::GetNullNodeID();

    m_nbClips = 0;
    m_differenceSets.SetDirty(false);
//#ifdef SM_BESQL_FORMAT
    m_nodeHeader.m_ptsIndiceID[0] = GetBlockID();
    m_ptsIndiceVec[0].SetBlockID(GetBlockID());
    m_graphVec.SetBlockID(GetBlockID());
    m_differenceSets.SetBlockID(GetBlockID());
//#endif
    }

template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::SMMeshIndexNode(HPMBlockID blockID,
                 HFCPtr<HPMCountLimitedPool<POINT> > pool,
                 HFCPtr<SMPointTileStore<POINT, EXTENT> > store,
                 SMMeshIndex<POINT, EXTENT>* meshIndex,
                 ISMPointIndexFilter<POINT, EXTENT>* filter,
                 bool balanced,
                 bool propagateDataDown,
                 ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                 ISMPointIndexMesher<POINT, EXTENT>* mesher3d,
                 CreatedNodeMap* createdNodeMap) 
                 : SMPointIndexNode<POINT, EXTENT>(blockID, pool, store, filter, balanced, propagateDataDown, createdNodeMap)
    {
    m_SMIndex = meshIndex;
    m_mesher2_5d = mesher2_5d;
    m_mesher3d = mesher3d;
    m_isGraphLoaded = false;
    m_graphVec.SetDirty(false);
    m_graphVec.SetDiscarded(true);
    m_isPtsIndiceLoaded = false;
    m_isUVLoaded = false;
    m_isUVsIndicesLoaded = false;
    m_ptsIndiceVec[0].SetDirty(false);
    m_uvVec.SetDirty(false);
    m_uvsIndicesVec[0].SetDirty(false);
    m_textureVec[0].SetDirty(false);
//    m_indiceVec.SetDiscarded(true);
    m_nodeHeader.m_graphID = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_ptsIndiceID.resize(1);
    m_nodeHeader.m_ptsIndiceID[0] = IDTMFile::GetNullNodeID();

    m_nodeHeader.m_uvsIndicesID.resize(1);
    m_nodeHeader.m_uvsIndicesID[0] = IDTMFile::GetNullNodeID();
//    m_nodeHeader.m_uvID.resize(1);
    m_nodeHeader.m_uvID/*[0]*/ = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_textureID.resize(1);
    m_nodeHeader.m_textureID[0] = IDTMFile::GetNullNodeID();

    m_nbClips = 0;
    m_differenceSets.SetDirty(false);
//#ifdef SM_BESQL_FORMAT
    m_nodeHeader.m_ptsIndiceID[0] = GetBlockID();
    m_ptsIndiceVec[0].SetBlockID(GetBlockID());
    m_graphVec.SetBlockID(GetBlockID());
    m_differenceSets.SetBlockID(GetBlockID());
//#endif
    }

template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::SMMeshIndexNode(HPMBlockID blockID,
                 HFCPtr<SMMeshIndexNode<POINT, EXTENT> > parent,
                 HFCPtr<HPMCountLimitedPool<POINT> > pool,
                 HFCPtr<SMPointTileStore<POINT, EXTENT> > store,
                 SMMeshIndex<POINT, EXTENT>* meshIndex,
                 ISMPointIndexFilter<POINT, EXTENT>* filter,
                 bool balanced,
                 bool propagateDataDown,
                 ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                 ISMPointIndexMesher<POINT, EXTENT>* mesher3d,
                 CreatedNodeMap*                      createdNodeMap)
                 : SMPointIndexNode<POINT, EXTENT>(blockID, static_pcast<SMPointIndexNode<POINT, EXTENT>, SMMeshIndexNode<POINT, EXTENT>>(parent), pool, store, filter, balanced, propagateDataDown, createdNodeMap)
    {
    m_SMIndex = meshIndex;
    m_mesher2_5d = mesher2_5d;
    m_mesher3d = mesher3d;
    m_isGraphLoaded = false;
    m_graphVec.SetDirty(false);
    m_graphVec.SetDiscarded(true);
    m_isPtsIndiceLoaded = false;
    m_isUVLoaded = false;
    m_isUVsIndicesLoaded = false;
    m_ptsIndiceVec[0].SetDirty(false);
    m_uvVec.SetDirty(false);
    m_uvsIndicesVec[0].SetDirty(false);
    m_textureVec[0].SetDirty(false);

    m_nbClips = 0;
    m_differenceSets.SetDirty(false);
    m_differenceSets.SetStore(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetClipStore());
    m_differenceSets.SetPool(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetClipPool());
    m_nodeHeader.m_graphID = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_ptsIndiceID.resize(1);
   m_nodeHeader.m_ptsIndiceID[0] = IDTMFile::GetNullNodeID();

    m_nodeHeader.m_uvsIndicesID.resize(1);
    m_nodeHeader.m_uvsIndicesID[0] = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_uvID = IDTMFile::GetNullNodeID();
    m_nodeHeader.m_textureID.resize(1);
    m_nodeHeader.m_textureID[0] = IDTMFile::GetNullNodeID();
//#ifdef SM_BESQL_FORMAT
    m_nodeHeader.m_ptsIndiceID[0] = GetBlockID();
    m_ptsIndiceVec[0].SetBlockID(GetBlockID());
    m_graphVec.SetBlockID(GetBlockID());
    m_differenceSets.SetBlockID(GetBlockID());
//#endif
    }

template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::~SMMeshIndexNode()
    {
    if (!IsDestroyed())
        {
        // Unload self ... this should result in discard 
        Unload();

        if (!Discarded() && IsDirty())
            {
            // This is a bit too late to discard as we are during destruction ... virtual overload of Discard is not accessible if inherited
            // Usually inheritance will not exist at this time ...
            Discard();
            }
        else 
            {
            if (m_graphVec.IsDirty())
                {
                if (IsGraphLoaded()) StoreGraph();
                else m_graphVec.SetDirty(false);
                }

            for (int i = 0; i < m_ptsIndiceVec.size(); i++)
            {
                if (m_ptsIndiceVec[i].IsDirty())
                {
                    if (IsPtsIndiceLoaded()) StorePtsIndice(i);
                    else m_ptsIndiceVec[i].SetDirty(false);
                }
            }

            for (int i = 0; i < m_uvsIndicesVec.size(); i++)
            {
                if (m_uvsIndicesVec[i].IsDirty())
                {
                    if (IsUVsIndicesLoaded()) StoreUVsIndices(i);
                    else m_uvsIndicesVec[i].SetDirty(false);
                }
            }

            for (int i = 0; i < m_textureVec.size(); i++)
            {
                if (m_textureVec[i].IsDirty())
                {
                    if (IsTextureLoaded()) StoreTexture(i);
                    else m_textureVec[i].SetDirty(false);
                }
            }
            
            if(m_uvVec.IsDirty())
                {
                if(IsUVLoaded()) StoreUV();
                else m_uvVec.SetDirty(false);
                }
            }
        }
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 07/15
//=======================================================================================
template<class POINT, class EXTENT> void  SMMeshIndexNode<POINT, EXTENT>::StoreAllGraphs()
    {
    if (!IsLoaded())
        return;
    StoreGraph();
    if (!IsLeaf())
        {
        if (static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit) != NULL)
            static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->StoreAllGraphs();
        else
            {
            for (size_t indexNode = 0; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                {
                if (m_apSubNodes[indexNode] != NULL)
                    static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->StoreAllGraphs();
                }
            }
        }

    }

//=======================================================================================
// @bsimethod                                                  Elenie.Godzaridis 03/15
//=======================================================================================
template <class POINT, class EXTENT> bool SMMeshIndexNode<POINT, EXTENT>::IsGraphLoaded() const
    {
    return m_isGraphLoaded;
    }

template<class POINT, class EXTENT> bool SMMeshIndexNode<POINT, EXTENT>::Destroy()
    {
    SMPointIndexNode::Destroy();
    //m_graphVec.clear();
    m_graphVec.SetDirty(false);
    m_graphVec.SetDiscarded(true);
    m_isGraphLoaded = false;
    if (m_graphVec.GetBlockID().IsValid())
        dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetGraphStore()->DestroyBlock(m_graphVec.GetBlockID());

    for (int i = 0; i < m_ptsIndiceVec.size(); i++)
    {
        m_ptsIndiceVec[i].SetDirty(false);

        m_isPtsIndiceLoaded = false;
        if (m_ptsIndiceVec[i].GetBlockID().IsValid())
            dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetPtsIndicesStore()->DestroyBlock(m_ptsIndiceVec[i].GetBlockID());
        
        m_uvsIndicesVec[i].SetDirty(false);

        m_isUVsIndicesLoaded = false;
        if (m_uvsIndicesVec[i].GetBlockID().IsValid())
            dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetUVsIndicesStore()->DestroyBlock(m_uvsIndicesVec[i].GetBlockID());

        m_textureVec[i].SetDirty(false);

        m_isTextureLoaded = false;
        if (m_textureVec[i].GetBlockID().IsValid())
            dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetTexturesStore()->DestroyBlock(m_textureVec[i].GetBlockID());
    }

    m_uvVec.SetDirty(false);

    m_isUVLoaded = false;
    if (m_uvVec.GetBlockID().IsValid())
        dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetUVStore()->DestroyBlock(m_uvVec.GetBlockID());

    HINVARIANTS;

    return true;
    }

template<class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndexNode<POINT, EXTENT>::Clone() const
    {
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = static_cast<SMPointIndexNode<POINT, EXTENT> *>(new SMMeshIndexNode<POINT, EXTENT>(GetSplitTreshold(), GetNodeExtent(), GetPool(), dynamic_cast<SMPointTileStore<POINT, EXTENT>* >(&*(GetStore())), dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex), GetFilter(), IsBalanced(), IsTextured(), PropagatesDataDown(), GetMesher2_5d(), GetMesher3d(), m_createdNodeMap));
    return pNewNode;
    }
template<class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndexNode<POINT, EXTENT>::Clone(const EXTENT& newNodeExtent) const
    {
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = static_cast<SMPointIndexNode<POINT, EXTENT> *>(new SMMeshIndexNode<POINT, EXTENT>(GetSplitTreshold(), newNodeExtent, GetPool(), dynamic_cast<SMPointTileStore<POINT, EXTENT>* >(&*(GetStore())), dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex), GetFilter(), IsBalanced(), IsTextured(), PropagatesDataDown(), GetMesher2_5d(), GetMesher3d(), m_createdNodeMap));
    return pNewNode;
    }
template<class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndexNode<POINT, EXTENT>::CloneChild(const EXTENT& newNodeExtent) const
    {
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = static_cast<SMPointIndexNode<POINT, EXTENT> *>(new SMMeshIndexNode<POINT, EXTENT>(GetSplitTreshold(), newNodeExtent, const_cast<SMMeshIndexNode<POINT, EXTENT>*>(this)));
    return pNewNode;
    }
template<class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndexNode<POINT, EXTENT>::CloneUnsplitChild(const EXTENT& newNodeExtent) const
    {
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = static_cast<SMPointIndexNode<POINT, EXTENT> *>(new SMMeshIndexNode<POINT, EXTENT>(GetSplitTreshold(), newNodeExtent, const_cast<SMMeshIndexNode<POINT, EXTENT>*>(this), true));
    return pNewNode;
    }

template<class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndexNode<POINT, EXTENT>::CloneUnsplitChildVirtual() const
    {
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = static_cast<SMPointIndexNode<POINT, EXTENT> *>(new SMIndexNodeVirtual<POINT,EXTENT,SMMeshIndexNode<POINT,EXTENT>>(const_cast<SMMeshIndexNode<POINT, EXTENT>*>(this)));
    return pNewNode;
    }

template<class POINT, class EXTENT>  HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndexNode<POINT, EXTENT>::CreateNewChildNode(HPMBlockID blockID)
    {
    auto node = new SMMeshIndexNode<POINT, EXTENT>(blockID, this, m_pool, static_cast<SMPointTileStore<POINT, EXTENT>*>(&*(m_store)), dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex), m_filter, m_needsBalancing, false, !(m_delayedDataPropagation), m_mesher2_5d, m_mesher3d, m_createdNodeMap);
    node->m_clipRegistry = m_clipRegistry;
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = static_cast<SMPointIndexNode<POINT, EXTENT> *>(node);
    return pNewNode;
    }

template<class POINT, class EXTENT>  HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndexNode<POINT, EXTENT>::CreateNewNode(HPMBlockID blockID, bool isRootNode)
    {
    auto node = new SMMeshIndexNode<POINT, EXTENT>(blockID, m_pool, static_cast<SMPointTileStore<POINT, EXTENT>*>(&*(m_store)), dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex), m_filter, m_needsBalancing, false, !(m_delayedDataPropagation), m_mesher2_5d, m_mesher3d, m_createdNodeMap);

    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = static_cast<SMPointIndexNode<POINT, EXTENT> *>(node);

    if (isRootNode)
        {
        HFCPtr<SMPointIndexNode<POINT, EXTENT>> parentNodePtr;
        pNewNode->SetParentNodePtr(parentNodePtr);
        }
    
    return pNewNode;
    }

//=======================================================================================
// @bsimethod                                                  Elenie.Godzaridis 03/15
//=======================================================================================
template<class POINT, class EXTENT> bool SMMeshIndexNode<POINT, EXTENT>::Discard() const
    {
    HINVARIANTS;
    bool returnValue = true;
    const_cast<SMMeshIndexNode<POINT, EXTENT>*>(this)->m_tileBcDTM = nullptr;
    if (!m_destroyed && !Discarded())
        {
        if (!m_graphVec.Discarded()) StoreGraph();
        else if (m_graphVec.GetBlockID().IsValid())  m_nodeHeader.m_graphID = m_graphVec.GetBlockID();
        
        if(m_differenceSets.IsDirty() && !m_differenceSets.Discarded()) m_differenceSets.Discard();
        if (m_differenceSets.GetBlockID().IsValid()) m_nodeHeader.m_clipSetsID.push_back(m_differenceSets.GetBlockID());
        
        std::string s;
        s += " N OF INDICE ARRAYS " + std::to_string(GetNbPtsIndiceArrays());
        for (size_t i = 0; i <GetNbPtsIndiceArrays(); ++i)
            {
            s += " ARRAY " + std::to_string(i) + " HAS " + std::to_string(GetNbPtsIndices(i)) + " INDICES ";
            }
        for (size_t i = 0; i < m_ptsIndiceVec.size(); ++i)
            {
            if (!m_ptsIndiceVec[i].Discarded() && m_ptsIndiceVec[i].IsDirty()) StorePtsIndice(i);
            else if (m_ptsIndiceVec[i].GetBlockID().IsValid())  m_nodeHeader.m_ptsIndiceID[i] = m_ptsIndiceVec[i].GetBlockID();
            }
        if(!m_SMIndex->m_useSTMFormat)
        if (m_nodeHeader.m_ptsIndiceID.size() > 0 && m_nodeHeader.m_ptsIndiceID[0].IsValid() && m_nodeHeader.m_ptsIndiceID[0] != IDTMFile::GetNullNodeID()) SetBlockID(m_nodeHeader.m_ptsIndiceID[0]);
        for (size_t i = 0; i < m_nodeHeader.m_ptsIndiceID.size(); ++i)
            {
            s += "INDICE ID " + std::to_string(i) + " IS " + std::to_string(m_nodeHeader.m_ptsIndiceID[i].m_integerID);
            }

        if (!m_uvVec.Discarded() && m_uvVec.IsDirty()) StoreUV();
        else if (m_uvVec.GetBlockID().IsValid())  m_nodeHeader.m_uvID = m_uvVec.GetBlockID();

            for (size_t i = 0; i < m_uvsIndicesVec.size(); ++i)
            {
                if (!m_uvsIndicesVec[i].Discarded() && m_uvsIndicesVec[i].IsDirty()) StoreUVsIndices(i);
                else if (m_uvsIndicesVec[i].GetBlockID().IsValid())  m_nodeHeader.m_uvsIndicesID[i] = m_uvsIndicesVec[i].GetBlockID();
            }

        for(size_t i = 0; i < m_textureVec.size(); ++i)
        {
            if (!m_textureVec[i].Discarded() && m_textureVec[i].IsDirty()) StoreTexture(i);
            else if (m_textureVec[i].GetBlockID().IsValid())  m_nodeHeader.m_textureID[i] = m_textureVec[i].GetBlockID();
        }

        if (IsLoaded()) returnValue = SMPointIndexNode<POINT, EXTENT>::Discard();
            else returnValue = HPMStoredPooledVector<POINT>::Discard();
        //returnValue = SMPointIndexNode<POINT, EXTENT>::Discard();
        }
    HINVARIANTS;

    return returnValue;

    }

//=======================================================================================
// @bsimethod                                                  Elenie.Godzaridis 03/15
//=======================================================================================
template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::Load() const
    {
    if (IsLoaded()) return;
    SMPointIndexNode<POINT, EXTENT>::Load();
    m_graphVec.SetBlockID(m_nodeHeader.m_graphID);
    if (m_nodeHeader.m_clipSetsID.size() > 0) m_differenceSets.SetBlockID(m_nodeHeader.m_clipSetsID[0]);


#if DEBUG && SM_TRACE_RASTER_TEXTURING
    std::string s;
    for (size_t i = 0; i < m_nodeHeader.m_ptsIndiceID.size(); ++i)
        {
        s += "INDICE ID " + std::to_string(i) + " IS " + std::to_string(m_nodeHeader.m_ptsIndiceID[i].m_integerID);
        }
#endif
    m_ptsIndiceVec.resize(m_nodeHeader.m_ptsIndiceID.size());
    for (size_t i = 0; i < m_nodeHeader.m_ptsIndiceID.size(); ++i)
        {
        m_ptsIndiceVec[i].SetStore(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetPtsIndicesStore());
        if (m_ptsIndiceVec[i].GetPool() == NULL) m_ptsIndiceVec[i].SetPool(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetPtsIndicesPool());
        m_ptsIndiceVec[i].SetBlockID(m_nodeHeader.m_ptsIndiceID[i]);
        if (m_ptsIndiceVec[i].GetBlockID().IsValid())
            m_ptsIndiceVec[i].SetDiscarded(true);
        m_ptsIndiceVec[i].SetDirty(false);
        }
    m_uvsIndicesVec.resize(m_nodeHeader.m_uvsIndicesID.size());
    for (size_t i = 0; i < m_nodeHeader.m_uvsIndicesID.size(); ++i)
        {
        m_uvsIndicesVec[i].SetStore(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetUVsIndicesStore());
        if (m_uvsIndicesVec[i].GetPool() == NULL) m_uvsIndicesVec[i].SetPool(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetUVsIndicesPool());
        m_uvsIndicesVec[i].SetBlockID(m_nodeHeader.m_uvsIndicesID[i]);
        if (m_uvsIndicesVec[i].GetBlockID().IsValid())
            m_uvsIndicesVec[i].SetDiscarded(true);
        m_uvsIndicesVec[i].SetDirty(false);
        }
#if DEBUG && SM_TRACE_RASTER_TEXTURING 

    s += " N OF INDICE ARRAYS " + std::to_string(GetNbPtsIndiceArrays());
    for (size_t i = 0; i <GetNbPtsIndiceArrays(); ++i)
        {
        s += " ARRAY " + std::to_string(i) + " HAS " + std::to_string(GetNbPtsIndices(i)) + " INDICES ";
        }
#endif
    m_uvVec.SetStore(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetUVStore());
    if (m_uvVec.GetPool() == NULL) m_uvVec.SetPool(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetUVPool());
    m_uvVec.SetBlockID(m_nodeHeader.m_uvID);
    if (m_uvVec.GetBlockID().IsValid())
        m_uvVec.SetDiscarded(true);
    m_uvVec.SetDirty(false);

    m_textureVec.resize(m_nodeHeader.m_textureID.size());
    for (size_t i = 0; i < m_nodeHeader.m_textureID.size(); ++i)
        {
        m_textureVec[i].SetStore(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetTexturesStore());
        if (m_textureVec[i].GetPool() == NULL) m_textureVec[i].SetPool(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetTexturesPool());
        m_textureVec[i].SetBlockID(m_nodeHeader.m_textureID[i]);
        if (m_textureVec[i].GetBlockID().IsValid())
            m_textureVec[i].SetDiscarded(true);
        m_textureVec[i].SetDirty(false);
        }
#ifdef SM_BESQL_FORMAT
    /*m_ptsIndiceVec[0].DoOnDiscard(std::bind([] (SMMeshIndexNode<POINT, EXTENT>* node) ->void
        {
        if (node->m_ptsIndiceVec.size() > 0 && node->m_ptsIndiceVec[0].GetBlockID().IsValid() && node->m_ptsIndiceVec[0].GetBlockID()!= IDTMFile::GetNullNodeID() && node->m_ptsIndiceVec[0].GetBlockID() != node->GetBlockID())
            {
            if(!node->Discarded())node->SetBlockID(node->m_ptsIndiceVec[0].GetBlockID());
            else
                {
                node->Inflate();
                node->SetBlockID(node->m_ptsIndiceVec[0].GetBlockID());
                node->Discard();
                }
            }
        }, const_cast<SMMeshIndexNode<POINT, EXTENT>*>(this)));*/
#endif
}

#ifdef INDEX_DUMPING_ACTIVATED
template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::DumpOctTreeNode(FILE* pi_pOutputXmlFileStream,
                             bool pi_OnlyLoadedNode) const
    {
    if ((pi_OnlyLoadedNode == true) && (IsLoaded() == false))
        return;

    if (!IsLoaded())
        Load();

    char   TempBuffer[3000];
    int    NbChars;
    size_t NbWrittenChars;
    __int64 nodeId;

    if (GetBlockID().IsValid())
        {
        nodeId = GetBlockID().m_integerID;
        }
    else
        {
        nodeId = IDTMFile::GetNullNodeID();
        }

    NbChars = sprintf(TempBuffer, "<ChildNode NodeId=\"%lli\" TotalPoints=\"%lli\" SplitDepth=\"%zi\" ArePoints3d=\"%i\">", nodeId, GetCount(), GetSplitDepth(), m_nodeHeader.m_arePoints3d ? 1 : 0);

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbChars == NbWrittenChars);

    //Extent
    NbChars = sprintf(TempBuffer,
                      "<NodeExtent><MinX>%.20f</MinX><MaxX>%.20f</MaxX><MinY>%.20f</MinY><MaxY>%.20f</MaxY><MinZ>%.20f</MinZ><MaxZ>%.20f</MaxZ></NodeExtent>\n",
                      ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
                      ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
                      ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
                      ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
                      ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                      ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent));

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    if (m_nodeHeader.m_contentExtentDefined)
        {
        NbChars = sprintf(TempBuffer,
                          "<ContentExtent><MinX>%.20f</MinX><MaxX>%.20f</MaxX><MinY>%.20f</MinY><MaxY>%.20f</MaxY><MinZ>%.20f</MinZ><MaxZ>%.20f</MaxZ></ContentExtent>\n",
                          ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_contentExtent),
                          ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_contentExtent),
                          ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_contentExtent),
                          ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_contentExtent),
                          ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_contentExtent),
                          ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_contentExtent));

        NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

        HASSERT(NbWrittenChars == NbChars);
        }


    //Number Of Points
    NbChars = sprintf(TempBuffer, "<NbOfPoints>%u</NbOfPoints>\n", GetNbObjects());

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    //Cumulative Number Of Points
    NbChars = sprintf(TempBuffer, "<CumulNbOfPoints>%llu</CumulNbOfPoints>\n", GetCount());

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    //Number Of Points
    NbChars = sprintf(TempBuffer, "<NbOfIndexes>%zu</NbOfIndexes>\n", m_nodeHeader.m_nbFaceIndexes);

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    //Level
    NbChars = sprintf(TempBuffer, "<Level>%zi</Level>\n", m_nodeHeader.m_level);

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    // SplitTreshold
    NbChars = sprintf(TempBuffer, "<SplitTreshold>%zi</SplitTreshold>", m_nodeHeader.m_SplitTreshold);

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    // Balanced
    if (m_nodeHeader.m_balanced)
        NbChars = sprintf(TempBuffer, "<Balanced>true</Balanced>");
    else
        NbChars = sprintf(TempBuffer, "<Balanced>false</Balanced>");

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);


    //View Dependent Metrics
    /*
    NbChars = sprintf(TempBuffer,
    "<ViewDependentMetrics>%.3f</ViewDependentMetrics>",
    m_nodeHeader.m_ViewDependentMetrics[0]);

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);
    */


    // Neighbor Node    
    NbChars = sprintf(TempBuffer, "<NeighborNode> ");

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    for (size_t neighborPosInd = 0; neighborPosInd < MAX_NEIGHBORNODES_COUNT; neighborPosInd++)
        {
        for (size_t neighborInd = 0; neighborInd < m_nodeHeader.m_apNeighborNodeID[neighborPosInd].size(); neighborInd++)
            {
            NbChars = sprintf(TempBuffer, "P %zi I %zi Id %lli ", neighborPosInd, neighborInd, m_nodeHeader.m_apNeighborNodeID[neighborPosInd][neighborInd].m_integerID);

            NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

            HASSERT(NbWrittenChars == NbChars);
            }
        }

    NbChars = sprintf(TempBuffer, "</NeighborNode>");

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    //GraphID
    NbChars = sprintf(TempBuffer, "<GraphID>%llu</GraphID>\n", m_nodeHeader.m_graphID.m_integerID);

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    for (size_t i = 0; i < m_nodeHeader.m_ptsIndiceID.size(); ++i)
        {
        NbChars = sprintf(TempBuffer, "<IndiceID>%llu</IndiceID>\n", m_nodeHeader.m_ptsIndiceID[i].m_integerID);

        NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

        HASSERT(NbWrittenChars == NbChars);
        }

    for (size_t i = 0; i <m_ptsIndiceVec.size(); ++i)
        {
        if (m_ptsIndiceVec[i].Discarded() && m_ptsIndiceVec[i].GetBlockID().IsValid()) m_ptsIndiceVec[i].Inflate();
        NbChars = sprintf(TempBuffer, "<IndiceVec Number=\"%zi\" BlockID=\"%lli\">%zu</IndiceVec>\n", i, m_ptsIndiceVec[i].GetBlockID().m_integerID, m_ptsIndiceVec[i].size());

        NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

        HASSERT(NbWrittenChars == NbChars);
        }

    // Neighbor Stitching    
    NbChars = sprintf(TempBuffer, "<NeighborStitching> ");

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    for (size_t neighborPosInd = 0; neighborPosInd < MAX_NEIGHBORNODES_COUNT; neighborPosInd++)
        {
        if (m_nodeHeader.m_apAreNeighborNodesStitched[neighborPosInd] == true)
            {
            NbChars = sprintf(TempBuffer, "1 ");

            NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

            HASSERT(NbWrittenChars == NbChars);
            }
        else
            {
            NbChars = sprintf(TempBuffer, "0 ");

            NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

            HASSERT(NbWrittenChars == NbChars);
            }
        }

    NbChars = sprintf(TempBuffer, "</NeighborStitching>");

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);


    if (!m_nodeHeader.m_IsLeaf)
        {
        if (m_pSubNodeNoSplit != NULL)
            {
            static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_pSubNodeNoSplit))->DumpOctTreeNode(pi_pOutputXmlFileStream, pi_OnlyLoadedNode);
            }
        else
            {
            for (size_t indexNode = 0; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                {
                static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->DumpOctTreeNode(pi_pOutputXmlFileStream, pi_OnlyLoadedNode);
                }
            }
        }

    NbChars = sprintf(TempBuffer, "</ChildNode>\n");

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbChars == NbWrittenChars);

        
    }

#endif
//=======================================================================================
// @bsimethod                                                  Elenie.Godzaridis 03/15
//=======================================================================================
template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::Unload() const
    {
    if (m_featureDefinitions.size() > 0)
        {
        for (auto& vec : m_featureDefinitions) if(!vec.Discarded()) vec.Discard();
        }
    if (!m_differenceSets.Discarded())
        {
        if(m_differenceSets.size() > 0) m_differenceSets.Discard();
        else m_differenceSets.SetDirty(false);
        }
    SMPointIndexNode<POINT, EXTENT>::Unload();
    }

//=======================================================================================
// @bsimethod                                                  Elenie.Godzaridis 03/15
//=======================================================================================
template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::CreateGraph(bool shouldPinGraph) const
    {
    m_graphVec.SetDiscarded(false);
    if (m_graphVec.size() == 0) m_graphVec.push_back(MTGGraph());
    if (shouldPinGraph)m_graphVec.Pin();
    }

//=======================================================================================
// @bsimethod                                                  Elenie.Godzaridis 03/15
//=======================================================================================
//NEEDS_WORK_SM : Need to be redesign like difference set.
template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::LoadGraph(bool shouldPinGraph) const
    {
    if (m_graphVec.GetBlockID().IsValid() && (!IsGraphLoaded() || m_graphVec.Discarded()))
        {
        if (shouldPinGraph) m_graphInflateMutex.lock();
        if (!m_graphVec.Discarded())
            {
            m_graphVec.SetDirty(false);
            m_graphVec.Discard();
            }
        if (shouldPinGraph) m_graphVec.Pin();
        else m_graphVec.Inflate();
        m_isGraphLoaded = true;
        if (shouldPinGraph) m_graphInflateMutex.unlock();
        }
    }

//=======================================================================================
// @bsimethod                                                  Elenie.Godzaridis 03/15
//=======================================================================================
template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::StoreGraph() const
    {
    const_cast<SMMeshIndexNode<POINT, EXTENT>*>(this)->ReleaseGraph();
    if (m_graphVec.size() > 0 && !m_graphVec.Discarded()) m_graphVec.Discard();
        m_nodeHeader.m_graphID = m_graphVec.GetBlockID();
    }



template <class POINT, class EXTENT> bool SMMeshIndexNode<POINT, EXTENT>::IsPtsIndiceLoaded() const
    {
    return m_isPtsIndiceLoaded;
    }

template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::LoadPtsIndice() const
    {
        for (int i = 0; i < m_ptsIndiceVec.size(); i++)
        {
            if (m_ptsIndiceVec[i].GetBlockID().IsValid())
            {
                if (!m_ptsIndiceVec[i].Discarded())
                {
                    m_ptsIndiceVec[i].SetDirty(false);
                    m_ptsIndiceVec[i].Discard();
                }
                m_ptsIndiceVec[i].Inflate();
                
            }
        }
        m_isPtsIndiceLoaded = true;
    }

template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::StorePtsIndice(size_t textureID) const
    {
    if (m_nodeHeader.m_ptsIndiceID.size() < textureID + 1) m_nodeHeader.m_ptsIndiceID.resize(textureID + 1);
        {
        if(m_ptsIndiceVec[textureID].IsDirty() && !m_ptsIndiceVec[textureID].Discarded()) m_ptsIndiceVec[textureID].Discard();
        if (m_ptsIndiceVec[textureID].GetBlockID().IsValid())
            m_nodeHeader.m_ptsIndiceID[textureID] = m_ptsIndiceVec[textureID].GetBlockID();
        }
    if (!m_SMIndex->m_useSTMFormat)
        {
        if (textureID == 0 && !GetBlockID().IsValid() && m_ptsIndiceVec[textureID].GetBlockID().IsValid() && m_nodeHeader.m_ptsIndiceID[0] != IDTMFile::GetNullNodeID())
            {
            SetBlockID(m_nodeHeader.m_ptsIndiceID[0]);
            if (GetParentNode() != nullptr) ((SMMeshIndexNode<POINT, EXTENT>*)GetParentNode().GetPtr())->AdviseSubNodeIDChanged(const_cast<SMMeshIndexNode<POINT, EXTENT>*>(this));
            AdviseParentNodeIDChanged();
            for (size_t neighborPosInd = 0; neighborPosInd < MAX_NEIGHBORNODES_COUNT; neighborPosInd++)
                {
                for (size_t neighborInd = 0; neighborInd < m_apNeighborNodes[neighborPosInd].size(); neighborInd++)
                    {
                    ((SMMeshIndexNode<POINT, EXTENT>*)m_apNeighborNodes[neighborPosInd][neighborInd].GetPtr())->AdviseNeighborNodeIDChanged(const_cast<SMMeshIndexNode<POINT, EXTENT>*>(this));
                    }
                }
            }
        }
    }

template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::PushPtsIndices(size_t texture_id, const int32_t* indices, size_t size) const
    {
    if (m_ptsIndiceVec.size() < texture_id + 1)
        {
        for(int i=0; i<m_ptsIndiceVec.size(); i++)
        StorePtsIndice(i);
        m_ptsIndiceVec.resize(texture_id + 1);
        }
    if (m_ptsIndiceVec[texture_id].GetStore() == nullptr) m_ptsIndiceVec[texture_id].SetStore(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetPtsIndicesStore().GetPtr());
    if (m_ptsIndiceVec[texture_id].GetPool() == nullptr) m_ptsIndiceVec[texture_id].SetPool(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetPtsIndicesPool());
    if (!m_SMIndex->m_useSTMFormat)
        {
        m_ptsIndiceVec[texture_id].SetBlockID(m_ptsIndiceVec[0].GetBlockID());
        }
    m_ptsIndiceVec[texture_id].push_back(indices, size);
    }

template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::ReplacePtsIndices(size_t texture_id, const int32_t* indices, size_t size) const
    {
    m_ptsIndiceVec[texture_id].clear();
    m_ptsIndiceVec[texture_id].push_back(indices, size);
    }

template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::ClearPtsIndices(size_t texture_id) const
    {
        if (m_ptsIndiceVec.size() > texture_id)
            m_ptsIndiceVec[texture_id].clear();
    }




template <class POINT, class EXTENT> bool SMMeshIndexNode<POINT, EXTENT>::IsUVLoaded() const
    {
    return m_isUVLoaded;
    }

template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::PushUV( const DPoint2d* uv, size_t size) const
    {
    if (!m_SMIndex->m_useSTMFormat)
        {
        m_uvVec.SetBlockID(GetBlockID());
        }
        m_uvVec.push_back(uv, size);
    }
#pragma optimize("", off)
template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::StoreUV() const
    {
    if(m_uvVec.IsDirty() && !m_uvVec.Discarded()) m_uvVec.Discard();
    if (m_uvVec.GetBlockID().IsValid())
        {
        m_nodeHeader.m_uvID = m_uvVec.GetBlockID();
        }
    }
#pragma optimize("", on)
template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::ClearUV() const
    {
    m_uvVec.clear();
    }


template <class POINT, class EXTENT> bool SMMeshIndexNode<POINT, EXTENT>::IsTextureLoaded() const
{
    return m_isTextureLoaded;
}

template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::StoreTexture(size_t textureID) const
{
    if (m_nodeHeader.m_textureID.size() < textureID + 1) m_nodeHeader.m_textureID.resize(textureID + 1);
if (m_textureVec.size() > 0)
    {
        if (m_textureVec[textureID].IsDirty() && !m_textureVec[textureID].Discarded()) m_textureVec[textureID].Discard();
        if (m_textureVec[textureID].GetBlockID().IsValid())
            m_nodeHeader.m_textureID[textureID] = m_textureVec[textureID].GetBlockID();
    }
}

template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::PushTexture(size_t texture_id, const Byte* texture, size_t size) const
{
if (m_textureVec.size() < texture_id + 1)
        {
        for(int i=0; i<m_textureVec.size(); i++)
        StoreTexture(i);
        m_textureVec.resize(texture_id + 1);
        }
m_textureVec[texture_id].SetStore(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetTexturesStore().GetPtr());
if (m_textureVec[texture_id].GetPool() == nullptr) m_textureVec[texture_id].SetPool(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetTexturesPool());
if (!m_SMIndex->m_useSTMFormat)
    {
    m_textureVec[texture_id].SetBlockID(GetBlockID());
    }
    m_textureVec[texture_id].push_back(texture, size);
}

template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::ClearTexture(size_t texture_id) const
{
    m_textureVec[texture_id].clear();
}





template <class POINT, class EXTENT> bool SMMeshIndexNode<POINT, EXTENT>::IsUVsIndicesLoaded() const
{
    return m_isUVsIndicesLoaded;
}

template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::LoadUVsIndices() const
{
    for (int i = 0; i < m_uvsIndicesVec.size(); i++)
    {
        if (m_uvsIndicesVec[i].GetBlockID().IsValid())
        {
            if (!m_uvsIndicesVec[i].Discarded())
            {
                m_uvsIndicesVec[i].SetDirty(false);
                m_uvsIndicesVec[i].Discard();
            }
            m_uvsIndicesVec[i].Inflate();

        }
    }
    m_isUVsIndicesLoaded = true;
}

template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::StoreUVsIndices(size_t textureID) const
{
    if (m_nodeHeader.m_uvsIndicesID.size() < textureID + 1) m_nodeHeader.m_uvsIndicesID.resize(textureID + 1);
    {
        if (m_uvsIndicesVec[textureID].IsDirty() && !m_uvsIndicesVec[textureID].Discarded()) m_uvsIndicesVec[textureID].Discard();
        if (m_uvsIndicesVec[textureID].GetBlockID().IsValid())
            m_nodeHeader.m_uvsIndicesID[textureID] = m_uvsIndicesVec[textureID].GetBlockID();
    }
}

template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::PushUVsIndices(size_t texture_id, const int32_t* uvsIndices, size_t size) const
{
    if (m_uvsIndicesVec.size() < texture_id + 1)
        {
        for(int i=0; i<m_uvsIndicesVec.size(); i++)
        StoreUVsIndices(i);
        m_uvsIndicesVec.resize(texture_id + 1);
        }
    m_uvsIndicesVec[texture_id].SetStore(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetUVsIndicesStore().GetPtr());
    if (m_uvsIndicesVec[texture_id].GetPool() == nullptr) m_uvsIndicesVec[texture_id].SetPool(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetUVsIndicesPool());
    if (!m_SMIndex->m_useSTMFormat)
        {
        m_uvsIndicesVec[texture_id].SetBlockID(GetBlockID());
        }
    m_uvsIndicesVec[texture_id].push_back(uvsIndices, size);
}

template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::ClearUVsIndices(size_t texture_id) const
{
    if (m_ptsIndiceVec.size() > texture_id)
        m_uvsIndicesVec[texture_id].clear();
}

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 08/14
//=======================================================================================
template<class POINT, class EXTENT> ISMPointIndexMesher<POINT, EXTENT>* SMMeshIndexNode<POINT, EXTENT>::GetMesher2_5d() const
    {
    if (!IsLoaded())
        Load();
    // Non validation of invariants in intentional ... this gets called during a
    // temporary state during the creation of sub-nodes, by the subnodes
    return(m_mesher2_5d);
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 08/14
//=======================================================================================
template<class POINT, class EXTENT> ISMPointIndexMesher<POINT, EXTENT>* SMMeshIndexNode<POINT, EXTENT>::GetMesher3d() const
    {
    if (!IsLoaded())
        Load();
    // Non validation of invariants in intentional ... this gets called during a
    // temporary state during the creation of sub-nodes, by the subnodes
    return(m_mesher3d);
    }


//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 08/14
//=======================================================================================
template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::Mesh()
    {
    if (!IsLoaded())
        Load();


    HINVARIANTS;

    // If there are sub-nodes and these need filtering then first do the subnodes
    if (HasRealChildren())
        {
        if (IsParentOfARealUnsplitNode())
            {
#ifdef __HMR_DEBUG
            if ((static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->m_unspliteable) || (static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->m_parentOfAnUnspliteableNode))
                this->m_parentOfAnUnspliteableNode = true;
#endif                        

            if (static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->NeedsMeshing())
                static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->Mesh();
            }
        else
            {
            for (size_t indexNode = 0; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                {
#ifdef __HMR_DEBUG
                if ((static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->m_unspliteable) || (static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->m_parentOfAnUnspliteableNode))
                    this->m_parentOfAnUnspliteableNode = true;
#endif
                if (static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->NeedsMeshing())
                    {
                    if (s_useThreadsInMeshing && m_nodeHeader.m_level == 0 && !m_nodeHeader.m_arePoints3d)
                        {
                        RunOnNextAvailableThread(std::bind([] (SMMeshIndexNode<POINT, EXTENT>* node,size_t threadId) ->void
                            {
                            node->Mesh();
                            SetThreadAvailableAsync(threadId);
                            }, static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode])), std::placeholders::_1));
                        }
                    else static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->Mesh();
                    }
                }

            }

        }
    else
        {
        assert(this->NeedsMeshing() == true);
        //assert(this->m_nodeHeader.m_balanced == true);
            bool isMeshed;

            if (m_nodeHeader.m_arePoints3d)
                {
                isMeshed = m_mesher3d->Mesh(this);
                }
            else
                {
                isMeshed = m_mesher2_5d->Mesh(this);
                }

            if (isMeshed)
                {
                SetDirty(true);
                }
        }

    if (m_nodeHeader.m_level == 0 && s_useThreadsInMeshing)
        WaitForThreadStop();
    // Now filtering can be performed using the sub-nodes filtered data. This data
    // accessed using the HPMPooledVector interface the Node is a descendant of.
    // Do not hesitate to increase the HPMPooledVector interface if required.
    // The result of the filtering must be added to the Node itself in the
    // HPMVectorPool<POINT> descendant class using the push_back interface.
    // If refiltering is required then clear() must be called beforehand.
    // The member m_nodeHeader.m_filtered should be set to true after the filtering process
    // All members that must be serialized in the file must be added in the
    // m_nodeHeader fields/struct and these will automatically be serialized in the
    // store. Note that changing this structure automatically
    // renders invalid any previous file.

    ValidateInvariants();
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 08/14
//=======================================================================================
template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::Stitch(int pi_levelToStitch, vector<SMMeshIndexNode<POINT, EXTENT>*>* nodesToStitch)
    {
    if (!IsLoaded())
        Load();

    HINVARIANTS;

//    size_t nodeInd;

    if (pi_levelToStitch == -1 || this->m_nodeHeader.m_level == pi_levelToStitch && this->GetNbObjects() > 0)
        {
            if (nodesToStitch != 0)
                {
                nodesToStitch->push_back(this);
                }
            else
                {
#if 0
                bool wait = true;
                while (wait)
                    {
                    for (size_t t = 0; t < 8; ++t)
                        {
                        bool expected = false;
                        if (s_areThreadsBusy[t].compare_exchange_weak(expected, true))
                            {
                            wait = false;
                            size_t threadId = t;
                            std::atomic<bool>* areThreadsBusy = s_areThreadsBusy;
                            std::thread* threadP = s_threads;
                            s_threads[t] = std::thread([threadId, this, areThreadsBusy, threadP] ()
                                {
                                bool isStitched;

                                if (this->AreAllNeighbor2_5d() && !this->m_nodeHeader.m_arePoints3d)
                                    {
                                    isStitched = this->m_mesher2_5d->Stitch(this);
                                    }
                                else
                                    {
                                    isStitched = this->m_mesher3d->Stitch(this);
                                    }

                                if (isStitched)
                                    this->SetDirty(true);
                                SetThreadAvailableAsync(threadId);
                                /*  std::thread t = std::thread([areThreadsBusy, threadId, threadP] ()
                                      {
                                      threadP[threadId].join();
                                      bool expected = true;
                                      areThreadsBusy[threadId].compare_exchange_strong(expected, false);
                                      assert(expected);
                                      });
                                      t.detach();*/
                                });
                            break;
                            }
                        }
                    }
#elif 0
                RunOnNextAvailableThread(std::bind([] (SMMeshIndexNode<POINT, EXTENT>* node, size_t threadId)
                    {
                    bool isStitched;

                    if (node->AreAllNeighbor2_5d() && !node->m_nodeHeader.m_arePoints3d)
                        {
                        isStitched = node->m_mesher2_5d->Stitch(node);
                        }
                    else
                        {
                        isStitched = node->m_mesher3d->Stitch(node);
                        }

                    if (isStitched)
                        node->SetDirty(true);
                    SetThreadAvailableAsync(threadId);
                    },node);
#else
                bool isStitched;

                if (AreAllNeighbor2_5d() && !this->m_nodeHeader.m_arePoints3d)
                    {
                    isStitched = m_mesher2_5d->Stitch(this);
                    }
                else
                    {
                    isStitched = m_mesher3d->Stitch(this);
                    }

                if (isStitched)
                    SetDirty(true);
#endif
                }
            }
       // }

        if (pi_levelToStitch == -1 || (int)this->m_nodeHeader.m_level < pi_levelToStitch)
            {
            if (!m_nodeHeader.m_IsLeaf)
                {
                if (m_pSubNodeNoSplit != NULL)
                    {
#ifdef __HMR_DEBUG
                    if ((static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->m_unspliteable) || (static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->m_parentOfAnUnspliteableNode))
                        this->m_parentOfAnUnspliteableNode = true;
#endif

                    static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->Stitch(pi_levelToStitch, nodesToStitch);

                    }
                else
                    {
                    for (size_t indexNode = 0; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                        {
#ifdef __HMR_DEBUG
                        if ((static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->m_unspliteable) || (static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->m_parentOfAnUnspliteableNode))
                            this->m_parentOfAnUnspliteableNode = true;
#endif                
                        if (this->m_nodeHeader.m_level == 0 && nodesToStitch == 0 && pi_levelToStitch > 1 && s_useThreadsInStitching)
                            {
                            RunOnNextAvailableThread(std::bind([] (SMMeshIndexNode<POINT, EXTENT>* node, int pi_levelToStitch, size_t threadId) ->void
                                {
                                node->Stitch(pi_levelToStitch, 0);
                                SetThreadAvailableAsync(threadId);
                                }, static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode])), pi_levelToStitch, std::placeholders::_1));
                            }
                        else static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->Stitch(pi_levelToStitch, nodesToStitch);
                        }
                    }
                }
            }
        //don't return until all threads are done
        if (m_nodeHeader.m_level == 0 && nodesToStitch == 0 && s_useThreadsInStitching)
            WaitForThreadStop();
       /* if (m_nodeHeader.m_level == 0 && pi_levelToStitch == 0)
            {
            m_nodeHeader.m_totalCountDefined = false;
            }*/
        // Now filtering can be performed using the sub-nodes filtered data. This data
        // accessed using the HPMPooledVector interface the Node is a descendant of.
        // Do not hesitate to increase the HPMPooledVector interface if required.
        // The result of the filtering must be added to the Node itself in the
        // HPMVectorPool<POINT> descendant class using the push_back interface.
        // If refiltering is required then clear() must be called beforehand.
        // The member m_nodeHeader.m_filtered should be set to true after the filtering process
        // All members that must be serialized in the file must be added in the
        // m_nodeHeader fields/struct and these will automatically be serialized in the
        // store. Note that changing this structure automatically
        // renders invalid any previous file.

        ValidateInvariants();
    }

template<class POINT, class EXTENT> bool SMMeshIndexNode<POINT, EXTENT>::Inflate() const
        {
#ifdef SM_BESQL_FORMAT
     /*   if (m_ptsIndiceVec.size() > 0 && m_ptsIndiceVec[0].GetBlockID().IsValid() && m_ptsIndiceVec[0].GetBlockID()!= IDTMFile::GetNullNodeID() && m_ptsIndiceVec[0].GetBlockID() != GetBlockID())
            SetBlockID(m_ptsIndiceVec[0].GetBlockID());*/
#endif
            return HPMStoredPooledVector<POINT>::Inflate();
        }

inline bool IsLinearFeature(IDTMFile::FeatureType type)
    {
    DTMFeatureType dtmType = (DTMFeatureType)type;
    return dtmType == DTMFeatureType::Breakline || dtmType == DTMFeatureType::SoftBreakline || dtmType == DTMFeatureType::ContourLine || dtmType == DTMFeatureType::GraphicBreak;
    }

inline bool IsClosedFeature(IDTMFile::FeatureType type)
    {
    DTMFeatureType dtmType = (DTMFeatureType)type;
    return dtmType == DTMFeatureType::Hole || dtmType == DTMFeatureType::Island || dtmType == DTMFeatureType::Void || dtmType == DTMFeatureType::BreakVoid ||
        dtmType == DTMFeatureType::Polygon || dtmType == DTMFeatureType::Region || dtmType == DTMFeatureType::Contour || dtmType == DTMFeatureType::Hull;
    }

static size_t s_featuresAddedToTree = 0;

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 08/15
//=======================================================================================
template<class EXTENT> void ClipFeatureDefinition(IDTMFile::FeatureType type, EXTENT clipExtent, bvector<DPoint3d>& points, DRange3d& extent, const bvector<DPoint3d>& origPoints, DRange3d& origExtent)
    {

    if (/*IsClosedFeature(type) ||*/ (origExtent.low.x >= ExtentOp<EXTENT>::GetXMin(clipExtent) && origExtent.low.y >= ExtentOp<EXTENT>::GetYMin(clipExtent) && origExtent.low.z >= ExtentOp<EXTENT>::GetZMin(clipExtent)
        && origExtent.high.x <= ExtentOp<EXTENT>::GetXMax(clipExtent) && origExtent.high.y <= ExtentOp<EXTENT>::GetYMax(clipExtent) && origExtent.high.z <= ExtentOp<EXTENT>::GetZMax(clipExtent)))
        {
        points.insert(points.end(), origPoints.begin(), origPoints.end());
        extent = origExtent;
        return;
        }
    DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(clipExtent), ExtentOp<EXTENT>::GetYMin(clipExtent), ExtentOp<EXTENT>::GetZMin(clipExtent),
                                        ExtentOp<EXTENT>::GetXMax(clipExtent), ExtentOp<EXTENT>::GetYMax(clipExtent), ExtentOp<EXTENT>::GetZMax(clipExtent));
    if (IsClosedFeature(type))
        {
        DPoint3d origins[6];
        DVec3d normals[6];
        nodeRange.Get6Planes(origins, normals);
        DPlane3d planes[6];
        for (size_t i = 0; i < 6; ++i)
            {
            planes[i] = DPlane3d::FromOriginAndNormal(origins[i], normals[i]);
            }

        points.insert(points.end(), origPoints.begin(), origPoints.end());
        for (auto& plane : planes)
            {
            double sign = 0;
            bool planeCutsPoly = false;
            for (size_t j = 0; j < points.size() && !planeCutsPoly; j++)
                {
                double sideOfPoint = plane.Evaluate(points[j]);
                if (fabs(sideOfPoint) < 1e-6) sideOfPoint = 0;
                if (sign == 0) sign = sideOfPoint;
                else if ((sign > 0 && sideOfPoint < 0) || (sign < 0 && sideOfPoint > 0))
                    planeCutsPoly = true;
                }
            if (!planeCutsPoly) continue;                
            bvector<DPoint3d> points2(points.size() + 10);
            int nPlaneClipSize = (int)points2.size();
            int nLoops = 0;
            bsiPolygon_clipToPlane(&points2[0], &nPlaneClipSize, &nLoops, (int)points2.size(), &points[0], (int)points.size(), &plane);
            if (nPlaneClipSize > 0)
                {
                points.clear();
                points2.resize(nPlaneClipSize);
                for (auto& pt : points2)
                    {
                    if (pt.x < DBL_MAX)
                        points.push_back(pt);
                    //else break;
                    }
                }
            }
        extent = DRange3d::From(points);
        return;
        }
    if (IsLinearFeature(type))
        {
        DPoint3d SENTINEL_PT = DPoint3d::From(DBL_MAX, DBL_MAX, DBL_MAX);
        bool withinExtent = false;
        for (size_t pt = 0; pt < origPoints.size(); ++pt)
            {
            bool isPointInRange = nodeRange.IsContained(origPoints[pt]);
            if (!withinExtent && isPointInRange && pt > 0)
                {
                points.push_back(origPoints[pt - 1]);
                }
            if (isPointInRange)
                {
                if (points.size() == 0) extent = DRange3d::From(origPoints[pt]);
                else extent.Extend(origPoints[pt]);
                points.push_back(origPoints[pt]);
                }
            if (!isPointInRange && withinExtent && pt < origPoints.size())
                {
                points.push_back(origPoints[pt]);
                points.push_back(SENTINEL_PT);
                }
            withinExtent = isPointInRange;
            }
        }
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 08/15
//=======================================================================================
    template<class POINT, class EXTENT> size_t SMMeshIndexNode<POINT, EXTENT>::AddFeatureDefinitionUnconditional(IDTMFile::FeatureType type, bvector<DPoint3d>& points, DRange3d& extent)
        {
        if (!IsLoaded())
            Load();
        if (m_DelayedSplitRequested)
            SplitNode(GetDefaultSplitPosition());


        DRange3d extentClipped;
        bvector<DPoint3d> pointsClipped;
        ClipFeatureDefinition(type, m_nodeHeader.m_nodeExtent, pointsClipped, extentClipped, points, extent);
        if (!HasRealChildren() && this->size() == 0) m_nodeHeader.m_arePoints3d = false;
        if (!m_nodeHeader.m_arePoints3d) SetNumberOfSubNodesOnSplit(4);
        if (!HasRealChildren() && (this->size() + pointsClipped.size() >= m_nodeHeader.m_SplitTreshold))
            {
            // There are too much objects ... need to split current node
            SplitNode(GetDefaultSplitPosition());
            }
        else if (m_delayedDataPropagation && (this->size() + pointsClipped.size() >= m_nodeHeader.m_SplitTreshold))
            {
            PropagateDataDownImmediately(false);
            }
        if (pointsClipped.size() == 0) return false;


        m_nodeHeader.m_totalCount += pointsClipped.size();
        EXTENT featureExtent = ExtentOp<EXTENT>::Create(extentClipped.low.x, extentClipped.low.y, extentClipped.low.z, extentClipped.high.x, extentClipped.high.y, extentClipped.high.z);
        if (!m_nodeHeader.m_contentExtentDefined)
            {
            m_nodeHeader.m_contentExtent = featureExtent;
            m_nodeHeader.m_contentExtentDefined = true;
            }
        else
            {
            m_nodeHeader.m_contentExtent = ExtentOp<EXTENT>::MergeExtents(m_nodeHeader.m_contentExtent, featureExtent);
            }

        size_t added = 0;
        if (!HasRealChildren() || (m_delayedDataPropagation && (this->size() + pointsClipped.size() < m_nodeHeader.m_SplitTreshold)))
            {
            ++s_featuresAddedToTree;
            vector<int32_t> indexes;
            DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                                ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent));
            for (auto pt : pointsClipped)
                {
                if (pt.x == DBL_MAX)
                    {
                    indexes.push_back(INT_MAX);
                    continue;
                    }
                if (nodeRange.IsContained(pt)) ++added;
                POINT pointToInsert = PointOp<POINT>::Create(pt.x, pt.y, pt.z);
                this->push_back(pointToInsert);
                indexes.push_back((int32_t)this->size()-1);
                }
            if (m_featureDefinitions.capacity() < m_featureDefinitions.size() +1) for(auto& def : m_featureDefinitions) if(!def.Discarded()) def.Discard();
            m_featureDefinitions.resize(m_featureDefinitions.size() + 1);
            auto& newFeatureDef = m_featureDefinitions.back();
            newFeatureDef.SetStore(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetFeatureStore());
            newFeatureDef.SetPool(dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetFeaturePool());
            newFeatureDef.push_back((int32_t)type);
            newFeatureDef.push_back(&indexes[0], indexes.size());
            }
        else
            {
            if (IsParentOfARealUnsplitNode())
                added = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pSubNodeNoSplit)->AddFeatureDefinitionUnconditional(type, pointsClipped, extentClipped);
            else
                {
                    for (size_t indexNode = 0; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                        {
                        added += dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_apSubNodes[indexNode])->AddFeatureDefinition(type, pointsClipped, extentClipped, true);
                        }
                }
            }
     
        SetDirty(true);
        return added;
        }

    //=======================================================================================
    // @bsimethod                                                   Elenie.Godzaridis 08/15
    //=======================================================================================
    template<class POINT, class EXTENT>  size_t  SMMeshIndexNode<POINT, EXTENT>::AddFeatureDefinition(IDTMFile::FeatureType type, bvector<DPoint3d>& points, DRange3d& extent, bool ExtentFixed)
        {
        assert(points.size()>0);
        if (s_inEditing)
            {
            InvalidateFilteringMeshing();
            }
        if (m_DelayedSplitRequested)
            SplitNode(GetDefaultSplitPosition());

        if (!ExtentFixed && GetParentNode() == NULL && m_nodeHeader.m_IsLeaf)
            {
            m_nodeHeader.m_nodeExtent = ExtentOp<EXTENT>::MergeExtents(GetNodeExtent(), ExtentOp<EXTENT>::Create(extent.low.x, extent.low.y, extent.low.z, extent.high.x, extent.high.y, extent.high.z));

            if (ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent) &&
                ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent))
                {
                ExtentOp<EXTENT>::SetXMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent)));
                ExtentOp<EXTENT>::SetZMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent)));
                }
            else
                if (ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent) &&
                    ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent))
                    {
                    ExtentOp<EXTENT>::SetYMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent)));
                    ExtentOp<EXTENT>::SetZMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent)));
                    }
                else
                    if (ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent) &&
                        ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent))
                        {
                        ExtentOp<EXTENT>::SetXMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent)));
                        ExtentOp<EXTENT>::SetYMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent)));
                        }
            if (points.size() + this->size() >= m_nodeHeader.m_SplitTreshold)
                {
                return AddFeatureDefinition(type, points, extent,true);
                }
            else
                {
                return AddFeatureDefinitionUnconditional(type,points,extent);              
                }
        }
    else
        {
        DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                            ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent));
        if (extent.IntersectsWith(nodeRange))
            {
            return AddFeatureDefinitionUnconditional(type, points, extent);
            }
        }
        return 0;

    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 08/15
//=======================================================================================
template<class POINT, class EXTENT>  size_t SMMeshIndexNode<POINT, EXTENT>::CountAllFeatures()
    {
    size_t nFeatures = IsLeaf() ? m_featureDefinitions.size() : 0;
    if (m_pSubNodeNoSplit != NULL && !m_pSubNodeNoSplit->IsVirtualNode())
        {
        nFeatures += dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pSubNodeNoSplit)->CountAllFeatures();
        }
    else if (!IsLeaf())
        {
        for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
            {
            assert(m_apSubNodes[indexNodes] != nullptr);
            nFeatures += dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_apSubNodes[indexNodes])->CountAllFeatures();
            }
        }
    return nFeatures;
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 08/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::OnPushNodeDown()
    {
    PropagateFeaturesToChildren();
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 08/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::OnPropagateDataDown()
    {
    PropagateFeaturesToChildren();
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 08/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::PropagateFeaturesToChildren()
    {
    bvector<bvector<DPoint3d>> featurePoints(m_featureDefinitions.size());
    bvector<DRange3d> extents(m_featureDefinitions.size());
    size_t featureId = 0;
    vector<int32_t> indices;
    vector<size_t> sentinels(m_featureDefinitions.size());
    DPoint3d SENTINEL_PT = DPoint3d::From(DBL_MAX, DBL_MAX, DBL_MAX);
    if (m_featureDefinitions.size() == 0) return;
    DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                        ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent));

    for (auto& feature : m_featureDefinitions)
        {
        --s_featuresAddedToTree;
        for (size_t pt = 1; pt < feature.size(); ++pt)
            {
            if (feature[pt] < INT_MAX)
                {
                POINT featurePt = this->operator[](feature[pt]);
                featurePoints[featureId].push_back(DPoint3d::From(PointOp<POINT>::GetX(featurePt), PointOp<POINT>::GetY(featurePt), PointOp<POINT>::GetZ(featurePt)));
                
                if (!nodeRange.IsContained(featurePoints[featureId].back())) ++sentinels[featureId];
                if (featurePoints[featureId].size() == 1) extents[featureId] = DRange3d::From(featurePoints[featureId].back());
                else extents[featureId].Extend(featurePoints[featureId].back());
                indices.push_back(feature[pt]);
                }
            else
                {
                featurePoints[featureId].push_back(SENTINEL_PT);
                ++sentinels[featureId];
                }
            }
        ++featureId;
        }
    for (auto& index : indices)
        {
        this->erase(index);
        for (auto& pair : m_nodeHeader.m_3dPointsDescBins)
            {
            if (pair.m_startIndex > index) --pair.m_startIndex;
            }
        }
    if (m_pSubNodeNoSplit != NULL && !m_pSubNodeNoSplit->IsVirtualNode())
        {
        for (size_t i = 0; i < featurePoints.size(); ++i)
            dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pSubNodeNoSplit)->AddFeatureDefinitionUnconditional((IDTMFile::FeatureType)m_featureDefinitions[i][0], featurePoints[i], extents[i]);
        }
    else if (!IsLeaf())
        {
        for (size_t i = 0; i < featurePoints.size(); ++i)
            {
            size_t added = 0;
            if (featurePoints[i].size() <= 1) continue;
            for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
                added += dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_apSubNodes[indexNodes])->AddFeatureDefinition((IDTMFile::FeatureType)m_featureDefinitions[i][0], featurePoints[i], extents[i], true);

            assert(added >= featurePoints[i].size() - sentinels[i]);
            }
        }
    for (auto& vec : m_featureDefinitions)
        {
        vec.clear();
        vec.Discard();
        }
    m_featureDefinitions.clear();

    }
//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 02/16
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::ClipActionRecursive(ClipAction action, uint64_t clipId, DRange3d& extent,bool setToggledWhenIdIsOn)
    {
    if (!IsLoaded()) return;
    if (/*size() == 0 || m_nodeHeader.m_nbFaceIndexes < 3*/m_nodeHeader.m_totalCount == 0) return;
    DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                        ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent));
    if (!extent.IntersectsWith(nodeRange, 2)) return;
    switch (action)
        {
        case ClipAction::ACTION_ADD:
            AddClip(clipId, false, setToggledWhenIdIsOn);
            break;
        case ClipAction::ACTION_MODIFY:
            ModifyClip(clipId, false, setToggledWhenIdIsOn);
            break;
        case ClipAction::ACTION_DELETE:
            DeleteClip(clipId, false, setToggledWhenIdIsOn);
            break;
        }
    if (m_pSubNodeNoSplit != NULL && !m_pSubNodeNoSplit->IsVirtualNode())
        {
        dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pSubNodeNoSplit)->ClipActionRecursive(action, clipId, extent, setToggledWhenIdIsOn);
        }
    else if (!IsLeaf())
        {
        for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
            {
            if (m_apSubNodes[indexNodes] != nullptr)
                dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_apSubNodes[indexNodes])->ClipActionRecursive(action, clipId, extent, setToggledWhenIdIsOn);
            }
        }
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 10/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::AddClipDefinitionRecursive(bvector<DPoint3d>& points, DRange3d& extent)
    {
    uint64_t id = -1;
    if (bsiGeom_getXYPolygonArea(&points[0], (int)points.size()) < 0) //need to flip polygon so it's counterclockwise
        {
        DPoint3d* flippedPts = new DPoint3d[points.size()];
        for (size_t pt = 0; pt < points.size(); ++pt) flippedPts[pt] = points[points.size() - 1 - pt];
        id = GetClipRegistry()->AddClip(flippedPts, points.size()) + 1;
        delete[] flippedPts;
        }
    else id = GetClipRegistry()->AddClip(&points[0], points.size()) + 1;
    bool wasClipAdded = AddClip(id, false);
    if (!wasClipAdded) return;
    if (m_pSubNodeNoSplit != NULL && !m_pSubNodeNoSplit->IsVirtualNode())
        {
        dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pSubNodeNoSplit)->AddClipDefinitionRecursive(points,extent);
        }
    else if (!IsLeaf())
        {
        for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
            {
            if(m_apSubNodes[indexNodes] != nullptr)
            dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_apSubNodes[indexNodes])->AddClipDefinitionRecursive(points,extent);
            }
        }
    }


//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 10/15
//=======================================================================================
template<class POINT, class EXTENT>
void SMMeshIndexNode<POINT, EXTENT>::SplitMeshForChildNodes()
    {
    DRange3d contentRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_contentExtent),
                                        ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_contentExtent));
    IScalableMeshMeshPtr meshPtr = nullptr;
    for (auto& nodeP : m_apSubNodes)
        {
        bvector<DPoint3d> pts(this->size());
        vector<POINT> nodePts(this->size());
        for (size_t pointInd = 0; pointInd < this->size(); pointInd++)
            {
            pts[pointInd].x =this->operator[](pointInd).x;
            pts[pointInd].y = this->operator[](pointInd).y;
            pts[pointInd].z = this->operator[](pointInd).z;
            nodePts[pointInd] = this->operator[](pointInd);
            }
        if (GetNbPtsIndices(0) <= 3) continue;
        IScalableMeshMeshPtr meshPtr = IScalableMeshMesh::Create(this->size(), &pts[0], GetNbPtsIndices(0), GetPtsIndicePtr(0), 0, 0, 0, 0, 0, 0);
        ScalableMeshMesh* meshP = (ScalableMeshMesh*)meshPtr.get();
        vector<int32_t> childIndices;
        DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(nodeP->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(nodeP->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(nodeP->m_nodeHeader.m_nodeExtent),
                                            ExtentOp<EXTENT>::GetXMax(nodeP->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(nodeP->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(nodeP->m_nodeHeader.m_nodeExtent));

        
        ClipMeshToNodeRange<POINT, EXTENT>(childIndices, nodePts, pts, nodeP->m_nodeHeader.m_contentExtent, nodeRange, meshP);
        if (childIndices.size() == 0) continue;
        nodeP->m_nodeHeader.m_nbFaceIndexes = childIndices.size();
        DRange3d childContentRange;
        childContentRange.IntersectionOf(contentRange, nodeRange);
        nodeP->m_nodeHeader.m_contentExtent = ExtentOp<EXTENT>::Create(childContentRange.low.x, childContentRange.low.y, childContentRange.low.z, childContentRange.high.x, childContentRange.high.y, childContentRange.high.z);
        nodeP->m_nodeHeader.m_contentExtentDefined = true;
        dynamic_pcast<SMMeshIndexNode<POINT,EXTENT>,SMPointIndexNode<POINT,EXTENT>>(nodeP)->PushPtsIndices(0, &childIndices[0], childIndices.size());
        dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(nodeP)->SetPtsIndiceDirty(0);
        dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(nodeP)->StorePtsIndice(0);
        nodeP->push_back(&nodePts[0], nodePts.size());
        nodeP->m_nodeHeader.m_totalCount = nodeP->size();
        nodeP->SetDirty(true);
        meshPtr = nullptr;
        }
    }


extern size_t s_nCreatedNodes;
//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 12/15
//=======================================================================================
template<class POINT, class EXTENT>
void SMMeshIndexNode<POINT, EXTENT>::UpdateFromGraph(MTGGraph * graph, bvector<DPoint3d>& pointList)
    {
    std::vector<int> faceIndices;
    MTGMask visitedMask = graph->GrabMask();
    bvector<DPoint3d> retainedPts;
    bvector<int> indices(pointList.size(), -1);
    MTGARRAY_SET_LOOP(edgeID, graph)
        {
        if (!graph->GetMaskAt(edgeID, visitedMask))
            {
            if ( FastCountNodesAroundFace(graph, edgeID) != 3)
                {
                int vIndex = -1;
                graph->TryGetLabel(edgeID, 0, vIndex);
                assert(vIndex > 0);
                assert(vIndex <= (int)pointList.size());
                if (indices[vIndex - 1] == -1)
                    {
                    retainedPts.push_back(pointList[vIndex - 1]);
                    indices[vIndex - 1] = (int)retainedPts.size();
                    }
                int idx = indices[vIndex - 1];
                graph->TrySetLabel(edgeID, 0, idx);
                graph->SetMaskAt(edgeID, MTG_EXTERIOR_MASK);
                graph->SetMaskAt(graph->EdgeMate(edgeID), MTG_BOUNDARY_MASK);
                continue;
                }
            MTGARRAY_FACE_LOOP(faceID, graph, edgeID)
                {
                int vIndex = -1;
                graph->TryGetLabel(faceID, 0, vIndex);
                assert(vIndex > 0);
                assert(vIndex <= (int)pointList.size());
                if (indices[vIndex - 1] == -1)
                    {
                    retainedPts.push_back(pointList[vIndex - 1]);
                    indices[vIndex - 1] = (int)retainedPts.size();
                    }
                int idx = indices[vIndex - 1];
                    faceIndices.push_back(idx);
                    graph->SetMaskAt(faceID, visitedMask);
                    if (graph->GetMaskAt(faceID, MTG_EXTERIOR_MASK)) graph->ClearMaskAt(faceID, MTG_EXTERIOR_MASK);
                    if (graph->GetMaskAt(faceID, MTG_BOUNDARY_MASK)) graph->ClearMaskAt(faceID, MTG_BOUNDARY_MASK);
                    graph->TrySetLabel(faceID, 0, idx);
                }
            MTGARRAY_END_FACE_LOOP(faceID, graph, edgeID)
            }
        }
    MTGARRAY_END_SET_LOOP(edgeID, graph)
        graph->ClearMask(visitedMask);
    graph->DropMask(visitedMask);
    if (NULL == this->GetGraphPtr()) this->LoadGraph();
    if (NULL == this->GetGraphPtr()) this->CreateGraph();
    *this->GetGraphPtr() = *graph;
    this->SetGraphDirty();
    this->m_nodeHeader.m_nbFaceIndexes = faceIndices.size();
    assert(faceIndices.size() % 3 == 0);
    if (faceIndices.size() > 0 && retainedPts.size() > 0)
        {
        this->clear();
        for (size_t i = 0; i < retainedPts.size(); ++i)
            this->push_back(PointOp<POINT>::Create(retainedPts[i].x, retainedPts[i].y, retainedPts[i].z));
        this->m_nodeHeader.m_nodeCount = retainedPts.size();
        this->ReplacePtsIndices(0, (int32_t*)&faceIndices[0], this->m_nodeHeader.m_nbFaceIndexes);
        this->SetPtsIndiceDirty(0);
        }
#if SM_OUTPUT_MESHES_GRAPH
    WString nameBefore = L"e:\\output\\scmesh\\2015-12-11\\afterfilter_";
    nameBefore.append(std::to_wstring(this->m_nodeHeader.m_level).c_str());
    nameBefore.append(L"_");
    nameBefore.append(std::to_wstring(ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_nodeExtent)).c_str());
    nameBefore.append(L"_");
    nameBefore.append(std::to_wstring(ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_nodeExtent)).c_str());
    nameBefore.append(L".m");
    size_t nVertices = retainedPts.size();
    size_t nIndices = this->m_nodeHeader.m_nbFaceIndexes;
    FILE* meshBeforeStitch = _wfopen(nameBefore.c_str(), L"wb");
    fwrite(&nVertices, sizeof(size_t), 1, meshBeforeStitch);
    fwrite(&retainedPts[0], sizeof(DPoint3d), nVertices, meshBeforeStitch);
    fwrite(&nIndices, sizeof(size_t), 1, meshBeforeStitch);
    fwrite((int32_t*)this->GetPtsIndicePtr(0), sizeof(int32_t), nIndices, meshBeforeStitch);
    fclose(meshBeforeStitch);
#endif
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 10/15
//=======================================================================================
template<class POINT, class EXTENT>
void SMMeshIndexNode<POINT, EXTENT>::SplitNodeBasedOnImageRes()
    {
    HPRECONDITION(IsLeaf());
    POINT splitPosition = GetDefaultSplitPosition();
    if (m_nodeHeader.m_numberOfSubNodesOnSplit == 4)
        {

        m_apSubNodes[0] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetX(splitPosition),
            ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

        m_apSubNodes[1] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

        m_apSubNodes[2] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetX(splitPosition),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

        m_apSubNodes[3] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
            ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));
        s_nCreatedNodes += 4;

        }
    else
        {
        HPRECONDITION(ExtentOp<EXTENT>::GetThickness(GetNodeExtent()) > 0.0);

        if (HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetZ(splitPosition),
            HNumeric<double>::EPSILON_MULTIPLICATOR() * ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent)))
            {
            // Values would be virtually equal ... we will not split
            HDEBUGCODE(m_unspliteable = true;)
                return;
            }


        m_apSubNodes[0] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetX(splitPosition),
            ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetZ(splitPosition)));

        m_apSubNodes[1] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetZ(splitPosition)));

        m_apSubNodes[2] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetX(splitPosition),
            PointOp<POINT>::GetY(splitPosition),
            PointOp<POINT>::GetZ(splitPosition)));

        m_apSubNodes[3] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
            ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetY(splitPosition),
            PointOp<POINT>::GetZ(splitPosition)));

        m_apSubNodes[4] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetY(splitPosition),
            PointOp<POINT>::GetZ(splitPosition),
            PointOp<POINT>::GetX(splitPosition),
            ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

        m_apSubNodes[5] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
            PointOp<POINT>::GetY(splitPosition),
            PointOp<POINT>::GetZ(splitPosition),
            ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

        m_apSubNodes[6] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetZ(splitPosition),
            PointOp<POINT>::GetX(splitPosition),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

        m_apSubNodes[7] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
            ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetZ(splitPosition),
            ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));
        s_nCreatedNodes += 8;
        }
    // Indicate node is not a leaf anymore
    m_nodeHeader.m_IsLeaf = false;
    m_nodeHeader.m_IsBranched = true;

    SetupNeighborNodesAfterSplit();

   SplitMeshForChildNodes();
    SetDirty(true);
    }

//=======================================================================================
// @description Sets texture data for this node based on a raster. This *adds* a texture
//              to the targeted node, this does not replace the existing texture.
//              See ScalableMeshSourceCreator::ImportRasterSourcesTo for information
//              on how to create a raster from image files to be used by this function.
// @bsimethod                                                   Elenie.Godzaridis 10/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::TextureFromRaster(HIMMosaic* sourceRasterP)
    {
    if (!IsLoaded()) Load();
    DRange2d rasterBox = DRange2d::From(sourceRasterP->GetEffectiveShape()->GetExtent().GetXMin(), sourceRasterP->GetEffectiveShape()->GetExtent().GetYMin(),
                                        sourceRasterP->GetEffectiveShape()->GetExtent().GetXMax(), sourceRasterP->GetEffectiveShape()->GetExtent().GetYMax());
    //get overlap between node and raster extent
    DRange2d contentExtent = DRange2d::From(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
                                            ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent));
    if (!rasterBox.IntersectsWith(contentExtent)) return;
    m_nodeHeader.m_areTextured = true;
   size_t texId = 0; //idx0 is reserved for eventual untextured part
    int textureWidthInPixels = 1024, textureHeightInPixels = 1024;
    double unitsPerPixelX = (contentExtent.high.x - contentExtent.low.x) / textureWidthInPixels;
    double unitsPerPixelY = (contentExtent.high.y - contentExtent.low.y) / textureHeightInPixels;
    contentExtent.low.x -= 5 * unitsPerPixelX;
    contentExtent.low.y -= 5 * unitsPerPixelY;
    contentExtent.high.x += 5 * unitsPerPixelX;
    contentExtent.high.y += 5 * unitsPerPixelY;
    HPRECONDITION((textureWidthInPixels != 0) && (textureHeightInPixels != 0));

    HFCMatrix<3, 3> transfoMatrix;
    transfoMatrix[0][0] = (contentExtent.high.x - contentExtent.low.x) / textureWidthInPixels;
    transfoMatrix[0][1] = 0;
    transfoMatrix[0][2] = contentExtent.low.x;
    transfoMatrix[1][0] = 0;
    transfoMatrix[1][1] = -(contentExtent.high.y - contentExtent.low.y) / textureHeightInPixels;
    transfoMatrix[1][2] = contentExtent.high.y;
    transfoMatrix[2][0] = 0;
    transfoMatrix[2][1] = 0;
    transfoMatrix[2][2] = 1;

    HFCPtr<HGF2DTransfoModel> pTransfoModel((HGF2DTransfoModel*)new HGF2DProjective(transfoMatrix));

    HFCPtr<HGF2DTransfoModel> pSimplifiedModel = pTransfoModel->CreateSimplifiedModel();

    if (pSimplifiedModel != 0)
        {
        pTransfoModel = pSimplifiedModel;
        }

    HFCPtr<HRABitmap> pTextureBitmap;

    HFCPtr<HRPPixelType> pPixelType(new HRPPixelTypeV32R8G8B8A8());

    HFCPtr<HCDCodec>     pCodec(new HCDCodecIdentity());
    byte* pixelBufferP = new byte[textureWidthInPixels * textureHeightInPixels * 3 + 3 * sizeof(int)];
    memcpy(pixelBufferP, &textureWidthInPixels, sizeof(int));
    memcpy(pixelBufferP + sizeof(int), &textureHeightInPixels, sizeof(int));
    int nOfChannels = 3;
    memcpy(pixelBufferP + 2 * sizeof(int), &nOfChannels, sizeof(int));

    pTextureBitmap = HRABitmap::Create(textureWidthInPixels,
                                   textureHeightInPixels,
                                   pTransfoModel.GetPtr(),
                                   sourceRasterP->GetCoordSys(),
                                   pPixelType,
                                   8);
    HGF2DExtent minExt, maxExt;
    sourceRasterP->GetPixelSizeRange(minExt, maxExt);
    minExt.ChangeCoordSys(pTextureBitmap->GetCoordSys());
    if (/*m_nodeHeader.m_level <= 6 && */IsLeaf() && (contentExtent.XLength() / minExt.GetWidth() > textureWidthInPixels || contentExtent.YLength() / minExt.GetHeight() > textureHeightInPixels) /*&& this->size() > 0*/)
        SplitNodeBasedOnImageRes();
    byte* pixelBufferPRGBA = new byte[textureWidthInPixels * textureHeightInPixels * 4];
    pTextureBitmap->GetPacket()->SetBuffer(pixelBufferPRGBA, textureWidthInPixels * textureHeightInPixels * 4);
    pTextureBitmap->GetPacket()->SetBufferOwnership(false);

    HRAClearOptions clearOptions;

    //CR 332863 - Quick trick to display the STM outside in smooth outside the area where texture data are available. 
    //              Note that this trick will lead to the translucent color shown throughout transparent raster being a shade of gray 
    //              instead of the color of the background.
    uint32_t whiteOpaque = 0xFFFFFFFF;

    clearOptions.SetRawDataValue(&whiteOpaque);

    pTextureBitmap->Clear(clearOptions);

    HRACopyFromOptions copyFromOptions;


    copyFromOptions.SetAlphaBlend(true);

    pTextureBitmap->CopyFrom(*sourceRasterP, copyFromOptions);
#ifdef ACTIVATE_TEXTURE_DUMP
    WString fileName = L"file://";
    fileName.append(L"e:\\output\\scmesh\\2015-11-19\\texture_before_");
    fileName.append(std::to_wstring(m_nodeHeader.m_level).c_str());
    fileName.append(L"_");
    fileName.append(std::to_wstring(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent)).c_str());
    fileName.append(L"_");
    fileName.append(std::to_wstring(ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent)).c_str());
    fileName.append(L".bmp");
    HFCPtr<HFCURL> fileUrl(HFCURL::Instanciate(fileName));
    HFCPtr<HRPPixelType> pImageDataPixelType(new HRPPixelTypeV24B8G8R8());
    byte* pixelBuffer = new byte[1024*1024*3];
    size_t t = 0;
    for (size_t i = 0; i < 1024 * 1024 * 4; i += 4)
        {
        pixelBuffer[t] = *(pixelBufferP + 3 * sizeof(int) + i);
        pixelBuffer[t + 1] = *(pixelBufferP + 3 * sizeof(int) + i + 1);
        pixelBuffer[t + 2] = *(pixelBufferP + 3 * sizeof(int) + i + 2);
        t += 3;
        }
    HRFBmpCreator::CreateBmpFileFromImageData(fileUrl,
                                              1024,
                                              1024,
                                              pImageDataPixelType,
                                              pixelBuffer);
    delete[] pixelBuffer;

    auto codec = new HCDCodecIJG(1024, 1024, 8 * 4);
    codec->SetSourceColorMode(HCDCodecIJG::ColorModes::RGB);
    codec->SetQuality(100);
    codec->SetSubsamplingMode(HCDCodecIJG::SubsamplingModes::SNONE);
    HFCPtr<HCDCodec> pCodec2 = codec;
    size_t compressedBufferSize = pCodec2->GetSubsetMaxCompressedSize();
    byte* pCompressedPixelBuffer = new byte[compressedBufferSize];
    size_t nCompressed = pCodec2->CompressSubset(pixelBufferP + 3 * sizeof(int), 1024 * 1024 * 4 * sizeof(Byte), pCompressedPixelBuffer, compressedBufferSize * sizeof(Byte));
    byte * pUncompressedPixelBuffer = new byte[1024*1024*4];
    pCodec2->DecompressSubset(pCompressedPixelBuffer, compressedBufferSize* sizeof(Byte), pUncompressedPixelBuffer, 1024 * 1024 * 4 * sizeof(Byte));
    /*WString fileName2;
    fileName2.append(L"e:\\output\\scmesh\\2015-11-19\\texture_compressed_");
    fileName2.append(std::to_wstring(m_nodeHeader.m_level).c_str());
    fileName2.append(L"_");
    fileName2.append(std::to_wstring(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent)).c_str());
    fileName2.append(L"_");
    fileName2.append(std::to_wstring(ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent)).c_str());
    fileName2.append(L".bin");
    FILE* binCompressed = _wfopen(fileName2.c_str(), L"wb");
    fwrite(pCompressedPixelBuffer,sizeof(byte),nCompressed, binCompressed);
    fclose(binCompressed);*/
    std::string myS = " BUFFER SIZE "+std::to_string(compressedBufferSize) +" DATA SIZE "+ std::to_string(nCompressed);
    delete[] pCompressedPixelBuffer;
    WString fileName2 = L"file://";
    fileName2.append(L"e:\\output\\scmesh\\2015-11-19\\texture_before_compressed_");
    fileName2.append(std::to_wstring(m_nodeHeader.m_level).c_str());
    fileName2.append(L"_");
    fileName2.append(std::to_wstring(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent)).c_str());
    fileName2.append(L"_");
    fileName2.append(std::to_wstring(ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent)).c_str());
    fileName2.append(L".bmp");
    HFCPtr<HFCURL> fileUrl2(HFCURL::Instanciate(fileName2));
    pixelBuffer = new byte[1024 * 1024 * 3];
    t = 0;
    for (size_t i = 0; i < 1024 * 1024 * 4; i += 4)
        {
        pixelBuffer[t] = *(pUncompressedPixelBuffer + i);
        pixelBuffer[t + 1] = *(pUncompressedPixelBuffer + i + 1);
        pixelBuffer[t + 2] = *(pUncompressedPixelBuffer + i + 2);
        t += 3;
        }
    delete[] pUncompressedPixelBuffer;
    HRFBmpCreator::CreateBmpFileFromImageData(fileUrl2,
                                              1024,
                                              1024,
                                              pImageDataPixelType,
                                              pixelBuffer);
    delete[] pixelBuffer;
#endif
    for (size_t i = 0; i < textureWidthInPixels*textureHeightInPixels; ++i)
        {
        *(pixelBufferP + 3 * sizeof(int) + i * 3) = pixelBufferPRGBA[i * 4];
        *(pixelBufferP + 3 * sizeof(int) + i * 3 + 1) = pixelBufferPRGBA[i * 4 + 1];
        *(pixelBufferP + 3 * sizeof(int) + i * 3 + 2) = pixelBufferPRGBA[i * 4 + 2];
        }
    PushTexture(texId, pixelBufferP + 3 * sizeof(int), textureWidthInPixels * textureHeightInPixels * 3);
     SetTextureDirty(texId);
    StoreTexture(texId);
    if (GetNbPtsIndices(0) >= 4)
        {
        //compute uv's
       vector<DPoint3d> points(this->size());
        for (size_t i = 0; i < this->size(); ++i)
            points[i] = DPoint3d::From(PointOp<POINT>::GetX(operator[](i)), PointOp<POINT>::GetY(operator[](i)), PointOp<POINT>::GetZ(operator[](i)));
        vector<int32_t> indicesOfTexturedRegion;
        vector<DPoint2d> uvsOfTexturedRegion(points.size());
    int32_t* existingFaces = GetPtsIndicePtr(0);
    for (size_t i = 0; i < GetNbPtsIndices(0); i+=3)
            {
            DPoint3d face[3];
            int32_t idx[3] = { existingFaces[i], existingFaces[i + 1], existingFaces[i + 2] };
            DPoint2d uvs[3];
            for (size_t i = 0; i < 3; ++i)
                {
                face[i] = points[idx[i] - 1];
                uvs[i].x = max(0.0,min((face[i].x - contentExtent.low.x) / (contentExtent.XLength()),1.0));
                uvs[i].y = max(0.0, min((face[i].y - contentExtent.low.y) / (contentExtent.YLength()), 1.0));
                //if (uvs[i].x == 0.0) uvs[i].x = 0.004;
                //if (uvs[i].y == 0.0) uvs[i].y = 0.004;
                }
            indicesOfTexturedRegion.push_back(idx[0]);
            indicesOfTexturedRegion.push_back(idx[1]);
            indicesOfTexturedRegion.push_back(idx[2]);
            uvsOfTexturedRegion[idx[0] - 1] = uvs[0];
            uvsOfTexturedRegion[idx[1] - 1] = uvs[1];
            uvsOfTexturedRegion[idx[2] - 1] = uvs[2];
            }

    ClearPtsIndices(0);
    SetPtsIndiceDirty(0);
    PushPtsIndices(texId + 1, &indicesOfTexturedRegion[0], indicesOfTexturedRegion.size());
    PushUV(/*texId + 1,*/ &uvsOfTexturedRegion[0], uvsOfTexturedRegion.size());
    PushUVsIndices(texId, &indicesOfTexturedRegion[0], indicesOfTexturedRegion.size());
    SetUVDirty(/*texId + 1*/);
    SetUVsIndicesDirty(texId);
    SetPtsIndiceDirty(texId + 1);
        StoreUV();
    StoreUVsIndices(texId);
    StorePtsIndice(texId+1);
#if DEBUG && SM_TRACE_RASTER_TEXTURING 
        std::string s;
    s += " N OF INDICE ARRAYS " + std::to_string(GetNbPtsIndiceArrays());
    for (size_t i = 0; i <GetNbPtsIndiceArrays(); ++i)
            {
        s += " ARRAY " + std::to_string(i) + " HAS " + std::to_string(GetNbPtsIndices(i)) + " INDICES ";
        }
#endif
    }
    delete[] pixelBufferP;
    delete[] pixelBufferPRGBA;
    pTextureBitmap = 0;
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 10/15
//=======================================================================================
    template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::TextureFromRasterRecursive(HIMMosaic* sourceRasterP)
    {
    TextureFromRaster(sourceRasterP);
    if (m_pSubNodeNoSplit != NULL && !m_pSubNodeNoSplit->IsVirtualNode())
        {
        dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pSubNodeNoSplit)->TextureFromRasterRecursive(sourceRasterP);
        }
    else if (!IsLeaf())
        {
        for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
            {
            if (m_apSubNodes[indexNodes] != nullptr)
                {
                auto mesh = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_apSubNodes[indexNodes]);
                assert(mesh != nullptr);
                dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_apSubNodes[indexNodes])->TextureFromRasterRecursive(sourceRasterP);
                }
            }
        }
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 10/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::RefreshMergedClipsRecursive()
    {
    BuildSkirts();
    ComputeMergedClips();
    if (m_pSubNodeNoSplit != NULL && !m_pSubNodeNoSplit->IsVirtualNode())
        {
        dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pSubNodeNoSplit)->RefreshMergedClipsRecursive();
        }
    else if (!IsLeaf())
        {
        for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
            {
            if(m_apSubNodes[indexNodes] != nullptr)
                dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_apSubNodes[indexNodes])->RefreshMergedClipsRecursive();
            }
        }
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 02/16
//=======================================================================================
template<class POINT, class EXTENT>  bool SMMeshIndexNode<POINT, EXTENT>::HasClip(uint64_t clipId)
    {
    for (auto& diffSet : m_differenceSets)
        {
        if (diffSet.clientID == clipId && (!diffSet.upToDate || !diffSet.IsEmpty() || diffSet.clientID == (uint64_t)-1)) return true;
        }
    return false;
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 10/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::ComputeMergedClips()
    {
#ifdef USE_DIFFSET
    for (auto& diffSet : m_differenceSets)
        {
        if (diffSet.clientID == (uint64_t)-1 && diffSet.upToDate) return;
        }
    bvector<bvector<DPoint3d>> clips;
    for (auto& diffSet : m_differenceSets)
        {
        uint64_t upperId = (diffSet.clientID >> 32);
        if (upperId == 0)
            {
            clips.push_back(bvector<DPoint3d>());
            GetClipRegistry()->GetClip(diffSet.clientID - 1, clips.back());
            }
        }
    if (clips.size() == 0) return;
    vector<DPoint3d> points(size());

    std::transform(begin(), end(), &points[0], PtToPtConverter());
    DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                        ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent));

    Clipper clipNode(&points[0], size(), (int32_t*)&this->operator[](this->size()), m_nodeHeader.m_nbFaceIndexes, nodeRange);
    DifferenceSet allClips = clipNode.ClipSeveralPolygons(clips);
    allClips.clientID = (uint64_t)-1;
    bool added = false;
    for (auto& diffSet : m_differenceSets)
        if (diffSet.clientID == (uint64_t)-1) { diffSet = allClips; added = true; }
    if (!added)
        {
        m_differenceSets.push_back(allClips);
        (m_differenceSets.begin() + (m_differenceSets.size() - 1))->upToDate = true;
        m_nbClips++;
        }
#else
    {
    if (m_differenceSets.size() == 0) return;
    for (auto& diffSet : m_differenceSets)
        {
        if (diffSet.clientID == (uint64_t)-1 && diffSet.upToDate) return;
        }
    vector<DPoint3d> points(size());

    std::transform(begin(), end(), &points[0], PtToPtConverter());
    DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                        ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent));


    bvector<bvector<DPoint3d>> polys;
    bvector<uint64_t> clipIds;
    bvector<DifferenceSet> skirts;
    bvector<bpair<double, int>> metadata;
    for (auto& diffSet : m_differenceSets)
        {
        //uint64_t upperId = (diffSet.clientID >> 32);
        if (diffSet.clientID < ((uint64_t)-1) && diffSet.clientID != 0 && diffSet.toggledForID)
            {
            clipIds.push_back(diffSet.clientID);
            polys.push_back(bvector<DPoint3d>());
            GetClipRegistry()->GetClip(diffSet.clientID, polys.back());
            double importance;
            int nDimensions;
            GetClipRegistry()->GetClipMetadata(diffSet.clientID, importance, nDimensions);
            metadata.push_back(make_bpair(importance, nDimensions));
            }
        else if (!diffSet.toggledForID)
            {
            skirts.push_back(diffSet);
            }
             
        }
    m_differenceSets.clear();
    for(auto& skirt: skirts) m_differenceSets.push_back(skirt);
    m_nbClips = skirts.size();
    for (size_t j = 0; j < GetNbPtsIndiceArrays(); ++j)
        {
        if (GetNbPtsIndices(j) == 0)
            {
            DifferenceSet current;
            current.clientID = 0;
            m_differenceSets.push_back(current);
            (m_differenceSets.begin() + (m_differenceSets.size() - 1))->upToDate = true;
            ++m_nbClips;
            continue;
            }

        Pin();    
        PinPtsIndices(j);
        PinUV();

        PinUVsIndices(j-1);

        Clipper clipNode(&points[0], size(), (int32_t*)GetPtsIndicePtr(j), GetNbPtsIndices(j), nodeRange, GetUVPtr(), GetUVsIndicesPtr(j-1));
        bvector<bvector<PolyfaceHeaderPtr>> polyfaces;
        auto nodePtr = HFCPtr<SMPointIndexNode<POINT, EXTENT>>(static_cast<SMPointIndexNode<POINT, EXTENT>*>(const_cast<SMMeshIndexNode<POINT, EXTENT>*>(this)));
        IScalableMeshNodePtr nodeP(new ScalableMeshNode<POINT>(nodePtr));
        BcDTMPtr dtm = nodeP->GetBcDTM().get();
        bool hasClip = false;
        if (dtm.get() != nullptr)
            {
            BcDTMPtr toClipBcDTM = dtm->Clone();
            DTMPtr toClipDTM = toClipBcDTM.get();
            if (toClipBcDTM->GetTinHandle() != nullptr) hasClip = clipNode.GetRegionsFromClipPolys(polyfaces, polys, metadata, toClipDTM);
            }
       // m_differenceSets.clear();
       // m_nbClips = 0;
        UnPin();    
        UnPinPtsIndices(j);
        UnPinUV();
        UnPinUVsIndices(j-1);

        if (!hasClip) continue;
        bvector<bvector<PolyfaceHeaderPtr>> skirts;
        //BuildSkirtMeshesForPolygonSet(skirts, polyfaces, polys, nodeRange);
        map<DPoint3d, int32_t, DPoint3dZYXTolerancedSortComparison> mapOfPoints(DPoint3dZYXTolerancedSortComparison(1e-5, 0));
        for (size_t i = 0; i < size(); ++i)
            mapOfPoints[points[i]] = (int)i;
        for (auto& polyface : polyfaces)
            {
            DifferenceSet current = DifferenceSet::FromPolyfaceSet(polyface, mapOfPoints, this->size() + 1);
            for (auto& poly : polyface) poly = nullptr;
            if (&polyface - &polyfaces[0] == 0) current.clientID = 0;
            else current.clientID = clipIds[(&polyface - &polyfaces[0]) - 1];
            m_differenceSets.push_back(current);
            (m_differenceSets.begin() + (m_differenceSets.size() - 1))->upToDate = true;
            ++m_nbClips;
           /* if (current.clientID != 0)
                {
                DifferenceSet skirt = DifferenceSet::FromPolyfaceSet(skirts[&polyface - &polyfaces[0]], mapOfPoints);
                skirt.clientID = current.clientID;
                skirt.toggledForID = false;
                m_differenceSets.push_back(skirt);
                (m_differenceSets.begin() + (m_differenceSets.size() - 1))->upToDate = true;
                ++m_nbClips;
                }*/
            }
        }
        }

    DifferenceSet allClips;
    allClips.clientID = (uint64_t)-1;
    bool added = false;
    for (auto& diffSet : m_differenceSets)
        if (diffSet.clientID == (uint64_t)-1) { diffSet = allClips; diffSet.upToDate = true; added = true; }
    if (!added)
        {
        m_differenceSets.push_back(allClips);
        (m_differenceSets.begin() + (m_differenceSets.size() - 1))->upToDate = true;
        m_nbClips++;
        }
#endif
    }

template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::BuildSkirts()
    {
        if (m_differenceSets.size() == 0) return;
        for (auto& diffSet : m_differenceSets)
            {
            if (diffSet.clientID == (uint64_t)-1 && diffSet.upToDate) return;
            }

        DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                            ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent));


        auto nodePtr = HFCPtr<SMPointIndexNode<POINT, EXTENT>>(static_cast<SMPointIndexNode<POINT, EXTENT>*>(const_cast<SMMeshIndexNode<POINT, EXTENT>*>(this)));
        IScalableMeshNodePtr nodeP(new ScalableMeshNode<POINT>(nodePtr));
        auto dtm = nodeP->GetBcDTM();
        if (dtm.get() == nullptr) return;
            SkirtBuilder builder(dtm);
            map<DPoint3d, int32_t, DPoint3dZYXTolerancedSortComparison> mapOfPoints(DPoint3dZYXTolerancedSortComparison(1e-5, 0));
            for (size_t i = 0; i < size(); ++i)
                mapOfPoints[this->operator[](i)] = (int)i;

        for (auto& diffSet : m_differenceSets)
            {
            if (diffSet.clientID < ((uint64_t)-1) && diffSet.clientID != 0 && !diffSet.toggledForID)
                {
                bvector<bvector<DPoint3d>> skirts;
                GetClipRegistry()->GetSkirt(diffSet.clientID, skirts);
                bvector<PolyfaceHeaderPtr> polyfaces;
                builder.BuildSkirtMesh(polyfaces, skirts);
                DifferenceSet current = DifferenceSet::FromPolyfaceSet(polyfaces, mapOfPoints,this->size() + 1);
                current.clientID = diffSet.clientID;
                current.toggledForID = false;
                diffSet = current;
                }
            }

    }


//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 09/15
//=======================================================================================
    template<class POINT, class EXTENT>  bool SMMeshIndexNode<POINT, EXTENT>::AddClip(uint64_t clipId, bool isVisible, bool setToggledWhenIdIsOn)
    {
    if (size() == 0 || m_nodeHeader.m_nbFaceIndexes < 3) return false;
#ifdef USE_DIFFSET
    bvector<DPoint3d> clipPts;
    GetClipRegistry()->GetClip(clipId - 1, clipPts);
    if (clipPts.size() == 0 || size() == 0 || m_nodeHeader.m_nbFaceIndexes < 3) return false;
    DRange3d clipExt = DRange3d::From(&clipPts[0], (int32_t)clipPts.size());
    DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                        ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent));
    if (clipExt.IntersectsWith(nodeRange,2) && GridBasedIntersect(clipPts, clipExt,nodeRange))
        
#endif
        {
        bool clipFound = false;
        for (auto& diffSet : m_differenceSets) if (diffSet.clientID == clipId && diffSet.toggledForID == setToggledWhenIdIsOn) clipFound = true;
        if (clipFound) return true; //clip already added
#ifdef USE_DIFFSET
        vector<DPoint3d> points(size());

        std::transform(begin(), end(), &points[0], PtToPtConverter());
        Clipper clipNode(&points[0], size(), (int32_t*)GetPtsIndicePtr(0), m_nodeHeader.m_nbFaceIndexes, nodeRange);
#endif
        DifferenceSet d;
#if DEBUG && SM_TRACE_CLIPS
        std::string s;
        s += " AREA IS" + std::to_string(bsiGeom_getXYPolygonArea(&clipPts[0], (int)clipPts.size()));
#endif
        //if (nodeRange.XLength() <= clipExt.XLength() * 10000 && nodeRange.YLength() <= clipExt.YLength() * 10000)
#ifdef USE_DIFFSET
            d = clipNode.ClipNonConvexPolygon2D(&clipPts[0], clipPts.size());
#else
            {
            d.clientID = clipId;
            d.firstIndex = (int32_t)size() + 1;
            d.toggledForID = setToggledWhenIdIsOn;
            m_differenceSets.push_back(d);
            m_nbClips++;
            (m_differenceSets.begin() + (m_differenceSets.size() - 1))->upToDate = false;
            for (auto& other : m_differenceSets)
                {
                if (other.clientID == ((uint64_t)-1)) other.upToDate = false;
                }
            
            }

#endif
        bool emptyClip = false;
       // if (d.addedFaces.size() == 0 && d.removedFaces.size() == 0 && d.addedVertices.size() == 0 && d.removedVertices.size() == 0) emptyClip = true;
#ifdef USE_DIFFSET
        d.clientID = clipId;
        vector<DifferenceSet> merged;
        if (!emptyClip)
            {
            for (auto& other : m_differenceSets)
                {
                if (other.clientID == ((uint64_t)-1)) other.upToDate = false;
                }
            }
       /* if (!emptyClip)
            {
            for (auto& other : m_differenceSets)
                {
                uint64_t upperId = (other.clientID >> 32);
                if (upperId == 0 && other.clientID != d.clientID && d.ConflictsWith(other))
                    {
                    bvector<DPoint3d> otherClipPts;
                    GetClipRegistry()->GetClip(other.clientID - 1, otherClipPts);
                    if (otherClipPts.size() == 0) continue;
                    DifferenceSet mergedSet = d.MergeSetWith(other, &points[0], clipPts, otherClipPts);
                    mergedSet.clientID = other.clientID > d.clientID ? other.clientID << 32 | d.clientID : d.clientID << 32 | other.clientID;
                    merged.push_back(mergedSet);
                    }
                }
            }*/
        m_differenceSets.push_back(d);
        (m_differenceSets.begin() + (m_differenceSets.size() - 1))->upToDate = true;
        for (auto& mergeSet : merged)
            {
            m_differenceSets.push_back(mergeSet);
            (m_differenceSets.begin() + (m_differenceSets.size() - 1))->upToDate = true;
            }
        m_nbClips++;
#endif
        if (isVisible && !emptyClip)
            {
            PropagateClipUpwards(clipId, ClipAction::ACTION_ADD);
            PropagateClipToNeighbors(clipId, ClipAction::ACTION_ADD);
            }
        PropagateClip(clipId, ClipAction::ACTION_ADD);
        return !emptyClip;
        }
#ifdef USE_DIFFSET
    else return false;
#endif
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 09/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::DoClip(uint64_t clipId, bool isVisible)
    {
    for (auto& diffSet : m_differenceSets) 
        if (diffSet.clientID == clipId)
        {
        if (diffSet.upToDate) return;
        DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                            ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent));
        bvector<DPoint3d> clipPts;
        GetClipRegistry()->GetClip(clipId, clipPts);
        if (clipPts.size() == 0) return;
        vector<DPoint3d> points(size());

        std::transform(begin(), end(), &points[0], PtToPtConverter());
        Clipper clipNode(&points[0], size(), (int32_t*)GetPtsIndicePtr(0), m_nodeHeader.m_nbFaceIndexes, nodeRange);
        diffSet = clipNode.ClipNonConvexPolygon2D(&clipPts[0], clipPts.size());
        bool emptyClip = false;
        if (diffSet.addedFaces.size() == 0 && diffSet.removedFaces.size() == 0 && diffSet.addedVertices.size() == 0 && diffSet.removedVertices.size() == 0) emptyClip = true;
        diffSet.clientID = clipId;
        diffSet.upToDate = true;
        if (isVisible && !emptyClip)
            {
            PropagateClipUpwards(clipId, ClipAction::ACTION_ADD);
            PropagateClipToNeighbors(clipId, ClipAction::ACTION_ADD);
            }
        if (m_nodeHeader.m_level < 7) PropagateClip(clipId, ClipAction::ACTION_ADD);
        return;
        }
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 09/15
//=======================================================================================
template<class POINT, class EXTENT>  bool SMMeshIndexNode<POINT, EXTENT>::DeleteClip(uint64_t clipId, bool isVisible, bool setToggledWhenIdIsOn)
    {
    if (size() == 0 || m_nodeHeader.m_nbFaceIndexes < 3) return false;
    bool found = false;
    for (auto it = m_differenceSets.begin(); it != m_differenceSets.end(); ++it)
        {
        if (it->clientID == clipId && it->toggledForID == setToggledWhenIdIsOn)
            {
            m_differenceSets.erase(it);
            m_nbClips--;
            //if (m_nodeHeader.m_level < 7) PropagateDeleteClipImmediately(clipId);
            found = true;
            }
        else if (it->clientID == (uint64_t)-1)
            {
            it->upToDate = false;
            }
        }
    return found;
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 09/15
//=======================================================================================
template<class POINT, class EXTENT>  bool SMMeshIndexNode<POINT, EXTENT>::ModifyClip(uint64_t clipId, bool isVisible, bool setToggledWhenIdIsOn)
    {
    if (size() == 0 || m_nodeHeader.m_nbFaceIndexes < 3) return false;
#ifdef USE_DIFFSET
    bvector<DPoint3d> clipPts;
    GetClipRegistry()->GetClip(clipId, clipPts);
    if (clipPts.size() == 0) return false;
    DRange3d clipExt = DRange3d::From(&clipPts[0], (int32_t)clipPts.size());
    DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                        ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent));
    bool clipIntersects = clipExt.IntersectsWith(nodeRange);
#endif
    bool found = false;
    for (auto it = m_differenceSets.begin(); it != m_differenceSets.end(); ++it)
        {
        if (it->clientID == clipId && it->toggledForID == setToggledWhenIdIsOn)
            {
#ifdef USE_DIFFSET
            if (isVisible)
                {
                PropagateClipUpwards(clipId, ClipAction::ACTION_MODIFY);
                PropagateClipToNeighbors(clipId, ClipAction::ACTION_MODIFY);
                }
            if (!clipIntersects)
                {
                m_differenceSets.erase(it);
                m_nbClips--;
                PropagateDeleteClipImmediately(clipId);
                }
            else
                {
                vector<DPoint3d> points(size());

                std::transform(begin(), end(), &points[0], PtToPtConverter());
                Clipper clipNode(&points[0], size(), (int32_t*)GetPtsIndicePtr(0), m_nodeHeader.m_nbFaceIndexes, nodeRange);
                DifferenceSet d = clipNode.ClipNonConvexPolygon2D(&clipPts[0], clipPts.size());
                if (d.addedFaces.size() == 0 && d.removedFaces.size() == 0 && d.addedVertices.size() == 0 && d.removedVertices.size() == 0)
                    {
                    m_differenceSets.erase(it);
                    m_nbClips--;
                    if (m_nodeHeader.m_level < 7) PropagateDeleteClipImmediately(clipId);
                    }
                else
                    {
                    d.clientID = clipId;
                    m_differenceSets.modify(it - m_differenceSets.begin(), d);
                    if (m_nodeHeader.m_level < 7) PropagateClip(clipId, ClipAction::ACTION_MODIFY);
                    return true;
                    }

#else
            *it = DifferenceSet();
            it->clientID = clipId;
            it->toggledForID = setToggledWhenIdIsOn;
#endif
            }
#ifndef USE_DIFFSET
        else if (it->clientID == (uint64_t)-1)
            {
            it->upToDate = false;
            }
#endif
        }
    return found;
    }
//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 09/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::PropagateDeleteClipImmediately(uint64_t clipId)
    {
    if (HasRealChildren())
        {
        if (m_pSubNodeNoSplit != nullptr)
            {
            dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pSubNodeNoSplit)->DeleteClip(clipId, false);
            }
        else
            {
            for (size_t i = 0; i < GetNumberOfSubNodesOnSplit(); ++i)
                {
                if (m_apSubNodes[i] != nullptr)
                    {
                    dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_apSubNodes[i])->DeleteClip(clipId, false);
                    }
                }
            }
        }
    auto node = GetParentNode();
    while (node != nullptr && node->GetLevel() > 1)
        {
        node = node->GetParentNode();
        }
    if (node != nullptr)
        {
        dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(node)->DeleteClip(clipId, false);
        dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(node->GetParentNode())->DeleteClip(clipId, false);
        for (size_t i = 0; i < node->GetParentNode()->GetNumberOfSubNodesOnSplit(); ++i)
            {
            if (node->GetParentNode()->m_apSubNodes[i] != nullptr) dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(node->GetParentNode()->m_apSubNodes[i])->DeleteClip(clipId, false);
            }
        }

    for (size_t n = 0; n < MAX_NUM_NEIGHBORNODE_POSITIONS; ++n)
        {
        for (auto& node : m_apNeighborNodes[n])
            if (node != nullptr)
                {
                dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(node)->DeleteClip(clipId, false);
                }
        }
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 09/15
//=======================================================================================
template<class POINT, class EXTENT>  bool SMMeshIndexNode<POINT, EXTENT>::AddClipAsync(uint64_t clipId, bool isVisible)
    {
    DifferenceSet d;
    d.clientID = clipId;
    m_differenceSets.push_back(d);
    (m_differenceSets.begin() + (m_differenceSets.size() - 1))->upToDate = false;
    m_nbClips++;
    SMTask task = SMTask(std::bind(std::mem_fn(&SMMeshIndexNode<POINT, EXTENT>::DoClip), this, clipId, isVisible));
    s_schedulerLock.lock();
    if (s_clipScheduler == nullptr) s_clipScheduler = new ScalableMeshScheduler();

    s_clipScheduler->ScheduleTask(task, isVisible ? ScalableMeshScheduler::PRIORITY_HIGH : ScalableMeshScheduler::PRIORITY_DEFAULT);

    s_schedulerLock.unlock();
    return true;
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 09/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::PropagateClip(uint64_t clipId, ClipAction action)
    {
    std::vector<SMTask> createdTasks;

    if (HasRealChildren())
        {
        //see http://stackoverflow.com/questions/14593995/problems-with-stdfunction for use of std::mem_fn below
        auto func = std::function < bool(SMMeshIndexNode<POINT, EXTENT>*,uint64_t, bool,bool) >();
        switch (action)
            {
            case ACTION_ADD:
                func = std::mem_fn(&SMMeshIndexNode<POINT, EXTENT>::AddClip);
                break;
            case ACTION_MODIFY:
                func = std::mem_fn(&SMMeshIndexNode<POINT, EXTENT>::ModifyClip);
                break;
            default:
                break;
            }
        if (m_pSubNodeNoSplit != nullptr)
            {
            createdTasks.push_back(SMTask(std::bind(func, (SMMeshIndexNode<POINT, EXTENT>*) m_pSubNodeNoSplit.GetPtr(), clipId,  false,true),false));
            }
        else
            {
            for (size_t i = 0; i < GetNumberOfSubNodesOnSplit(); ++i)
                {
                if (m_apSubNodes[i] != nullptr)
                    {
                    createdTasks.push_back(SMTask(std::bind(func, (SMMeshIndexNode<POINT, EXTENT>*) m_apSubNodes[i].GetPtr(), clipId, false,true),false));
                    }
                }
            }
        }

    s_schedulerLock.lock();
    if (s_clipScheduler == nullptr) s_clipScheduler = new ScalableMeshScheduler();

    for (auto& task : createdTasks) s_clipScheduler->ScheduleTask(task);

    s_schedulerLock.unlock();
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 09/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::PropagateClipToNeighbors(uint64_t clipId,  ClipAction action)
    {
    std::vector<SMTask> createdTasks;
    auto func = std::function < bool(SMMeshIndexNode<POINT, EXTENT>*, uint64_t,  bool, bool) >();
    switch (action)
        {
        case ACTION_ADD:
            func = std::mem_fn(&SMMeshIndexNode<POINT, EXTENT>::AddClip);
            break;
        case ACTION_MODIFY:
            func = std::mem_fn(&SMMeshIndexNode<POINT, EXTENT>::ModifyClip);
            break;

        default:
            break;
        }

    for (size_t n = 0; n < MAX_NUM_NEIGHBORNODE_POSITIONS; ++n)
        {
        for (auto& node : m_apNeighborNodes[n])
            if (node != nullptr)
                {
                createdTasks.push_back(SMTask(std::bind(func, (SMMeshIndexNode<POINT, EXTENT>*)node.GetPtr(), clipId,  false,true), false));
                }
        }

    s_schedulerLock.lock();
    if (s_clipScheduler == nullptr) s_clipScheduler = new ScalableMeshScheduler();

    for (auto& task : createdTasks) s_clipScheduler->ScheduleTask(task);

    s_schedulerLock.unlock();
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 09/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::PropagateClipUpwards(uint64_t clipId, ClipAction action)
    {
    std::vector<SMTask> createdTasks;
    auto func = std::function < bool(SMMeshIndexNode<POINT, EXTENT>*, uint64_t, bool, bool) >();
    switch (action)
        {
        case ACTION_ADD:
            func = std::mem_fn(&SMMeshIndexNode<POINT, EXTENT>::AddClip);
            break;
        case ACTION_MODIFY:
            func = std::mem_fn(&SMMeshIndexNode<POINT, EXTENT>::ModifyClip);
            break;
        default:
            break;
        }
    auto node = GetParentNode();
    while (node != nullptr && node->GetLevel() > 1)
        {
        node = node->GetParentNode();
        }
    if (node != nullptr)
        {
        createdTasks.push_back(SMTask(std::bind(func, (SMMeshIndexNode<POINT, EXTENT>*)node.GetPtr(), clipId, false,true), false));
        if (node->GetParentNode() != nullptr)
            {
            createdTasks.push_back(SMTask(std::bind(func, (SMMeshIndexNode<POINT, EXTENT>*)node->GetParentNode().GetPtr(), clipId, false,true), false));

            for (size_t i = 0; i < node->GetParentNode()->GetNumberOfSubNodesOnSplit(); ++i)
                {
                if (node->GetParentNode()->m_apSubNodes[i] != nullptr) createdTasks.push_back(SMTask(std::bind(func, (SMMeshIndexNode<POINT, EXTENT>*)node->GetParentNode()->m_apSubNodes[i].GetPtr(), clipId, false,true), false));
                }
            }
        }
    s_schedulerLock.lock();
    if (s_clipScheduler == nullptr) s_clipScheduler = new ScalableMeshScheduler();

    for (auto& task : createdTasks) s_clipScheduler->ScheduleTask(task, ScalableMeshScheduler::PRIORITY_HIGH);

    s_schedulerLock.unlock();
    }

    //=======================================================================================
    // @bsimethod                                                   Mathieu.St-Pierre 10/14
    //=======================================================================================
    template<class POINT, class EXTENT> bool SMMeshIndexNode<POINT, EXTENT>::NeedsMeshing() const
        {
        if (!IsLoaded())
            Load();

        bool needsMeshing = false;

        if (!HasRealChildren())
            needsMeshing = m_nodeHeader.m_nbFaceIndexes == 0;
        else
            {
            if (IsParentOfARealUnsplitNode())
                {
                needsMeshing = m_nodeHeader.m_nbFaceIndexes == 0 || static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->NeedsMeshing();
                }
            else
                {
                needsMeshing = m_nodeHeader.m_nbFaceIndexes == 0;

                for (size_t indexNode = 0; !needsMeshing && indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                    {
                    needsMeshing = (needsMeshing || static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->NeedsMeshing());
                    }
                }
            }

        return needsMeshing;

        }
    static size_t s_importedFeatures;

template <class POINT, class EXTENT> SMMeshIndex<POINT, EXTENT>::SMMeshIndex(HFCPtr<HPMCountLimitedPool<POINT> > pool,
                                                                               HFCPtr<SMPointTileStore<POINT, EXTENT> > store,
                                                                               HFCPtr<HPMCountLimitedPool<int32_t>> ptsIndicesPool,
                                                                               HFCPtr<SMPointTileStore<int32_t, EXTENT> > ptsIndicesStore,
                                                                               HFCPtr<HPMIndirectCountLimitedPool<MTGGraph> > graphPool,
                                                                               HFCPtr<IHPMPermanentStore<MTGGraph, Byte, Byte>> graphStore,
                                                                               HFCPtr<HPMCountLimitedPool<Byte>> texturesPool,
                                                                               HFCPtr<IHPMPermanentStore<Byte, float, float>> texturesStore,
                                                                               HFCPtr<HPMCountLimitedPool<DPoint2d>> uvPool,
                                                                               HFCPtr<SMPointTileStore<DPoint2d, EXTENT>> uvStore,
                                                                               HFCPtr<HPMCountLimitedPool<int32_t>> uvIndicesPool,
                                                                               HFCPtr<SMPointTileStore<int32_t, EXTENT>> uvIndicesStore,
                                                                               size_t SplitTreshold,
                                                                               ISMPointIndexFilter<POINT, EXTENT>* filter,
                                                                               bool balanced,
                                                                               bool textured,
                                                                               bool propagatesDataDown,
                                                                               ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                                                                               ISMPointIndexMesher<POINT, EXTENT>* mesher3d)
                                                                               : SMPointIndex<POINT, EXTENT>(pool, store, SplitTreshold, filter, balanced, propagatesDataDown, false), m_graphPool(graphPool), m_graphStore(graphStore),
                                                                               m_ptsIndicesPool(ptsIndicesPool), m_ptsIndicesStore(ptsIndicesStore),
                                                                               m_uvPool(uvPool), m_uvStore(uvStore),
                                                                               m_uvsIndicesPool(uvIndicesPool), m_uvsIndicesStore(uvIndicesStore),
                                                                               m_texturesPool(texturesPool), m_texturesStore(texturesStore)
    {
    m_mesher2_5d = mesher2_5d;
    m_mesher3d = mesher3d;
    if (0 == graphStore->LoadMasterHeader(NULL, 0)) graphStore->StoreMasterHeader(NULL, 0);
    m_featureStore = nullptr;
    m_featurePool = nullptr;
    m_clipStore = nullptr;
    m_clipPool = nullptr;
    s_importedFeatures = 0;
    if (m_indexHeader.m_rootNodeBlockID.IsValid() && m_pRootNode == nullptr)
        {
        m_pRootNode = CreateNewNode(m_indexHeader.m_rootNodeBlockID);
        }
    if (0 == ptsIndicesStore->LoadMasterHeader(NULL, 0)) ptsIndicesStore->StoreMasterHeader(NULL, 0);
    if (0 == uvStore->LoadMasterHeader(NULL, 0)) uvStore->StoreMasterHeader(NULL, 0);
    if (0 == uvIndicesStore->LoadMasterHeader(NULL, 0)) uvIndicesStore->StoreMasterHeader(NULL, 0);
    if (0 == texturesStore->LoadMasterHeader(NULL, 0)) texturesStore->StoreMasterHeader(NULL, 0);
    }


template <class POINT, class EXTENT> SMMeshIndex<POINT, EXTENT>::~SMMeshIndex()
    {
    if (m_mesher2_5d != NULL)
        delete m_mesher2_5d;

    if (m_mesher3d != NULL)
        delete m_mesher3d;
    Store();
    if (m_pRootNode != NULL)
        m_pRootNode->Unload();

    m_pRootNode = NULL;

    }


template <class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndex<POINT, EXTENT>::CreateNewNode(EXTENT extent, bool isRootNode)
    {
    SMMeshIndexNode<POINT, EXTENT> * meshNode = new SMMeshIndexNode<POINT, EXTENT>(m_indexHeader.m_SplitTreshold, extent, m_pool, m_store, this, m_filter, m_needsBalancing, IsTextured(), PropagatesDataDown(), m_mesher2_5d, m_mesher3d, &m_createdNodeMap);
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = dynamic_cast<SMPointIndexNode<POINT, EXTENT>*>(meshNode);
    pNewNode->m_isGenerating = m_isGenerating;

    if (isRootNode)
        {
        HFCPtr<SMPointIndexNode<POINT, EXTENT>> parentNodePtr;
        pNewNode->SetParentNodePtr(parentNodePtr);
        }

    return pNewNode;
    }

template<class POINT, class EXTENT>  HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndex<POINT, EXTENT>::CreateNewNode(HPMBlockID blockID, bool isRootNode)
    {
    auto meshNode = new SMMeshIndexNode<POINT, EXTENT>(blockID, m_pool, m_store, this,m_filter, m_needsBalancing, IsTextured(), PropagatesDataDown(), m_mesher2_5d, m_mesher3d, &m_createdNodeMap);
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = static_cast<SMPointIndexNode<POINT, EXTENT> *>(meshNode);
    pNewNode->m_isGenerating = m_isGenerating;

    if (isRootNode)
        {
        HFCPtr<SMPointIndexNode<POINT, EXTENT>> parentNodePtr;
        pNewNode->SetParentNodePtr(parentNodePtr);
        }

    return pNewNode;
    }


template<class POINT, class EXTENT> ISMPointIndexMesher<POINT, EXTENT>* SMMeshIndex<POINT, EXTENT>::GetMesher2_5d()
    {
    return(m_mesher2_5d);
    }

template<class POINT, class EXTENT> ISMPointIndexMesher<POINT, EXTENT>* SMMeshIndex<POINT, EXTENT>::GetMesher3d()
    {
    return(m_mesher3d);
    }


template<class POINT, class EXTENT>  void  SMMeshIndex<POINT, EXTENT>::TextureFromRaster(HIMMosaic* sourceRasterP)
    {
    if (m_indexHeader.m_terrainDepth == (size_t)-1)
        {
        m_indexHeader.m_terrainDepth = m_pRootNode->GetDepth();
        m_indexHeader.m_depth = (size_t)-1;
        }
    if (sourceRasterP == nullptr || sourceRasterP->GetEffectiveShape() == nullptr || sourceRasterP->GetEffectiveShape()->IsEmpty()) return;
    if (m_pRootNode != NULL)   dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pRootNode)->TextureFromRasterRecursive(sourceRasterP);
    }

template<class POINT, class EXTENT>  void  SMMeshIndex<POINT, EXTENT>::PerformClipAction(ClipAction action, uint64_t clipId, DRange3d& extent, bool setToggledWhenIDIsOn)
    {
    if (m_pRootNode != NULL)   dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pRootNode)->ClipActionRecursive(action, clipId, extent, setToggledWhenIDIsOn);
    }

template<class POINT, class EXTENT>  void  SMMeshIndex<POINT, EXTENT>::AddFeatureDefinition(IDTMFile::FeatureType type, bvector<DPoint3d>& points, DRange3d& extent)
    {
    ++s_importedFeatures;
    if (0 == points.size())
        return;

    // Check if initial node allocated
    if (m_pRootNode == 0)
        {
        // There is no root node at the moment
        // Allocate root node the size of the object extent
        if (m_indexHeader.m_HasMaxExtent)
            m_pRootNode = CreateNewNode(m_indexHeader.m_MaxExtent); 
        else
        m_pRootNode = CreateNewNode(ExtentOp<EXTENT>::Create(extent.low.x, extent.low.y, extent.low.z, extent.high.x, extent.high.y, extent.high.z), true);
        }
    size_t nAddedPoints = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pRootNode)->AddFeatureDefinition(type, points, extent, m_indexHeader.m_HasMaxExtent);
    if (0 == nAddedPoints)
        {
        //can't add feature, need to grow extent
        DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_pRootNode->m_nodeHeader.m_nodeExtent),
                                            ExtentOp<EXTENT>::GetXMax(m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_pRootNode->m_nodeHeader.m_nodeExtent));
        while (!extent.IsContained(nodeRange))
            {
            // If the item is not in root node and extent is limited then it is impossible to add item
            if (m_indexHeader.m_HasMaxExtent)
                return;

            // The extent is not contained... we must create a new node
            PushRootDown(ExtentOp<EXTENT>::Create(extent.low.x, extent.low.y, extent.low.z, extent.high.x, extent.high.y, extent.high.z));
            nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_pRootNode->m_nodeHeader.m_nodeExtent),
                                       ExtentOp<EXTENT>::GetXMax(m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_pRootNode->m_nodeHeader.m_nodeExtent));
            }


        // The root node contains the spatial object ... add it
        nAddedPoints = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pRootNode)->AddFeatureDefinition(type, points, extent, m_indexHeader.m_HasMaxExtent);
        assert(nAddedPoints >= points.size());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Elenie.Godzaridis   10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<class POINT, class EXTENT>  void  SMMeshIndex<POINT, EXTENT>::AddClipDefinition(bvector<DPoint3d>& points, DRange3d& extent)
    {
    if (m_pRootNode != NULL)   dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pRootNode)->AddClipDefinitionRecursive(points,extent);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Elenie.Godzaridis   10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<class POINT, class EXTENT>  void  SMMeshIndex<POINT, EXTENT>::RefreshMergedClips()
    {
    if (m_pRootNode != NULL)   dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pRootNode)->RefreshMergedClipsRecursive();
    }

/**----------------------------------------------------------------------------
Mesh
Mesh the data.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void SMMeshIndex<POINT, EXTENT>::Mesh()
    {
    HINVARIANTS;

    bool result = m_mesher2_5d->Init(*this);
    assert(result == true);

    result = m_mesher3d->Init(*this);
    assert(result == true);

    // Check if root node is present
    if (m_pRootNode != NULL)
        {
        HFCPtr<SMMeshIndexNode<POINT, EXTENT>> node = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pRootNode);
            node->Mesh();
        }

    HINVARIANTS;
    }
/**----------------------------------------------------------------------------
Stitch
Stitch the data.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void SMMeshIndex<POINT, EXTENT>::Stitch(int pi_levelToStitch, bool do2_5dStitchFirst)
    {
    HINVARIANTS;

    // Check if root node is present
    if (m_pRootNode != NULL)
        {

        try
            {

            if (do2_5dStitchFirst)
                {
                //Done for level stitching.
                assert(pi_levelToStitch != -1);
                vector<SMMeshIndexNode<POINT, EXTENT>*> nodesToStitch;
                ((SMMeshIndexNode<POINT, EXTENT>*)&*m_pRootNode)->Stitch(pi_levelToStitch, &nodesToStitch);

                auto nodeIter(nodesToStitch.begin());

                //Do 2.5d stitch first
                while (nodeIter != nodesToStitch.end())
                    {
                    if (!(*nodeIter)->m_nodeHeader.m_arePoints3d && (*nodeIter)->AreAllNeighbor2_5d())
                        {
                        if (m_mesher2_5d->Stitch(*nodeIter))
                            {
                            (*nodeIter)->SetDirty(true);
                            }

                        nodeIter = nodesToStitch.erase(nodeIter);
                        }
                    else
                        {
                        nodeIter++;
                        }
                    }

                //Do 3d stitch last
                nodeIter = nodesToStitch.begin();
                auto nodeIterEnd = nodesToStitch.end();

                while (nodeIter != nodeIterEnd)
                    {
                    assert((*nodeIter)->m_nodeHeader.m_arePoints3d || !(*nodeIter)->AreAllNeighbor2_5d());

                    if (m_mesher3d->Stitch(*nodeIter))
                        {
                        (*nodeIter)->SetDirty(true);
                        }

                    nodeIter++;
                    }
                }
            else if (s_useThreadsInStitching)
                {
                vector<SMMeshIndexNode<POINT, EXTENT>*> nodesToStitch;
                ((SMMeshIndexNode<POINT, EXTENT>*)&*m_pRootNode)->Stitch(pi_levelToStitch, &nodesToStitch);
                if (nodesToStitch.size() == 0) return;
                if (nodesToStitch.size() <= 72)
                    {
                    ((SMMeshIndexNode<POINT, EXTENT>*)&*m_pRootNode)->Stitch(pi_levelToStitch, 0);
                    return;
                    }
                set<SMMeshIndexNode<POINT, EXTENT>*> stitchedNodes;
                std::recursive_mutex stitchedMutex;
                for (auto& node : nodesToStitch) s_nodeMap.insert(std::make_pair((void*)node, (unsigned int)-1));
                for (size_t i =0; i < 8; ++i)
                    RunOnNextAvailableThread(std::bind([] (SMMeshIndexNode<POINT, EXTENT>** vec, set<SMMeshIndexNode<POINT,EXTENT>*>* stitchedNodes, std::recursive_mutex* stitchedMutex, size_t nNodes, size_t threadId) ->void
                    {
                    vector<SMMeshIndexNode<POINT, EXTENT>*> myNodes;
                    size_t firstIdx = nNodes / 8 * threadId;
                    myNodes.push_back(vec[firstIdx]);
                    while (myNodes.size() > 0)
                        {
                        //grab node
                        SMMeshIndexNode<POINT, EXTENT>* current = myNodes[0];
                        vector<SMPointIndexNode<POINT, EXTENT>*> neighbors;
                        current->GetAllNeighborNodes(neighbors);
                        neighbors.push_back(current);
                        bool reservedNode = false;

                        reservedNode = TryReserveNodes(s_nodeMap, (void**)&neighbors[0], neighbors.size(),(unsigned int)threadId);

                        if (reservedNode == true)
                            {
                            bool needsStitching = false;
                            stitchedMutex->lock();
                            if (stitchedNodes->count(current) != 0) needsStitching = false;
                            else
                                {
                                stitchedNodes->insert(current);
                                needsStitching = true;
                                }
                            for (auto& neighbor : neighbors)
                                if (std::find(myNodes.begin(), myNodes.end(), neighbor) == myNodes.end() && stitchedNodes->count((SMMeshIndexNode<POINT,EXTENT>*)neighbor) == 0) myNodes.push_back((SMMeshIndexNode<POINT, EXTENT>*)neighbor);
                            stitchedMutex->unlock();
                            if(needsStitching) current->Stitch((int)current->m_nodeHeader.m_level, 0);
                            unsigned int val = (unsigned int)-1;
                            unsigned int id = (unsigned int)threadId;
                            s_nodeMap[current].compare_exchange_strong(id, val);
                            }
                        myNodes.erase(myNodes.begin());
                        if (myNodes.size() == 0 && firstIdx + 1 < nNodes / 8 * (threadId+1))
                            {
                            firstIdx += 1;
                            myNodes.push_back(vec[firstIdx]);
                            }
                        }
                    SetThreadAvailableAsync(threadId);
                    }, &nodesToStitch[0], &stitchedNodes, &stitchedMutex, nodesToStitch.size(), std::placeholders::_1));

                WaitForThreadStop();
                s_nodeMap.clear();
                for (auto& node : nodesToStitch)
                    {
                    if (stitchedNodes.count(node) == 0)
                        node->Stitch((int)node->m_nodeHeader.m_level, 0);
                    }
                }
            else
                {
                ((SMMeshIndexNode<POINT, EXTENT>*)&*m_pRootNode)->Stitch(pi_levelToStitch, 0);
                }
            }
        catch (...)
            {
            throw;
            }
        }

    HINVARIANTS;
    }


template<class POINT, class EXTENT>  void  SMMeshIndex<POINT, EXTENT>::SetFeatureStore(HFCPtr<SMPointTileStore<int32_t, EXTENT>>& featureStore)
    {
    m_featureStore = featureStore;
    }

template<class POINT, class EXTENT>  void SMMeshIndex<POINT, EXTENT>::SetFeaturePool(HFCPtr<HPMCountLimitedPool<int32_t>>& featurePool)
    {
    m_featurePool = featurePool;
    }

template<class POINT, class EXTENT>  void  SMMeshIndex<POINT, EXTENT>::SetClipStore(HFCPtr<IHPMPermanentStore<DifferenceSet, Byte, Byte>>& clipStore)
    {
    m_clipStore = clipStore;
    //if (!m_clipStore->LoadMasterHeader(NULL, 1)) m_clipStore->StoreMasterHeader(NULL, 0);
    if (m_pRootNode != nullptr)dynamic_cast<SMMeshIndexNode<POINT, EXTENT>*>(m_pRootNode.GetPtr())->m_differenceSets.SetStore(m_clipStore);
    }


template<class POINT, class EXTENT>  void  SMMeshIndex<POINT, EXTENT>::SetClipRegistry(ClipRegistry* clipRegistry)
    {
    m_clipRegistry = clipRegistry;
    }

template<class POINT, class EXTENT>  void SMMeshIndex<POINT, EXTENT>::SetClipPool(HFCPtr<HPMIndirectCountLimitedPool<DifferenceSet>>& clipPool)
    {
    m_clipPool = clipPool;
    if (m_pRootNode != nullptr)dynamic_cast<SMMeshIndexNode<POINT, EXTENT>*>(m_pRootNode.GetPtr())->m_differenceSets.SetPool(m_clipPool);
    }
