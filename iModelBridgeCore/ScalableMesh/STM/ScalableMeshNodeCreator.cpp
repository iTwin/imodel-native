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
                                                       StatusInt&                  status)
    {
    status = BSISUCCESS;
    return dynamic_cast<IScalableMeshNodeCreator::Impl*>(m_implP.get())->AddChildNode(parentNode,extent, status);
    }

IScalableMeshNodeEditPtr IScalableMeshNodeCreator::AddNode(StatusInt&                  status)
    {
    status = BSISUCCESS;
    return dynamic_cast<IScalableMeshNodeCreator::Impl*>(m_implP.get())->AddNode(status);
    }


void IScalableMeshNodeCreator::NotifyAllChildrenAdded(const IScalableMeshNodePtr& parentNode,
                                                      StatusInt&                  status)
    {
    status = BSISUCCESS;
    return dynamic_cast<IScalableMeshNodeCreator::Impl*>(m_implP.get())->NotifyAllChildrenAdded(parentNode, status);
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

        //NEEDS_WORK_SM : Try put it in CreateDataIndex as sharedptr
        HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment myMemMgr(100, 2000 * sizeof(PointType));

        StatusInt status = CreateDataIndex(m_pDataIndex, myMemMgr);

        assert(status == SUCCESS && m_pDataIndex != 0);

        }
    catch (...)
        {
        return BSIERROR;
        }

    return status;
    }

void IScalableMeshNodeCreator::Impl::NotifyAllChildrenAdded(const IScalableMeshNodePtr& parentNode,
                                                            StatusInt&                  status)
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
    nodeP->SetupNeighborNodesAfterSplit();
    status = BSISUCCESS;
    return;
    }

IScalableMeshNodeEditPtr IScalableMeshNodeCreator::Impl::AddChildNode(const IScalableMeshNodePtr& parentNode,
                                                                      DRange3d&                   childExtent,
                                                                      StatusInt&                  status)
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
        auto childNodeP = nodeP->AddChild(newExtent);
        status = m_pDataIndex->FindNode(newExtent, childNodeP->GetLevel()) != nullptr ? BSISUCCESS : BSIERROR;
        return IScalableMeshNodeEditPtr(new ScalableMeshNodeEdit<PointType>(childNodeP));
        }
    status = BSISUCCESS;
    return IScalableMeshNodeEditPtr();
    }

IScalableMeshNodeEditPtr IScalableMeshNodeCreator::Impl::AddNode(StatusInt&   status)
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
    auto rootNodeP = m_pDataIndex->CreateRootNode();
    return IScalableMeshNodeEditPtr(new ScalableMeshNodeEdit<PointType>(rootNodeP));
    }
END_BENTLEY_SCALABLEMESH_NAMESPACE