/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshSourceCreatorWorker.cpp $
|    $RCSfile: ScalableMeshSourceCreator.cpp,v $
|   $Revision: 1.90 $
|       $Date: 2015/07/15 10:41:29 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>

#include <process.h>

#include <BeXml/BeXml.h>
#include "ImagePPHeaders.h"
#include "ScalableMeshSourceCreatorWorker.h"
#include "ScalableMeshQuadTreeBCLIBFilters.h"

#define TASK_PER_WORKER 3

USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

StatusInt IScalableMeshSourceCreatorWorker::CreateMeshTasks() const
    {    
    return static_cast<IScalableMeshSourceCreatorWorker::Impl*>(m_implP.get())->CreateMeshTasks();
    }
	
StatusInt IScalableMeshSourceCreatorWorker::CreateTaskPlan() const
    {
    return static_cast<IScalableMeshSourceCreatorWorker::Impl*>(m_implP.get())->CreateTaskPlan();
    }

StatusInt IScalableMeshSourceCreatorWorker::CreateGenerationTasks(uint32_t maxGroupSize, const WString& jobName, const BeFileName& smFileName) const
    {
    return static_cast<IScalableMeshSourceCreatorWorker::Impl*>(m_implP.get())->CreateGenerationTasks(maxGroupSize, jobName, smFileName);
    }

StatusInt IScalableMeshSourceCreatorWorker::ExecuteNextTaskInTaskPlan() const
    {
    return static_cast<IScalableMeshSourceCreatorWorker::Impl*>(m_implP.get())->ExecuteNextTaskInTaskPlan();
    }

StatusInt IScalableMeshSourceCreatorWorker::ProcessMeshTask(BeXmlNodeP pXmlTaskNode) const
    {   
    return static_cast<IScalableMeshSourceCreatorWorker::Impl*>(m_implP.get())->ProcessMeshTask(pXmlTaskNode);
    }

StatusInt IScalableMeshSourceCreatorWorker::ProcessStitchTask(BeXmlNodeP pXmlTaskNode) const
    {
    return static_cast<IScalableMeshSourceCreatorWorker::Impl*>(m_implP.get())->ProcessStitchTask(pXmlTaskNode);
    }

StatusInt IScalableMeshSourceCreatorWorker::ProcessFilterTask(BeXmlNodeP pXmlTaskNode) const
    {
    return static_cast<IScalableMeshSourceCreatorWorker::Impl*>(m_implP.get())->ProcessFilterTask(pXmlTaskNode);
    }

StatusInt IScalableMeshSourceCreatorWorker::ProcessGenerateTask(BeXmlNodeP pXmlTaskNode) const
    {   
    return static_cast<IScalableMeshSourceCreatorWorker::Impl*>(m_implP.get())->ProcessGenerateTask(pXmlTaskNode);
    }


IScalableMeshSourceCreatorWorkerPtr IScalableMeshSourceCreatorWorker::GetFor(const WChar*  filePath,
                                                                             uint32_t      nbWorkers,
                                                                             StatusInt&    status)
    {
    RegisterDelayedImporters();

    using namespace ISMStore;
    BeFileName fileName = BeFileName(filePath);

#ifdef VANCOUVER_API
    if (fileName.IsUrl() || (!BeFileName::DoesPathExist(fileName.c_str()) && !canCreateFile(filePath)))
#else
    if (IsUrl(fileName) || (!fileName.DoesPathExist() /*&& !canCreateFile(filePath)*/))
#endif	
    {
        status = BSIERROR;
        return 0;
    }
    
    IScalableMeshSourceCreatorWorkerPtr pCreatorWorker = new IScalableMeshSourceCreatorWorker(new IScalableMeshSourceCreatorWorker::Impl(filePath, nbWorkers));    
    IScalableMeshSourceCreatorWorker::Impl* implP(dynamic_cast<IScalableMeshSourceCreatorWorker::Impl*>(pCreatorWorker->m_implP.get()));
    assert(implP != nullptr);


    implP->OpenSqlFiles(false, true);
    status = pCreatorWorker->m_implP->LoadFromFile();
    implP->CloseSqlFiles();

    if (BSISUCCESS != status)
        return 0;
    
    return pCreatorWorker.get();
}


IScalableMeshSourceCreatorWorkerPtr IScalableMeshSourceCreatorWorker::GetFor(const IScalableMeshPtr& scmPtr,
                                                                             uint32_t                nbWorkers,
                                                                             StatusInt&              status)
    {
    using namespace ISMStore;

    RegisterDelayedImporters();

    IScalableMeshSourceCreatorWorkerPtr pCreatorWorker = new IScalableMeshSourceCreatorWorker(new IScalableMeshSourceCreatorWorker::Impl(scmPtr, nbWorkers));

    status = pCreatorWorker->m_implP->LoadFromFile();
    if (BSISUCCESS != status)
        return 0;

    return pCreatorWorker.get();
    }

IScalableMeshSourceCreatorWorker::IScalableMeshSourceCreatorWorker(Impl* implP)
    : IScalableMeshSourceCreator(implP)
    {
    }

IScalableMeshSourceCreatorWorker::~IScalableMeshSourceCreatorWorker()
    {  
    //Since ScalableMeshCreator::~Impl is implemented in another DLL and its implementation is hidden the code below is require to ensure that the destructors of the 
    //Impl classes inheriting from ScalableMeshCreator::Impl are called.
    if (m_implP.get() != nullptr)
        { 
        IScalableMeshSourceCreatorWorker::Impl* impl = (IScalableMeshSourceCreatorWorker::Impl*)m_implP.release();
        delete impl;
        }
    }

IScalableMeshSourceCreatorWorker::Impl::Impl(const WChar* scmFileName, uint32_t nbWorkers)
    : IScalableMeshSourceCreator::Impl(scmFileName) 
    {        
    m_smDb = nullptr;
    m_smSisterDb = nullptr;
    m_nbWorkers = nbWorkers;
    m_lockSleeper = BeDuration::FromSeconds(0.5);

    //Setting meshing and filtering to thread lead to crash/unexpected behavior.
    SetThreadingOptions(false, true, false);
    SetShareable(true);
    ScalableMeshDb::SetEnableSharedDatabase(true);
#ifdef TRACE_ON	    
    CachedDataEventTracer::GetInstance()->setOutputObjLog(false);
    CachedDataEventTracer::GetInstance()->setLogDirectory("D:\\MyDoc\\RMA - July\\CloudWorker\\Log\\");    
    CachedDataEventTracer::GetInstance()->start();
#endif	
    }

IScalableMeshSourceCreatorWorker::Impl::Impl(const IScalableMeshPtr& scmPtr, uint32_t nbWorkers)
    : IScalableMeshSourceCreator::Impl(scmPtr)
    {      
    m_smDb = nullptr;
    m_smSisterDb = nullptr;
    m_nbWorkers = nbWorkers;
    m_lockSleeper = BeDuration::FromSeconds(0.5);

    //Setting meshing and filtering to thread lead to crash/unexpected behavior.
    SetThreadingOptions(false, true, false);
    SetShareable(true);
    ScalableMeshDb::SetEnableSharedDatabase(true);
#ifdef TRACE_ON		    
    CachedDataEventTracer::GetInstance()->setOutputObjLog(false);
    CachedDataEventTracer::GetInstance()->setLogDirectory("D:\\MyDoc\\RMA - July\\CloudWorker\\Log\\");    
    CachedDataEventTracer::GetInstance()->start();
#endif	
    }

IScalableMeshSourceCreatorWorker::Impl::~Impl()
    {    
    m_pDataIndex = nullptr;
    
#ifdef TRACE_ON			
    CachedDataEventTracer::GetInstance()->analyze(::_getpid());
#endif	
    }
   
HFCPtr<MeshIndexType> IScalableMeshSourceCreatorWorker::Impl::GetDataIndex()
    {
    if (m_pDataIndex.GetPtr() == nullptr)
        {       
        StatusInt status = IScalableMeshCreator::Impl::CreateDataIndex(m_pDataIndex, true, SM_ONE_SPLIT_THRESHOLD);

        assert(m_pDataIndex.GetPtr() != nullptr);   
        assert(status == SUCCESS);
        assert(dynamic_cast<SMSQLiteStore<PointIndexExtentType>*>(m_pDataIndex->GetDataStore().get()) != nullptr);

        SMSQLiteStore<PointIndexExtentType>* pSqliteStore(static_cast<SMSQLiteStore<PointIndexExtentType>*>(m_pDataIndex->GetDataStore().get()));
        pSqliteStore->SetRemoveTempGenerationFile(false);        
        }

    return m_pDataIndex;
    }


void IScalableMeshSourceCreatorWorker::Impl::GetSisterMainLockFileName(BeFileName& lockFileName) const
    {
    lockFileName = BeFileName(m_scmFileName);
    lockFileName = lockFileName.GetDirectoryName();
    lockFileName.AppendString(L"sisterMainFilesAccess.lock");
    }


void IScalableMeshSourceCreatorWorker::Impl::GetTaskPlanFileName(BeFileName& taskPlanFileName) const
    {
    taskPlanFileName = BeFileName(m_scmFileName);
    taskPlanFileName = taskPlanFileName.GetDirectoryName();
    taskPlanFileName.AppendString(L"Tasks.xml.Plan");
    }

uint32_t IScalableMeshSourceCreatorWorker::Impl::GetNbNodesPerTask(size_t nbNodes) const
    {
    uint32_t nbTasks = m_nbWorkers * TASK_PER_WORKER;
    uint32_t nbNodesPerTask = max(1.0, ceil((double)nbNodes / nbTasks));

    if (nbNodesPerTask == 1)
        {
        nbNodesPerTask = max(1.0, ceil((double)nbNodes / m_nbWorkers));
        }

    return nbNodesPerTask;
    }


struct NodesToGenerate
    {   
    bool              m_isSorted = false;
    //uint64_t          m_levelId;
    bvector<uint64_t> m_nodeIds; //Node ID to mesh and filter.
    bvector<uint64_t> m_nodeStitchIds; //Node ID to stitch. Should be a subset of m_nodeId

    bool FindNode(uint64_t nodeId)
        {
        if (m_isSorted == false)
            {
            std::sort(m_nodeIds.begin(), m_nodeIds.end());
            }  

        return std::binary_search(m_nodeIds.begin(), m_nodeIds.end(), nodeId);
        }



    /*
    void Merge(const NodesToGenerate& nodesToGenerate)
        {        
        m_nodeIds.insert(m_nodeIds.end(), nodesToGenerate.m_nodeIds.begin(), nodesToGenerate.m_nodeIds.end());
        }
        */
    };


uint64_t GetTotalCountWithSubResolutions(const HFCPtr<SMPointIndexNode<DPoint3d, DRange3d>>& currentNode, uint64_t nbResolutions, uint64_t deepestRes)
    {
    uint64_t totalCount = 0; 
    
    uint64_t totalPointsInRelatedLeaves = currentNode->GetCount();

    for (int currentRes = deepestRes; currentRes >= (int)currentNode->GetLevel(); currentRes--)
        {
        totalCount += totalPointsInRelatedLeaves * 1 / std::pow(4, nbResolutions - currentRes - 1);
        }
        
    return totalCount;         
    }


struct GenerationTask;

typedef RefCountedPtr<GenerationTask> GenerationTaskPtr;

struct GenerationTask : public RefCountedBase
    {
    GenerationTask(bvector<GenerationTaskPtr>& currentTasks, int nbResolutions, IScalableMeshNodePtr& rootNode)
        {             
        m_orderId = 0;
        m_totalNbPoints = 0;
        m_groupRootNode = rootNode;
        m_resolutionToGenerate.resize(nbResolutions);

        bvector<IScalableMeshNodePtr> groupNodes;

        AccumulateNodes(groupNodes, currentTasks, m_groupRootNode);

        for (auto& node : groupNodes)
            {
            AddNode(node);
            //m_resolutionToGenerate[node->GetLevel()].m_nodeIds.push_back(node->GetNodeId());
            }

        for (auto& node : groupNodes)
            {
            bool isNeighborFound = true; 

            for (char relativePosX = -1; relativePosX <= 1 && isNeighborFound; relativePosX++)
                for (char relativePosY = -1; relativePosY <= 1 && isNeighborFound; relativePosY++)
                    for (char relativePosZ = -1; relativePosZ <= 1 && isNeighborFound; relativePosZ++)
                        {
                        bvector<IScalableMeshNodePtr> neighborNodes = node->GetNeighborAt(relativePosX, relativePosY, relativePosZ);
                        for (auto& neighborNode : neighborNodes)
                            {
                            if (!m_resolutionToGenerate[node->GetLevel()].FindNode(neighborNode->GetNodeId()))
                                isNeighborFound = false;
                            }
                        }
                        
            if (isNeighborFound)
                m_resolutionToGenerate[node->GetLevel()].m_nodeStitchIds.push_back(node->GetNodeId());
            }
        }

    void AccumulateNodes(bvector<IScalableMeshNodePtr>& groupNodes, bvector<GenerationTaskPtr>& currentTasks, IScalableMeshNodePtr& currentNode)
        {        
        for (auto& task : currentTasks)
            {
            if (task->m_groupRootNode->GetNodeId() == currentNode->GetNodeId())
                {                
                m_orderId = max(m_orderId, task->m_orderId + 1);
                return;
                }
            }


        ScalableMeshNode<DPoint3d>* smNode(dynamic_cast<ScalableMeshNode<DPoint3d>*>(currentNode.get()));

        if (smNode->GetNodePtr()->GetCount() == 0)
            return;

        groupNodes.push_back(currentNode);
        
        bvector<IScalableMeshNodePtr> childrenNodes(currentNode->GetChildrenNodes());

        for (auto& childNode : childrenNodes)
            {            
            AccumulateNodes(groupNodes, currentTasks, childNode);
            }
        }

    /*
    void SetGroupRootNode(int nbResolutions, bvector<IScalableMeshNodePtr>& groupRootNode)
        {
        m_groupRootNode = groupRootNode;
        m_resolutionToGenerate.resize(nbResolutions);
        AccumulateNodes(m_groupRootNode);            
        }
        */
    
    void AddNode(IScalableMeshNodePtr& currentNode)
        {                
        ScalableMeshNode<DPoint3d>* smNode(dynamic_cast<ScalableMeshNode<DPoint3d>*>(currentNode.get()));

        m_resolutionToGenerate[currentNode->GetLevel()].m_nodeIds.push_back(currentNode->GetNodeId());
        //m_totalNbPoints += currentNode->GetPointCount();
        m_totalNbPoints += GetTotalCountWithSubResolutions(smNode->GetNodePtr(), m_resolutionToGenerate.size(), currentNode->GetLevel());
        }    

    /*
    void MergeGenerationTask(GenerationTask& generationTask)
        {
        assert(generationTask.m_resolutionToGenerate.size() == m_resolutionToGenerate.size());

        for (uint32_t levelInd = 0; levelInd < m_resolutionToGenerate.size(); levelInd++)
            {
            m_resolutionToGenerate[levelInd].Merge(generationTask.m_resolutionToGenerate[levelInd]);
            }          

        m_totalNbPoints += generationTask.m_totalNbPoints;
        }
        */
    
    IScalableMeshNodePtr     m_groupRootNode;
    bvector<NodesToGenerate> m_resolutionToGenerate;
    uint32_t                 m_orderId;
    uint64_t                 m_totalNbPoints;
    };

void CreateGenerationTask(bvector<GenerationTaskPtr>& toExecuteTasks, IScalableMeshNodePtr& groupRootNode, int nbResolutions)
    {           
    GenerationTaskPtr newGenerationTask = new GenerationTask(toExecuteTasks, nbResolutions, groupRootNode);    
    
    if (newGenerationTask->m_totalNbPoints > 0)    
        toExecuteTasks.push_back(newGenerationTask);
#ifndef NDEBUG
    else
        {
        for (auto& nodesToGenerate : newGenerationTask->m_resolutionToGenerate)
            {                                    
            assert(nodesToGenerate.m_nodeIds.size() == 0);
            }
        }
#endif
    }

void GroupNodes(bvector<GenerationTaskPtr>& toExecuteTasks, IScalableMeshNodePtr& currentNode, uint64_t pointThreshold, int& childrenGroupingSize, int nbResolutions)
    {    
    ScalableMeshNode<DPoint3d>* smNode(dynamic_cast<ScalableMeshNode<DPoint3d>*>(currentNode.get()));
    assert(smNode != nullptr);

    assert(childrenGroupingSize == 0);

    //if (smNode->GetNodePtr()->GetCount() < pointThreshold)
    if (GetTotalCountWithSubResolutions(smNode->GetNodePtr(), nbResolutions, nbResolutions - 1) < pointThreshold)
        {        
        CreateGenerationTask(toExecuteTasks, currentNode, nbResolutions);

        childrenGroupingSize = smNode->GetNodePtr()->GetCount();
        return;
        }

    bvector<IScalableMeshNodePtr> childrenNodes(currentNode->GetChildrenNodes());
    
    if (childrenNodes.size() > 0)
        {                
        for (auto& node : childrenNodes)
            {
            int childGroupingSize = 0;
            GroupNodes(toExecuteTasks, node, pointThreshold, childGroupingSize, nbResolutions);
            childrenGroupingSize += childGroupingSize;            
            }        
        }

    const HFCPtr<SMPointIndexNode<DPoint3d, DRange3d>>& parentNode(smNode->GetNodePtr()->GetParentNode());

    if (parentNode == nullptr)
        {
        //Create final group
        CreateGenerationTask(toExecuteTasks, currentNode, nbResolutions);
        //childrenGroupingSize = smNode->GetNodePtr()->GetCount();
        childrenGroupingSize = GetTotalCountWithSubResolutions(smNode->GetNodePtr(), nbResolutions, nbResolutions - 1);
        return;
        }
        
    uint64_t totalCountCurrentNode = GetTotalCountWithSubResolutions(smNode->GetNodePtr(), nbResolutions, nbResolutions - 1);

    if (totalCountCurrentNode - childrenGroupingSize > pointThreshold)
        {        
        //Will lead to group bigger than threshold but preferable than creating a group for each children, which can results in smaller group.
        assert(totalCountCurrentNode - childrenGroupingSize <= pointThreshold * 4);

        CreateGenerationTask(toExecuteTasks, currentNode, nbResolutions);        
        childrenGroupingSize = totalCountCurrentNode;
        return;
        }


    /*
    if (currentNode->GetCount() - childrenGroupingSize > pointThreshold)
        {        
        assert(childrenNodes.size() > 0);
        childrenGroupingSize = 0;  


        for (auto& node : childrenNodes)
            {        
            CreateGenerationTask(toExecuteTasks, node, nbResolutions);            
            childrenGroupingSize += GetTotalCountWithSubResolutions(smNode->GetNodePtr(), nbResolutions, nbResolutions - 1);
            }     

        return;
        }
        */



    return;
    }


void IScalableMeshSourceCreatorWorker::Impl::GetGenerationTasks(bvector<GenerationTaskPtr>& toExecuteTasks, uint32_t maxGroupSize)
    {   
    HFCPtr<MeshIndexType> pDataIndex(GetDataIndex());

    HFCPtr<SMPointIndexNode<DPoint3d, DRange3d>> rootNode(pDataIndex->GetRootNode());
    
    IScalableMeshNodePtr meshRootNode(new ScalableMeshNode<DPoint3d>(rootNode));
                
    int childrenGroupingSize = 0;     
    int nbResolutions = (int)(pDataIndex->GetDepth() + 1);

    GroupNodes(toExecuteTasks, meshRootNode, maxGroupSize, childrenGroupingSize, nbResolutions);
    }

StatusInt IScalableMeshSourceCreatorWorker::Impl::CreateGenerationTasks(uint32_t maxGroupSize, const WString& jobName, const BeFileName& smFileName)
    {
    BeFileName taskDirectory(m_scmFileName);

    taskDirectory = taskDirectory.GetDirectoryName();

    HFCPtr<MeshIndexType> pDataIndex(GetDataIndex());

    bvector<uint64_t> nodesToMesh;
    
    bvector<GenerationTaskPtr> generationTasks;

    //Explicitly open the sql file to improve speed when 3SM is located on network drive
    StatusInt status = OpenSqlFiles(true, false);
    assert(status == SUCCESS);

    GetGenerationTasks(generationTasks, maxGroupSize);
        
    CloseSqlFiles();

    uint64_t totalNodes = 0;
    uint64_t totalStichableNodes = 0;

    for (auto& generationTask : generationTasks)
        {
        for (auto& resTask : generationTask->m_resolutionToGenerate)
            {
            totalNodes += resTask.m_nodeIds.size();
            totalStichableNodes += resTask.m_nodeStitchIds.size();
            }
        }         

    wchar_t stringBuffer[100000];
    
    for (size_t ind = 0; ind < generationTasks.size(); ind++)    
        {        
        if (generationTasks[ind]->m_orderId > 0)
            continue;

        BeFileName meshTaskFile(taskDirectory);

        swprintf(stringBuffer, L"Generate%zi.xml", ind);
        meshTaskFile.AppendString(stringBuffer);
        BeXmlDomPtr xmlDomPtr(BeXmlDom::CreateEmpty());
        
        BeXmlNodeP workerNode(xmlDomPtr->AddNewElement("workerTask", nullptr, nullptr));
        workerNode->AddAttributeStringValue("type", "generate");                
        workerNode->AddAttributeStringValue("jobName", jobName.c_str());
        workerNode->AddAttributeStringValue("smName", smFileName.c_str());

        /*
         IScalableMeshNodePtr     m_groupRootNode;
    bvector<NodesToGenerate> m_resolutionToGenerate;
    uint32_t                 m_orderId;
    uint64_t                 m_totalNbPoints;
    */

                            
/*
        bvector<uint64_t> m_nodeIds; //Node ID to mesh and filter.
    bvector<uint64_t> m_nodeStitchIds; //Node ID to stitch. Should be a subset of m_nodeId
    */

        for (int resInd = (int)generationTasks[ind]->m_resolutionToGenerate.size() - 1; resInd >= 0; resInd--)
            {            
            assert(generationTasks[ind]->m_resolutionToGenerate[resInd].m_nodeStitchIds.size() == 0 || generationTasks[ind]->m_resolutionToGenerate[resInd].m_nodeIds.size() != 0);

            if (generationTasks[ind]->m_resolutionToGenerate[resInd].m_nodeIds.size() > 0)
                {
                BeXmlNodeP tileNode(xmlDomPtr->AddNewElement("MeshTiles", nullptr, workerNode));
                WString tileList; 

                bool needFiltering = false;

                if (resInd != (int)generationTasks[ind]->m_resolutionToGenerate.size() - 1)
                    needFiltering = true;

                tileNode->AddAttributeBooleanValue("needFiltering", needFiltering);

                for (auto& nodeId : generationTasks[ind]->m_resolutionToGenerate[resInd].m_nodeIds)
                    {
                    WPrintfString tileId(L"%u,", nodeId);
                    tileList.append(tileId);                    
                    }

                tileNode->AddAttributeStringValue("ids", tileList.c_str());

                if (generationTasks[ind]->m_resolutionToGenerate[resInd].m_nodeStitchIds.size() > 0)
                    {
                    //BeXmlNodeP tileNode(xmlDomPtr->AddNewElement("StitchTiles", nullptr, workerNode));
                    WString tileList; 

                    for (auto& nodeId : generationTasks[ind]->m_resolutionToGenerate[resInd].m_nodeStitchIds)
                        {
                        WPrintfString tileId(L"%u,", nodeId);
                        tileList.append(tileId);                    
                        }

                    tileNode->AddAttributeStringValue("stitchIds", tileList.c_str());
                    }  
                }

#if 0 
            if (generationTasks[ind]->m_resolutionToGenerate[resInd].m_nodeStitchIds.size() > 0)
                {
                BeXmlNodeP tileNode(xmlDomPtr->AddNewElement("StitchTiles", nullptr, workerNode));
                WString tileList; 

                for (auto& nodeId : generationTasks[ind]->m_resolutionToGenerate[resInd].m_nodeStitchIds)
                    {
                    WPrintfString tileId(L"%u,", nodeId);
                    tileList.append(tileId);                    
                    }

                tileNode->AddAttributeStringValue("ids", tileList.c_str());
                }                                                                                    
#endif
            }

        BeXmlDom::ToStringOption toStrOption = (BeXmlDom::ToStringOption)(BeXmlDom::TO_STRING_OPTION_Formatted | BeXmlDom::TO_STRING_OPTION_Indent);
                    
        BeXmlStatus status = xmlDomPtr->ToFile(meshTaskFile, toStrOption, BeXmlDom::FILE_ENCODING_Utf8);
        assert(status == BEXML_Success);
        }

#if 0
    wchar_t stringBuffer[100000];

    pDataIndex->Mesh(&nodesToMesh);

    uint32_t nbNodesPerTask = GetNbNodesPerTask(nodesToMesh.size());

    for (size_t ind = 0; ind < nodesToMesh.size();)
            {
        BeFileName meshTaskFile(taskDirectory);

        swprintf(stringBuffer, L"Mesh%zi.xml", ind);
        meshTaskFile.AppendString(stringBuffer);
        BeXmlDomPtr xmlDomPtr(BeXmlDom::CreateEmpty());

        BeXmlNodeP workerNode(xmlDomPtr->AddNewElement("workerTask", nullptr, nullptr));
        workerNode->AddAttributeStringValue("type", "mesh");

        for (size_t nodeInd = 0; nodeInd < nbNodesPerTask; nodeInd++)
            {
            BeXmlNodeP tileNode(xmlDomPtr->AddNewElement("tile", nullptr, workerNode));
            tileNode->AddAttributeUInt64Value("id", nodesToMesh[ind]);
            ind++;

            if (ind >= nodesToMesh.size())
                break;
            

        BeXmlDom::ToStringOption toStrOption = (BeXmlDom::ToStringOption)(BeXmlDom::TO_STRING_OPTION_Formatted | BeXmlDom::TO_STRING_OPTION_Indent);

        BeXmlStatus status = xmlDomPtr->ToFile(meshTaskFile, toStrOption, BeXmlDom::FILE_ENCODING_Utf8);
        assert(status == BEXML_Success);
        }
#endif

    return SUCCESS;
    }


StatusInt IScalableMeshSourceCreatorWorker::Impl::CreateMeshTasks()
    {
    BeFileName taskDirectory(m_scmFileName);

    taskDirectory = taskDirectory.GetDirectoryName();

    HFCPtr<MeshIndexType> pDataIndex(GetDataIndex());                

    bvector<uint64_t> nodesToMesh;

    wchar_t stringBuffer[100000];
        
    pDataIndex->Mesh(&nodesToMesh);    

    uint32_t nbNodesPerTask = GetNbNodesPerTask(nodesToMesh.size());
        
    for (size_t ind = 0; ind < nodesToMesh.size();)
        {         
        BeFileName meshTaskFile(taskDirectory);

        swprintf(stringBuffer, L"Mesh%zi.xml", ind);
        meshTaskFile.AppendString(stringBuffer);
        BeXmlDomPtr xmlDomPtr(BeXmlDom::CreateEmpty());

        BeXmlNodeP workerNode(xmlDomPtr->AddNewElement("workerTask", nullptr, nullptr));
        workerNode->AddAttributeStringValue("type", "mesh");

        for (size_t nodeInd  = 0; nodeInd < nbNodesPerTask; nodeInd++)
            {           
            BeXmlNodeP tileNode(xmlDomPtr->AddNewElement("tile", nullptr, workerNode));
            tileNode->AddAttributeUInt64Value("id", nodesToMesh[ind]);
            ind++;

            if (ind >= nodesToMesh.size())
                break;
            }                        

        BeXmlDom::ToStringOption toStrOption = (BeXmlDom::ToStringOption)(BeXmlDom::TO_STRING_OPTION_Formatted | BeXmlDom::TO_STRING_OPTION_Indent);
                    
        BeXmlStatus status = xmlDomPtr->ToFile(meshTaskFile, toStrOption, BeXmlDom::FILE_ENCODING_Utf8);
        assert(status == BEXML_Success);
        }
            
    return SUCCESS;
    }


StatusInt IScalableMeshSourceCreatorWorker::Impl::CreateStitchTasks(uint32_t resolutionInd)
    {    
    BeFileName taskDirectory(m_scmFileName);

    taskDirectory = taskDirectory.GetDirectoryName();

    HFCPtr<MeshIndexType> pDataIndex(GetDataIndex());
    
    wchar_t stringBuffer[100000];

    vector<SMMeshIndexNode<DPoint3d, DRange3d>*> nodesToStitchInfo;

    pDataIndex->SetForceReload(true);

    pDataIndex->Stitch(resolutionInd, false, &nodesToStitchInfo);

    pDataIndex->SetForceReload(false);

    uint32_t nbNodesPerTask = GetNbNodesPerTask(nodesToStitchInfo.size());

    for (size_t ind = 0; ind < nodesToStitchInfo.size();)
        {        
        BeFileName meshTaskFile(taskDirectory);
        
        swprintf(stringBuffer, L"Stitch%zi.xml", ind);
        meshTaskFile.AppendString(stringBuffer);

        BeXmlDomPtr xmlDomPtr(BeXmlDom::CreateEmpty());
        BeXmlNodeP workerNode(xmlDomPtr->AddNewElement("workerTask", nullptr, nullptr));
        workerNode->AddAttributeStringValue("type", "stitch");
        
        for (size_t nodeInd = 0; nodeInd < nbNodesPerTask; nodeInd++)
            {
            uint64_t nodeId = nodesToStitchInfo[ind]->GetBlockID().m_integerID;

            BeXmlNodeP tileNode(xmlDomPtr->AddNewElement("tile", nullptr, workerNode));
            tileNode->AddAttributeUInt64Value("id", nodeId);
            ind++;

            if (ind >= nodesToStitchInfo.size())
                break;
            }

        BeXmlDom::ToStringOption toStrOption = (BeXmlDom::ToStringOption)(BeXmlDom::TO_STRING_OPTION_Formatted | BeXmlDom::TO_STRING_OPTION_Indent);

        BeXmlStatus status = xmlDomPtr->ToFile(meshTaskFile, toStrOption, BeXmlDom::FILE_ENCODING_Utf8);
        assert(status == BEXML_Success);
        }

    return SUCCESS;
    }



StatusInt IScalableMeshSourceCreatorWorker::Impl::CreateFilterTasks(uint32_t resolutionInd)
    {
    BeFileName taskDirectory(m_scmFileName);

    taskDirectory = taskDirectory.GetDirectoryName();

    HFCPtr<MeshIndexType> pDataIndex(GetDataIndex());

    bvector<SMPointIndexNode<DPoint3d, DRange3d>*> nodesToFilter;

    wchar_t stringBuffer[100000];    

    pDataIndex->Filter(resolutionInd, &nodesToFilter);

    uint32_t nbNodesPerTask = GetNbNodesPerTask(nodesToFilter.size());

    for (size_t ind = 0; ind < nodesToFilter.size();)
        {
        BeFileName meshTaskFile(taskDirectory);

        uint64_t nodeId = nodesToFilter[ind]->GetBlockID().m_integerID;

        swprintf(stringBuffer, L"Filter%zi.xml", nodeId);
        meshTaskFile.AppendString(stringBuffer);

        BeXmlDomPtr xmlDomPtr(BeXmlDom::CreateEmpty());
        BeXmlNodeP workerNode(xmlDomPtr->AddNewElement("workerTask", nullptr, nullptr));
        workerNode->AddAttributeStringValue("type", "filter");

        for (size_t nodeInd = 0; nodeInd < nbNodesPerTask; nodeInd++)
            {
            uint64_t nodeId = nodesToFilter[ind]->GetBlockID().m_integerID;

            BeXmlNodeP tileNode(xmlDomPtr->AddNewElement("tile", nullptr, workerNode));
            tileNode->AddAttributeUInt64Value("id", nodeId);
            ind++;

            if (ind >= nodesToFilter.size())
                break;
            }
    
        BeXmlDom::ToStringOption toStrOption = (BeXmlDom::ToStringOption)(BeXmlDom::TO_STRING_OPTION_Formatted | BeXmlDom::TO_STRING_OPTION_Indent);

        BeXmlStatus status = xmlDomPtr->ToFile(meshTaskFile, toStrOption, BeXmlDom::FILE_ENCODING_Utf8);
        assert(status == BEXML_Success);
        }

   // pDataIndex->ClearNodeMap();

    return SUCCESS;
    }


StatusInt IScalableMeshSourceCreatorWorker::Impl::ProcessGenerateTask(BeXmlNodeP pXmlTaskNode)
    {
    BeXmlNodeP pChildNode = pXmlTaskNode->GetFirstChild();

    HFCPtr<MeshIndexType> pDataIndex(GetDataIndex());

    ScalableMeshQuadTreeBCLIBMeshFilter1<DPoint3d, DRange3d>* filter = dynamic_cast<ScalableMeshQuadTreeBCLIBMeshFilter1<DPoint3d, DRange3d>*>(pDataIndex->GetFilter());
    if (filter != nullptr)
        {
        filter->SetIsMultiProcessGeneration(true);
        }
        
    bvector<HFCPtr<SMMeshIndexNode<DPoint3d, DRange3d>>>     generatedNodes;
    bvector<RefCountedPtr<SMMemoryPoolVectorItem<DPoint3d>>> generatedPtsNeighbors;

    bvector<HFCPtr<SMMeshIndexNode<DPoint3d, DRange3d>>>     nodesToMesh;
    bvector<RefCountedPtr<SMMemoryPoolVectorItem<DPoint3d>>> ptsNeighbors;

    SMMeshDataToLoad meshDataToLoad;

    meshDataToLoad.m_features = true;
    meshDataToLoad.m_graph = true;
            
    do
        {
        assert(pChildNode != nullptr);
        assert(Utf8String(pChildNode->GetName()).CompareTo("MeshTiles") == 0);
        
        bvector<uint64_t> tileIds;
        bvector<uint64_t> stitchTileIds;
        WString           attrStr;

        BeXmlStatus xmlStatus = pChildNode->GetAttributeStringValue(attrStr, "ids");
        assert(xmlStatus == BEXML_Success);
        //assert(pChildNode->GetNextSibling() == nullptr);

        //TRACEPOINT(THREAD_ID(), EventType::WORKER_MESH_TASK, tileId, (uint64_t)-1, -1, -1, 0, 0)

        bvector<WString> idAttrs;
        BeStringUtilities::ParseArguments(idAttrs, attrStr.c_str(), L",");

        for (auto& idStr : idAttrs)
            {
            tileIds.push_back(BeStringUtilities::ParseUInt64(Utf8String(idStr).c_str()));                
            }

        assert(tileIds.size() > 0);

        xmlStatus = pChildNode->GetAttributeStringValue(attrStr, "stitchIds");
        assert(xmlStatus == BEXML_Success || xmlStatus == BEXML_AttributeNotFound);

        if (xmlStatus == BEXML_Success)
            {        
            //assert(pChildNode->GetNextSibling() == nullptr);

            //TRACEPOINT(THREAD_ID(), EventType::WORKER_MESH_TASK, tileId, (uint64_t)-1, -1, -1, 0, 0)

            idAttrs.clear();
            BeStringUtilities::ParseArguments(idAttrs, attrStr.c_str(), L",");

            for (auto& idStr : idAttrs)
                {
                stitchTileIds.push_back(BeStringUtilities::ParseUInt64(Utf8String(idStr).c_str()));                
                }
            }

        bool needFiltering; 

        xmlStatus = pChildNode->GetAttributeBooleanValue(needFiltering, "needFiltering");
        assert(xmlStatus == BEXML_Success);

        //Load all the nodes created during the indexing step. 
        
        //TBD_G : Need lock file?        
        StatusInt status = OpenSqlFiles(true, true);
        assert(status == SUCCESS);

        bool dbOpResult = m_smDb->StartTransaction();
        assert(dbOpResult == true);

        dbOpResult = m_smSisterDb->StartTransaction();
        assert(dbOpResult == true);
		
        for (auto& tileId : tileIds)
            {            
            HPMBlockID blockID(tileId);

            HFCPtr<SMMeshIndexNode<DPoint3d, DRange3d>> meshNode((SMMeshIndexNode<DPoint3d, DRange3d>*)pDataIndex->CreateNewNode(blockID, false, true).GetPtr());

            if (std::find(stitchTileIds.begin(), stitchTileIds.end(), tileId) != stitchTileIds.end())    
                meshNode->NeedToLoadNeighbors(true);
            else
                meshNode->NeedToLoadNeighbors(false);
        
            meshNode->Load();
            meshNode->LoadData(&meshDataToLoad);
            ptsNeighbors.push_back(meshNode->GetPointsPtr());
                        
            nodesToMesh.push_back(meshNode);            
            }
        
        dbOpResult = m_smDb->CommitTransaction();
        assert(dbOpResult == true);    
        
        dbOpResult = m_smSisterDb->CommitTransaction();
        assert(dbOpResult == true);

        CloseSqlFiles();

        for (auto& node : nodesToMesh)
            {
            vector<HFCPtr<SMPointIndexNode<DPoint3d, DRange3d>>> subNodes(node->GetSubNodes());
            

            for (auto& subNode : subNodes)
                {         
                bool found = false;

                if (subNode.GetPtr() == nullptr)
                    {
                    found = true;
                    }
                else
                    {
                    for (auto& generatedNode : generatedNodes)
                        {
                        if (generatedNode.GetPtr() == subNode.GetPtr())
                            found = true;
                        }    
                    }

                if (!found)
                    {
                    found = found;
                    }
                }
            }
            
                    
        if (needFiltering)
            {
            for (auto& node : nodesToMesh)
                {                  
                node->Filter((int)node->GetLevel(), nullptr);                
                node->SetDirty(true);
                }
            }
            

        //Mesh tiles
        for (auto& node : nodesToMesh)
            {
            assert(!node->m_nodeHeader.m_arePoints3d);

            bool isMeshed = pDataIndex->GetMesher2_5d()->Mesh(node);

            if (isMeshed)
                {
                node->SetDirty(true);
                }
            }

        //Stitch tiles
        for (auto& tileId : stitchTileIds)
            {
            for (auto& node : nodesToMesh)
                {
                assert(!node->m_nodeHeader.m_arePoints3d);            

                if (node->GetBlockID().m_integerID == tileId)
                    {        
                    bool isStitched = pDataIndex->GetMesher2_5d()->Stitch(node);
                
                    if (isStitched)
                        {
                        node->SetDirty(true);
                        }

                    break;
                    }
                }
            }

        generatedNodes.insert(generatedNodes.end(), nodesToMesh.begin(), nodesToMesh.end());
        generatedPtsNeighbors.insert(generatedPtsNeighbors.end(), ptsNeighbors.begin(), ptsNeighbors.end());

        nodesToMesh.clear();
        ptsNeighbors.clear();

                 
        pChildNode = pChildNode->GetNextSibling();
        } while (pChildNode != nullptr);
            
    //Flush all the data on disk    
    OpenSqlFiles(false, true);

    generatedPtsNeighbors.clear();

    for (auto& node : generatedNodes)
        {
        node->Discard();
        // pDataIndex->ClearNodeMap();

        node->Unload();
        }

    generatedNodes.clear();

    pDataIndex->Store();            

    CloseSqlFiles();

#if 0 
    //The save and save sister files are closing the db, which can create deadlock. Add sisterMain lock to prevent this for happening.
    FILE* lockFile;
    BeFileName lockFileName;
    GetSisterMainLockFileName(lockFileName);

    while ((lockFile = _wfsopen(lockFileName, L"ab+", _SH_DENYRW)) == nullptr)
        {
        m_lockSleeper.Sleep();
        }

    m_smSQLitePtr->Save();

    SMSQLiteStore<PointIndexExtentType>* pSqliteStore(static_cast<SMSQLiteStore<PointIndexExtentType>*>(m_pDataIndex->GetDataStore().get()));
    assert(pSqliteStore != nullptr);
    pSqliteStore->SaveSisterFiles();
    
    fclose(lockFile);
#endif

    assert(SMMemoryPool::GetInstance()->GetCurrentlyUsed() == 0);
        
    return SUCCESS;
    }


StatusInt IScalableMeshSourceCreatorWorker::Impl::ProcessMeshTask(BeXmlNodeP pXmlTaskNode)
    {
    BeXmlNodeP pChildNode = pXmlTaskNode->GetFirstChild();

    HFCPtr<MeshIndexType> pDataIndex(GetDataIndex());

    bvector<HFCPtr<SMMeshIndexNode<DPoint3d, DRange3d>>> nodesToMesh;


    bvector<RefCountedPtr<SMMemoryPoolVectorItem<DPoint3d>>>      ptsNeighbors;
    //bvector<RefCountedPtr<SMMemoryPoolVectorItem<int32_t>>>       ptsIndicesNeighbors;
    //bvector<RefCountedPtr<SMMemoryPoolGenericBlobItem<MTGGraph>>> graphNeighbors;



    OpenSqlFiles(true, true);

    do
        {
        assert(pChildNode != nullptr);
        assert(Utf8String(pChildNode->GetName()).CompareTo("tile") == 0);

        uint64_t tileId;

        BeXmlStatus xmlStatus = pChildNode->GetAttributeUInt64Value(tileId, "id");
        assert(xmlStatus == BEXML_Success);
        //assert(pChildNode->GetNextSibling() == nullptr);

        TRACEPOINT(THREAD_ID(), EventType::WORKER_MESH_TASK, tileId, (uint64_t)-1, -1, -1, 0, 0)

        HPMBlockID blockID(tileId);

        HFCPtr<SMMeshIndexNode<DPoint3d, DRange3d>> meshNode((SMMeshIndexNode<DPoint3d, DRange3d>*)pDataIndex->CreateNewNode(blockID, false, true).GetPtr());

        meshNode->NeedToLoadNeighbors(false);
        meshNode->Load();

        ptsNeighbors.push_back(meshNode->GetPointsPtr());
        nodesToMesh.push_back(meshNode);
        /*
        ptsIndicesNeighbors.push_back(pMeshIndexNode->GetPtsIndicePtr());
        graphNeighbors.push_back(pMeshIndexNode->GetGraphPtr());

        Get
        */
        
        pChildNode = pChildNode->GetNextSibling();
        } while (pChildNode != nullptr);

    CloseSqlFiles();


    for (auto& node : nodesToMesh)
        {
        assert(!node->m_nodeHeader.m_arePoints3d);

        bool isMeshed = pDataIndex->GetMesher2_5d()->Mesh(node);

        if (isMeshed)
            {
            node->SetDirty(true);
            }
        }


    OpenSqlFiles(false, true);

    ptsNeighbors.clear();

    for (auto& node : nodesToMesh)
        {
        node->Discard();
        // pDataIndex->ClearNodeMap();

        node->Unload();
        }

    nodesToMesh.clear();

    pDataIndex->Store();

    CloseSqlFiles();

    FILE* lockFile;
    BeFileName lockFileName;
    GetSisterMainLockFileName(lockFileName);

    while ((lockFile = _wfsopen(lockFileName, L"ab+", _SH_DENYRW)) == nullptr)
        {
        m_lockSleeper.Sleep();
        }

    m_smSQLitePtr->Save();

    SMSQLiteStore<PointIndexExtentType>* pSqliteStore(static_cast<SMSQLiteStore<PointIndexExtentType>*>(m_pDataIndex->GetDataStore().get()));
    assert(pSqliteStore != nullptr);
    pSqliteStore->SaveSisterFiles();

    fclose(lockFile);

    
    
    return SUCCESS;
    }


#if 0 
StatusInt IScalableMeshSourceCreatorWorker::Impl::ProcessMeshTask(BeXmlNodeP pXmlTaskNode)
    {
    BeXmlNodeP pChildNode = pXmlTaskNode->GetFirstChild();

    HFCPtr<MeshIndexType> pDataIndex(GetDataIndex());

    bvector<HFCPtr<SMMeshIndexNode<DPoint3d, DRange3d>>> nodesToMesh;

    do
        {
        assert(pChildNode != nullptr);
        assert(Utf8String(pChildNode->GetName()).CompareTo("tile") == 0);

        uint64_t tileId;

        BeXmlStatus xmlStatus = pChildNode->GetAttributeUInt64Value(tileId, "id");
        assert(xmlStatus == BEXML_Success);
        //assert(pChildNode->GetNextSibling() == nullptr);

        TRACEPOINT(THREAD_ID(), EventType::WORKER_MESH_TASK, tileId, (uint64_t)-1, -1, -1, 0, 0)
       
        HPMBlockID blockID(tileId);

        HFCPtr<SMMeshIndexNode<DPoint3d, DRange3d>> meshNode((SMMeshIndexNode<DPoint3d, DRange3d>*)pDataIndex->CreateNewNode(blockID, false, true).GetPtr());

        meshNode->NeedToLoadNeighbors(false);
        meshNode->Load();
        }

        assert(!meshNode->m_nodeHeader.m_arePoints3d);

        bool isMeshed = pDataIndex->GetMesher2_5d()->Mesh(meshNode);

        if (isMeshed)
            {
            meshNode->SetDirty(true);
            }

        meshNode->Discard();
        // pDataIndex->ClearNodeMap();

        meshNode->Unload();
        meshNode = nullptr;

        pChildNode = pChildNode->GetNextSibling();
        } while (pChildNode != nullptr);

    pDataIndex->Store();
    m_smSQLitePtr->Save();

    SMSQLiteStore<PointIndexExtentType>* pSqliteStore(static_cast<SMSQLiteStore<PointIndexExtentType>*>(m_pDataIndex->GetDataStore().get()));
    assert(pSqliteStore != nullptr);
    pSqliteStore->SaveSisterFiles();

    return SUCCESS;    
    }

#endif

StatusInt IScalableMeshSourceCreatorWorker::Impl::OpenSqlFiles(bool readOnly, bool needSisterMainLockFile)
    {            
    if (m_smDb == nullptr || m_smSisterDb == nullptr)
        {
        SMSQLiteStore<PointIndexExtentType>* pSqliteStore(static_cast<SMSQLiteStore<PointIndexExtentType>*>(m_pDataIndex->GetDataStore().get()));
        assert(pSqliteStore != nullptr);
        
        m_mainFilePtr = GetFile(true);

        if (m_smDb == nullptr)
            {
            m_smDb = m_mainFilePtr->GetDb();
            assert(m_smDb != nullptr);
            }
        
        if (m_smSisterDb == nullptr)
            {
            m_sisterFilePtr = pSqliteStore->GetSisterSQLiteFile(SMStoreDataType::LinearFeature, false, true);

            m_smSisterDb = m_sisterFilePtr->GetDb();
            assert(m_smSisterDb != nullptr);
            }
        }
    
    FILE* lockFile = nullptr;

    BeFileName lockFileName;

    if (needSisterMainLockFile)
        {
        GetSisterMainLockFileName(lockFileName);

        while ((lockFile = _wfsopen(lockFileName, L"ab+", _SH_DENYRW)) == nullptr)
            {
            m_lockSleeper.Sleep();
            }
        }
                
    bool dbOpResult = true;

    if (!m_smDb->IsDbOpen() || !m_smDb->IsReadonly())
        dbOpResult = m_smDb->ReOpenShared(readOnly, true);

    assert(dbOpResult == true);
            
    if (!m_smSisterDb->IsDbOpen() || !m_smSisterDb->IsReadonly())
        dbOpResult = m_smSisterDb->ReOpenShared(readOnly, true);

    assert(dbOpResult == true);
	
/*			
	if (!m_smDb->IsDbOpen() || !m_smDb->IsReadonly())
        {
        while (!m_smDb->ReOpenShared(readOnly, true));    
        }

    assert(dbOpResult == true);
            
    if (!m_smSisterDb->IsDbOpen() || !m_smSisterDb->IsReadonly())
        {
        while (!m_smSisterDb->ReOpenShared(readOnly, true));
        }
		*/


    if (needSisterMainLockFile)
        {
        fclose(lockFile);
        }

    return SUCCESS;
    }

StatusInt IScalableMeshSourceCreatorWorker::Impl::CloseSqlFiles()
    {    
    assert(m_smDb->IsDbOpen());
    assert(m_smSisterDb->IsDbOpen());

    bool wasTransactionAbandoned;
        
    m_smDb->CloseShared(wasTransactionAbandoned);
    assert(wasTransactionAbandoned == false);

    m_smSisterDb->CloseShared(wasTransactionAbandoned);
    assert(wasTransactionAbandoned == false);

    return SUCCESS;
    }

StatusInt IScalableMeshSourceCreatorWorker::Impl::ProcessStitchTask(BeXmlNodeP pXmlTaskNode)
    {
    BeXmlNodeP pChildNode = pXmlTaskNode->GetFirstChild();

    do
        {
        assert(pChildNode != nullptr);
        assert(Utf8String(pChildNode->GetName()).CompareTo("tile") == 0);

        uint64_t tileId;

        BeXmlStatus xmlStatus = pChildNode->GetAttributeUInt64Value(tileId, "id");
        assert(xmlStatus == BEXML_Success);
        
        HFCPtr<MeshIndexType> pDataIndex(GetDataIndex());

        TRACEPOINT(THREAD_ID(), EventType::WORKER_STITCH_TASK, tileId, (uint64_t)-1, -1, -1, 0, 0)
                
        HPMBlockID blockID(tileId);              
 
        HFCPtr<SMMeshIndexNode<DPoint3d, DRange3d>> meshNode((SMMeshIndexNode<DPoint3d, DRange3d>*)pDataIndex->CreateNewNode(blockID, false, true).GetPtr());

        assert(!meshNode->IsDirty());
        meshNode->Unload();
        meshNode->RemoveNonDisplayPoolData();

        /*
        if (!meshNode->IsNeighborsLoaded())
            {
            meshNode->Unload();
            }  
            */

        meshNode->NeedToLoadNeighbors(true);    
        meshNode->Load();
        assert(!meshNode->m_nodeHeader.m_arePoints3d);

        //Ensure that loaded neighbor node and data are in sync with each other.
        StatusInt status = OpenSqlFiles(true, true);
        assert(status == SUCCESS);

        vector<SMPointIndexNode<DPoint3d, DRange3d>*> neighborNodes;

        meshNode->GetAllNeighborNodes(neighborNodes);    
        bool dbOpResult = m_smDb->StartTransaction();
        assert(dbOpResult == true);

        dbOpResult = m_smSisterDb->StartTransaction();
        assert(dbOpResult == true);

        
        bvector<RefCountedPtr<SMMemoryPoolVectorItem<DPoint3d>>>      ptsNeighbors;
        bvector<RefCountedPtr<SMMemoryPoolVectorItem<int32_t>>>       ptsIndicesNeighbors;
        bvector<RefCountedPtr<SMMemoryPoolGenericBlobItem<MTGGraph>>> graphNeighbors;
        
        for (auto node : neighborNodes)
            {                
            TRACEPOINT(THREAD_ID(), EventType::WORKER_STITCH_TASK_NEIGHBOR, node->GetBlockID().m_integerID, (uint64_t)-1, -1, -1, 0, 0)

            assert(!node->IsDirty());
            node->Unload(); 
            node->RemoveNonDisplayPoolData();
            meshNode->NeedToLoadNeighbors(true);
            node->Load();
                
            SMMeshIndexNode<DPoint3d, DRange3d>* pMeshIndexNode(dynamic_cast<SMMeshIndexNode<DPoint3d, DRange3d>*>(node));
            assert(pMeshIndexNode != nullptr);

            RefCountedPtr<SMMemoryPoolVectorItem<DPoint3d>> pointPtr(pMeshIndexNode->GetPointsPtr());
            size_t nbPoints = node->GetNbPoints();

            RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> ptsIndices(pMeshIndexNode->GetPtsIndicePtr());

            if (nbPoints == 0 || ptsIndices->size() == 0)
                {
                int i = 0;
                i = i;
                }
               
            assert(nbPoints == pointPtr->size());
                
            ptsNeighbors.push_back(pMeshIndexNode->GetPointsPtr());
            ptsIndicesNeighbors.push_back(pMeshIndexNode->GetPtsIndicePtr());
            graphNeighbors.push_back(pMeshIndexNode->GetGraphPtr());
            }

        bool isStitched = pDataIndex->GetMesher2_5d()->Stitch(meshNode);
        
        dbOpResult = m_smDb->CommitTransaction();
        assert(dbOpResult == true);    
        
        dbOpResult = m_smSisterDb->CommitTransaction();
        assert(dbOpResult == true);

        CloseSqlFiles();


        //Ensure that loaded neighbor node and data are in sync with each other - END    
        /*
        fclose(lockFile);

        while ((lockFile = _wfsopen(lockFileName, L"ab+", _SH_DENYRW)) == nullptr)
            {
            sleeper.Sleep();
            }
          */          

        status = OpenSqlFiles(false, true);
        assert(status == SUCCESS);            

        ptsNeighbors.clear();
        ptsIndicesNeighbors.clear();
        graphNeighbors.clear();

    /*
    if (!smDb->IsDbOpen() || smDb->IsReadonly())
        {
        if (smDb->IsDbOpen())
            {
            smDb->CloseShared(wasTransactionAbandoned);
            assert(wasTransactionAbandoned == false);
            }

        dbOpResult = smDb->ReOpenShared(false, true);
        }

    if (!smSisterDb->IsDbOpen() || smSisterDb->IsReadonly())
        {
        if (smSisterDb->IsDbOpen())
            {
            smSisterDb->CloseShared(wasTransactionAbandoned);
            assert(wasTransactionAbandoned == false);
            }

        dbOpResult = smSisterDb->ReOpenShared(false, true);
        }
        */

    
        if (isStitched)
            {
            meshNode->SetDirty(true);
            }

        for (auto node : neighborNodes)
            {
            assert(!node->IsDirty());        
            }

        {
            size_t nbPoints = meshNode->GetNbPoints();

            RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> ptsIndices(meshNode->GetPtsIndicePtr());

            if (nbPoints == 0 || ptsIndices->size() == 0)
            {
                int i = 0;
                i = i;
            }
        }

        FILE* lockFile;
        BeFileName lockFileName;
        GetSisterMainLockFileName(lockFileName);
                        
        while ((lockFile = _wfsopen(lockFileName, L"ab+", _SH_DENYRW)) == nullptr)
            {
            m_lockSleeper.Sleep();
            }

        meshNode->Discard();    
        meshNode->Unload();
    
       // pDataIndex->ClearNodeMap();
        pDataIndex->Store();
        m_mainFilePtr->Save();
        m_sisterFilePtr->Save();
  
        //pSqliteStore->SaveSisterFiles();
        
        meshNode = nullptr;
      
        CloseSqlFiles();
    
        fclose(lockFile);

        pChildNode = pChildNode->GetNextSibling();

        } while (pChildNode != nullptr);
            
    return SUCCESS;
    }


StatusInt IScalableMeshSourceCreatorWorker::Impl::ProcessFilterTask(BeXmlNodeP pXmlTaskNode)
    {
    BeXmlNodeP pChildNode = pXmlTaskNode->GetFirstChild();

    do
        {
        assert(pChildNode != nullptr);
        assert(Utf8String(pChildNode->GetName()).CompareTo("tile") == 0);

        uint64_t tileId;

        BeXmlStatus xmlStatus = pChildNode->GetAttributeUInt64Value(tileId, "id");
        assert(xmlStatus == BEXML_Success);
                    
        TRACEPOINT(THREAD_ID(), EventType::WORKER_FILTER_TASK, tileId, (uint64_t)-1, -1, -1, 0, 0)    

        HFCPtr<MeshIndexType> pDataIndex(GetDataIndex());

        HPMBlockID blockID(tileId);

        HFCPtr<SMMeshIndexNode<DPoint3d, DRange3d>> meshNode((SMMeshIndexNode<DPoint3d, DRange3d>*)pDataIndex->CreateNewNode(blockID, false, true).GetPtr());

        meshNode->Unload();
        meshNode->NeedToLoadNeighbors(false);
        meshNode->Load();

        assert(!meshNode->m_nodeHeader.m_arePoints3d);

        //Ensure that all children nodes are reloaded so that it the contains the latest data which might have been generated by another worker.    
        HFCPtr<SMPointIndexNode<DPoint3d, DRange3d>> subNodeNoSplit(meshNode->GetSubNodeNoSplit());

        if (subNodeNoSplit != nullptr)
            {
            assert(!subNodeNoSplit->IsDirty());
            subNodeNoSplit->Unload();
            subNodeNoSplit->RemoveNonDisplayPoolData();
            meshNode->NeedToLoadNeighbors(false);
            subNodeNoSplit->Load();        
            }
        else
            {
            vector<HFCPtr<SMPointIndexNode<DPoint3d, DRange3d>>> childrenNodes(meshNode->GetSubNodes());

            for (auto& node : childrenNodes)
                {
                assert(!node->IsDirty());
                node->Unload();
                node->RemoveNonDisplayPoolData();
                meshNode->NeedToLoadNeighbors(false);    
                node->Load();            
                }
            }
    
        meshNode->Filter((int)meshNode->GetLevel(), nullptr);
        
        meshNode->SetDirty(true);

        meshNode->Discard();        
       // pDataIndex->ClearNodeMap();
        pDataIndex->Store();
        m_smSQLitePtr->Save();
        meshNode->Unload();

        meshNode = nullptr;

        pChildNode = pChildNode->GetNextSibling();

        } while (pChildNode != nullptr);


    return SUCCESS;
    }


StatusInt IScalableMeshSourceCreatorWorker::Impl::CreateTaskPlan()
    {        
    BeXmlDomPtr xmlDomPtr(BeXmlDom::CreateEmpty());
    BeXmlNodeP taskPlanNode(xmlDomPtr->AddNewElement("taskPlan", nullptr, nullptr));    

    taskPlanNode = taskPlanNode;

    /*
    HFCPtr<MeshIndexType> pDataIndex(GetDataIndex());        
    uint32_t nbRes = (uint32_t)pDataIndex->GetDepth();

    for (uint32_t resInd = nbRes; resInd > 0; resInd--)
        {                                             
        BeXmlNodeP stitchTaskNode(xmlDomPtr->AddNewElement("stitch", nullptr, taskPlanNode));
        stitchTaskNode->AddAttributeUInt32Value("res", resInd);

        BeXmlNodeP filterTaskNode(xmlDomPtr->AddNewElement("filter", nullptr, taskPlanNode));
        filterTaskNode->AddAttributeUInt32Value("res", resInd - 1);        
        }
        */
    BeFileName taskPlanFileName;

    GetTaskPlanFileName(taskPlanFileName);
    
    BeXmlDom::ToStringOption toStrOption = (BeXmlDom::ToStringOption)(BeXmlDom::TO_STRING_OPTION_Formatted | BeXmlDom::TO_STRING_OPTION_Indent);

    BeXmlStatus status = xmlDomPtr->ToFile(taskPlanFileName, toStrOption, BeXmlDom::FILE_ENCODING_Utf8);
    assert(status == BEXML_Success);

    return SUCCESS;
    }




StatusInt IScalableMeshSourceCreatorWorker::Impl::ExecuteNextTaskInTaskPlan()
    {
    BeFileName taskPlanFileName;

    GetTaskPlanFileName(taskPlanFileName);

    struct _stat64i32 buffer;

    //If task plan doesn't exist. 
    if (_wstat(taskPlanFileName.c_str(), &buffer) != 0)
        return SUCCESS_TASK_PLAN_COMPLETE;                

    FILE* file = nullptr;

    file = _wfsopen(taskPlanFileName.c_str(), L"ab+", _SH_DENYRW);
    
    if (file == nullptr)
        { 
        return SUCCESS;
        } 

    assert(file != nullptr);

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    BeXmlStatus status;
    WString     errorMsg;

    bvector<char> xmlFileContent(size);
    size_t readSize = fread(&xmlFileContent[0], 1, size, file);
    assert(readSize == size);
    
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromMemory(status, &xmlFileContent[0], xmlFileContent.size(), &errorMsg);

    assert(pXmlDom.IsValid());

    BeXmlNodeP pXmlTaskPlanNode(pXmlDom->GetRootElement());
	
	assert(pXmlTaskPlanNode != nullptr);
    
    assert(Utf8String(pXmlTaskPlanNode->GetName()).CompareTo("taskPlan") == 0);

    BeXmlNodeP pTaskNode = pXmlTaskPlanNode->GetFirstChild();

    if (pTaskNode == nullptr)
        {
        int closeStatus = fclose(file);
        assert(closeStatus == 0);

        return SUCCESS_TASK_PLAN_COMPLETE;
        }

    uint32_t resInd;

    BeXmlStatus xmlStatus = pTaskNode->GetAttributeUInt32Value(resInd, "res");
    assert(xmlStatus == BEXML_Success);
    
    if (Utf8String(pTaskNode->GetName()).CompareTo("stitch") == 0)
        {
        CreateStitchTasks(resInd);        
        }
    else
    if (Utf8String(pTaskNode->GetName()).CompareTo("filter") == 0)
        {
        CreateFilterTasks(resInd);
        }
    else
        {
        assert(!"Error - unkown task plan operation");
        }
 
    pXmlTaskPlanNode->RemoveChildNode(pTaskNode);

    Utf8String updatedTaskPlanXml;

    BeXmlDom::ToStringOption toStrOption = (BeXmlDom::ToStringOption)(BeXmlDom::TO_STRING_OPTION_Formatted | BeXmlDom::TO_STRING_OPTION_Indent);

    pXmlDom->ToString(updatedTaskPlanXml, toStrOption);
    
    FILE* reOpenFile = nullptr;

    while ((reOpenFile = _wfreopen(taskPlanFileName.c_str(), L"wb", file)) == nullptr);
    
    fwrite(updatedTaskPlanXml.c_str(), updatedTaskPlanXml.SizeInBytes(), 1, reOpenFile);
    int closeStatus = fclose(reOpenFile);
    assert(closeStatus == 0);

    return SUCCESS;
}



END_BENTLEY_SCALABLEMESH_NAMESPACE