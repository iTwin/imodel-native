/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshNodeCreator.cpp $
|    $RCSfile: ScalableMeshNodeCreator.cpp,v $
|   $Revision: 1.90 $
|       $Date: 2015/07/15 21:55:29 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"
USING_NAMESPACE_BENTLEY_TERRAINMODEL

#include "ScalableMesh.h"
#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh/IScalableMeshNodeCreator.h>
#include "ScalableMeshNodeCreator.h"
#include "ScalableMeshQuery.h"
#include "InternalUtilityFunctions.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
IScalableMeshNodeCreatorPtr IScalableMeshNodeCreator::GetFor(const WChar*  filePath,
StatusInt&      status)
    {


    IScalableMeshNodeCreatorPtr pCreator = new IScalableMeshNodeCreator(new Impl(filePath));

    status = pCreator->m_implP->LoadFromFile();
    if (BSISUCCESS != status)
        return 0;

    return pCreator.get();
    }


IScalableMeshNodeCreatorPtr IScalableMeshNodeCreator::GetFor(const IScalableMeshPtr&    scmPtr,
                                                                 StatusInt&          status)
    {

    IScalableMeshNodeCreatorPtr pCreator = new IScalableMeshNodeCreator(new Impl(scmPtr));

    status = pCreator->m_implP->LoadFromFile();
    if (BSISUCCESS != status)
        return 0;

    return pCreator.get();
    }


IScalableMeshNodeCreator::IScalableMeshNodeCreator(Impl* implP)
    : IScalableMeshCreator(implP)
    {}


IScalableMeshNodeCreator::~IScalableMeshNodeCreator()
    {}


IScalableMeshNodeCreator::Impl::Impl(const WChar* scmFileName)
    : IScalableMeshCreator::Impl(scmFileName)
    {
    }

IScalableMeshNodeCreator::Impl::Impl(const IScalableMeshPtr& scmPtr)
    : IScalableMeshCreator::Impl(scmPtr)
    {
    }

IScalableMeshNodeCreator::Impl::~Impl()
    {
    m_pDataIndex = 0;
    m_scmPtr = 0;
    }


IScalableMeshNodeEditPtr IScalableMeshNodeCreator::AddNode(const IScalableMeshNodePtr& parentNode,
                                                       DRange3d& extent,
                                                       StatusInt&                  status,
                                                       bool computeNodeID,
                                                       uint64_t nodeId)
    {
    status = BSISUCCESS;
    return dynamic_cast<IScalableMeshNodeCreator::Impl*>(m_implP.get())->AddChildNode(parentNode,extent, status, computeNodeID, nodeId);
    }

IScalableMeshNodeEditPtr IScalableMeshNodeCreator::AddNode(StatusInt&                  status,
                                                           bool computeNodeID,
                                                           uint64_t nodeId)
    {
    status = BSISUCCESS;
    return dynamic_cast<IScalableMeshNodeCreator::Impl*>(m_implP.get())->AddNode(status, computeNodeID, nodeId);
    }


int64_t  IScalableMeshNodeCreator::AddTexture(int width, int height, int nOfChannels, const byte* texData)
    {
    return dynamic_cast<IScalableMeshNodeCreator::Impl*>(m_implP.get())->AddTexture(width,height,nOfChannels, texData);
    }

void IScalableMeshNodeCreator::NotifyAllChildrenAdded(const IScalableMeshNodePtr& parentNode,
                                                      StatusInt&                  status,
                                                      bool computeNeighbors)
    {
    status = BSISUCCESS;
    return dynamic_cast<IScalableMeshNodeCreator::Impl*>(m_implP.get())->NotifyAllChildrenAdded(parentNode, status, computeNeighbors);
    }

int IScalableMeshNodeCreator::Impl::CreateScalableMesh(bool isSingleFile)
    {
    int status = BSISUCCESS;

    //MS : Some cleanup needs to be done here.
    try
        {
        if (m_scmPtr != 0)
            {
            if (SCM_STATE_UP_TO_DATE == m_scmPtr->GetState())
                return BSIERROR;

            // NOTE: Need to be able to recreate : Or the file offers some functions for deleting all its data directory or the file name can be obtained
            }


        SetupFileForCreation();

        m_smSQLitePtr->SetSingleFile(isSingleFile);

        StatusInt status = CreateDataIndex(m_pDataIndex);

        assert(status == SUCCESS && m_pDataIndex != 0);

        }
    catch (...)
        {
        return BSIERROR;
        }

    return status;
    }

void IScalableMeshNodeCreator::Impl::NotifyAllChildrenAdded(const IScalableMeshNodePtr& parentNode,
                                                            StatusInt&                  status,
                                                            bool computeNeighbors)
    {
    if (m_pDataIndex == 0 || parentNode == 0)
        {
        status = BSIERROR;
        return;
        }
    
    DRange3d extent = parentNode->GetNodeExtent();
    PointIndexExtentType ext = ExtentOp<PointIndexExtentType>::Create(extent.low.x, extent.low.y, extent.low.z, extent.high.x, extent.high.y, extent.high.z);
    auto nodeP = m_pDataIndex->FindNode(ext, parentNode->GetLevel());
    if (nodeP.GetPtr() == nullptr)
        {
        status = BSIERROR;
        return;
        }
    nodeP->SortSubNodes();
    if (computeNeighbors) nodeP->SetupNeighborNodesAfterSplit();
    status = BSISUCCESS;
    return;
    }

IScalableMeshNodeEditPtr IScalableMeshNodeCreator::Impl::AddChildNode(const IScalableMeshNodePtr& parentNode,
                                                                      DRange3d&                   childExtent,
                                                                      StatusInt&                  status,
                                                                      bool computeNodeID,
                                                                      uint64_t nodeId)
    {

    if (m_pDataIndex == 0)
        {
        if (CreateScalableMesh(true) != BSISUCCESS)
            {
            status = BSIERROR;
            return IScalableMeshNodeEditPtr();
            }
        }
    if (parentNode == 0)
        {
        return AddNode(status);
        }

    if (parentNode != 0)
        {
        DRange3d extent = parentNode->GetNodeExtent();
        PointIndexExtentType ext = ExtentOp<PointIndexExtentType>::Create(extent.low.x, extent.low.y, extent.low.z, extent.high.x, extent.high.y, extent.high.z);
        auto nodeP = m_pDataIndex->FindNode(ext, parentNode->GetLevel());
        if (nodeP.GetPtr() == nullptr)
            {
            status = BSIERROR;
            return IScalableMeshNodeEditPtr();
            }

        PointIndexExtentType newExtent = ExtentOp<PointIndexExtentType>::Create(childExtent.low.x, childExtent.low.y, childExtent.low.z, childExtent.high.x, childExtent.high.y, childExtent.high.z);
        auto childNodeP = nodeP->AddChild(newExtent, computeNodeID, nodeId);
        status = m_pDataIndex->FindNode(newExtent, childNodeP->GetLevel()) != nullptr ? BSISUCCESS : BSIERROR;
        return IScalableMeshNodeEditPtr(new ScalableMeshNodeEdit<PointType>(childNodeP));
        }
    status = BSISUCCESS;
    return IScalableMeshNodeEditPtr();
    }

IScalableMeshNodeEditPtr IScalableMeshNodeCreator::Impl::AddNode(StatusInt&   status,
                                                                 bool computeNodeID,
                                                                 uint64_t nodeId)
    {
    if (m_pDataIndex == 0)
        {
        if (CreateScalableMesh(true) != BSISUCCESS)
            {
            status = BSIERROR;
            return IScalableMeshNodeEditPtr();
            }
        }
    if (m_pDataIndex->GetRootNode() != nullptr)
        {
        status = BSIERROR;
        auto rootNodeP = m_pDataIndex->GetRootNode();
        return IScalableMeshNodeEditPtr(new ScalableMeshNodeEdit<PointType>(rootNodeP));
        }
    auto rootNodeP = computeNodeID ? m_pDataIndex->CreateRootNode() : m_pDataIndex->CreateRootNode(nodeId);
    return IScalableMeshNodeEditPtr(new ScalableMeshNodeEdit<PointType>(rootNodeP));
    }


int64_t  IScalableMeshNodeCreator::Impl::AddTexture(int width, int height, int nOfChannels, const byte* texData)
    {
    if (m_pDataIndex == 0)
        {
        if (CreateScalableMesh(true) != BSISUCCESS)
            {
            return -1;
            }
        }

    ISMTextureDataStorePtr nodeDataStore;
    SMIndexNodeHeader<PointIndexExtentType> nodeHeader;
    bool result = m_pDataIndex->GetDataStore()->GetNodeDataStore(nodeDataStore, &nodeHeader);
    assert(result == true);

    size_t texID = m_pDataIndex->GetNextTextureId();

    size_t size = width*height*nOfChannels + 3 * sizeof(int);
    bvector<uint8_t> texture(size);
    memcpy(texture.data(), &width, sizeof(int));
    memcpy(texture.data()+sizeof(int), &height, sizeof(int));
    memcpy(texture.data() + 2 * sizeof(int), &nOfChannels, sizeof(int));
    memcpy(texture.data() + 3 * sizeof(int), texData, width*height*nOfChannels);
    RefCountedPtr<SMStoredMemoryPoolBlobItem<Byte>> storedMemoryPoolVector(
#ifndef VANCOUVER_API
        new SMStoredMemoryPoolBlobItem<Byte>(texID, nodeDataStore, texture.data(), size, SMStoreDataType::Texture, (uint64_t)(m_pDataIndex.GetPtr()))
#else
        SMStoredMemoryPoolBlobItem<Byte>::CreateItem(texID, nodeDataStore, texture.data(), size,SMStoreDataType::Texture, (uint64_t)(m_pDataIndex.GetPtr()))
#endif
        );
    SMMemoryPoolItemBasePtr memPoolItemPtr(storedMemoryPoolVector.get());
    auto texPoolId = m_pDataIndex->GetMemoryPool()->AddItem(memPoolItemPtr);
    m_pDataIndex->GetMemoryPool()->RemoveItem(texPoolId, texID, SMStoreDataType::Texture, (uint64_t)(m_pDataIndex.GetPtr()));
    return (int64_t) texID;
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE