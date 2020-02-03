/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>

#include <process.h>
#include <Bentley/BeDirectoryIterator.h>
#include <BeXml/BeXml.h>
#include "ImagePPHeaders.h"
#include "ScalableMeshSourceCreatorWorker.h"
#include "ScalableMeshQuadTreeBCLIBFilters.h"



#define TASK_PER_WORKER 3
#define TASK_PRIORITY_FOLDER_NAME L"TaskPriority"

USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

//This value returns an estimation of the number of points in a node after filtering. 
inline uint64_t GetNbObjectsEstimate(HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>> pNode, size_t nbResolutions)
    {
    return (double)pNode->GetCount() / (double)std::pow(4, nbResolutions - pNode->GetLevel() - 1);
    }

inline uint64_t GetNbObjectsEstimate(HFCPtr<SMMeshIndexNode<DPoint3d, Extent3dType>> pNode, size_t nbResolutions)
    {
    HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>> pPointIndexNode((SMPointIndexNode<DPoint3d, Extent3dType>*)pNode.GetPtr());
    return GetNbObjectsEstimate(pPointIndexNode, nbResolutions);
    }
    
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

StatusInt IScalableMeshSourceCreatorWorker::CreateTextureTasks(uint32_t maxGroupSize, const WString& jobName, const BeFileName& smFileName) const
{
    return static_cast<IScalableMeshSourceCreatorWorker::Impl*>(m_implP.get())->CreateTextureTasks(maxGroupSize, jobName, smFileName);
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

StatusInt IScalableMeshSourceCreatorWorker::ProcessTextureTask(BeXmlNodeP pXmlTaskNode) const
    {
    return static_cast<IScalableMeshSourceCreatorWorker::Impl*>(m_implP.get())->ProcessTextureTask(pXmlTaskNode);
    }


StatusInt IScalableMeshSourceCreatorWorker::OpenSqlFiles(bool readOnly, bool needSisterMainLockFile) 
    {
    return static_cast<IScalableMeshSourceCreatorWorker::Impl*>(m_implP.get())->OpenSqlFiles(readOnly, needSisterMainLockFile);
    }

StatusInt IScalableMeshSourceCreatorWorker::CloseSqlFiles()
    {
    return static_cast<IScalableMeshSourceCreatorWorker::Impl*>(m_implP.get())->CloseSqlFiles();
    }


IScalableMeshSourceCreatorWorkerPtr IScalableMeshSourceCreatorWorker::GetFor(const WChar*  filePath,
                                                                             uint32_t      nbWorkers,
                                                                             bool          isSharable,
                                                                             StatusInt&    status)
    {
    RegisterDelayedImporters();

    using namespace ISMStore;
    BeFileName fileName = BeFileName(filePath);

    
    IScalableMeshSourceCreatorWorkerPtr pCreatorWorker = new IScalableMeshSourceCreatorWorker(new IScalableMeshSourceCreatorWorker::Impl(filePath, nbWorkers, isSharable));    
    /*
    IScalableMeshSourceCreatorWorker::Impl* implP(dynamic_cast<IScalableMeshSourceCreatorWorker::Impl*>(pCreatorWorker->m_implP.get()));
    assert(implP != nullptr);
    */

    //implP->OpenSqlFiles(false, true);
    status = pCreatorWorker->m_implP->LoadFromFile();
    //implP->CloseSqlFiles();

    if (BSISUCCESS != status)
        return 0;
    
    return pCreatorWorker.get();
}


IScalableMeshSourceCreatorWorkerPtr IScalableMeshSourceCreatorWorker::GetFor(const IScalableMeshPtr& scmPtr,
                                                                             uint32_t                nbWorkers,
                                                                             bool                    isSharable,
                                                                             StatusInt&              status)
    {
    using namespace ISMStore;

    RegisterDelayedImporters();

    IScalableMeshSourceCreatorWorkerPtr pCreatorWorker = new IScalableMeshSourceCreatorWorker(new IScalableMeshSourceCreatorWorker::Impl(scmPtr, nbWorkers, isSharable));

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

IScalableMeshSourceCreatorWorker::Impl::Impl(const WChar* scmFileName, uint32_t nbWorkers, bool isSharable)
    : IScalableMeshSourceCreator::Impl(scmFileName) 
    {        
    m_smDb = nullptr;
    m_smSisterDb = nullptr;
    m_nbWorkers = nbWorkers;
    m_lockSleeper = BeDuration::FromSeconds(0.5);
    m_scalableMeshFileLock = nullptr;

    //Setting meshing and filtering to thread lead to crash/unexpected behavior.
    SetThreadingOptions(false, true, false);
    SetShareable(isSharable);
    ScalableMeshDb::SetEnableSharedDatabase(isSharable);
#ifdef TRACE_ON	    
    CachedDataEventTracer::GetInstance()->setOutputObjLog(false);
    CachedDataEventTracer::GetInstance()->setLogDirectory("C:\\traceLogs\\");
    CachedDataEventTracer::GetInstance()->start();
#endif	
    }

IScalableMeshSourceCreatorWorker::Impl::Impl(const IScalableMeshPtr& scmPtr, uint32_t nbWorkers, bool isSharable)
    : IScalableMeshSourceCreator::Impl(scmPtr)
    {      
    m_smDb = nullptr;
    m_smSisterDb = nullptr;
    m_nbWorkers = nbWorkers;
    m_lockSleeper = BeDuration::FromSeconds(0.5);
    m_scalableMeshFileLock = nullptr;

    //Setting meshing and filtering to thread lead to crash/unexpected behavior.
    SetThreadingOptions(false, true, false);
    SetShareable(isSharable);
    ScalableMeshDb::SetEnableSharedDatabase(isSharable);
#ifdef TRACE_ON		    
    CachedDataEventTracer::GetInstance()->setOutputObjLog(false);
    CachedDataEventTracer::GetInstance()->setLogDirectory("C:\\traceLogs\\");
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


void IScalableMeshSourceCreatorWorker::Impl::GetScalableMeshFileLock(bool readOnly)
    {
    assert(m_scalableMeshFileLock == nullptr);

    BeDuration sleeper(BeDuration::FromSeconds(0.5));

    BeFileName lockFileName(m_scmFileName);
    lockFileName.AppendString(L".lock");

    int accessMode;

    if (readOnly)
        {
        accessMode = _SH_DENYWR;
        }
    else
        {
        accessMode = _SH_DENYRW;
        }
    
    while ((m_scalableMeshFileLock  = _wfsopen(lockFileName, L"ab+", accessMode)) == nullptr)
        {
        sleeper.Sleep();            
        }        
    }

void IScalableMeshSourceCreatorWorker::Impl::ReleaseScalableMeshFileLock()
    {
    assert(m_scalableMeshFileLock != nullptr);
    fclose(m_scalableMeshFileLock);
    m_scalableMeshFileLock = nullptr;
    }   
                   
HFCPtr<MeshIndexType> IScalableMeshSourceCreatorWorker::Impl::GetDataIndex()
    {
    if (m_pDataIndex.GetPtr() == nullptr)
        {     
        /*
        BeFileName lockFileName(m_scmFileName);
        lockFileName.AppendString(L".lock");

        FILE* lockFile; 
        BeDuration sleeper(BeDuration::FromSeconds(0.5));

        while ((lockFile = _wfsopen(lockFileName, L"ab+", _SH_DENYRW)) == nullptr)
            {
            sleeper.Sleep();            
            }
            */
        StatusInt status = IScalableMeshCreator::Impl::CreateDataIndex(m_pDataIndex, true, SM_ONE_SPLIT_THRESHOLD);

        //fclose(lockFile);

/*
#ifndef NDEBUG
        if (m_mainFilePtr != nullptr)
            m_mainFilePtr->GetDb()->SetCanReopenShared(false);

        if (m_sisterFilePtr != nullptr)
            m_sisterFilePtr->GetDb()->SetCanReopenShared(false);
#endif
*/

        assert(m_pDataIndex.GetPtr() != nullptr);   
        assert(status == SUCCESS);
        assert(dynamic_cast<SMSQLiteStore<PointIndexExtentType>*>(m_pDataIndex->GetDataStore().get()) != nullptr);

        SMSQLiteStore<PointIndexExtentType>* pSqliteStore(static_cast<SMSQLiteStore<PointIndexExtentType>*>(m_pDataIndex->GetDataStore().get()));
        pSqliteStore->SetRemoveTempGenerationFile(false);        
        }

    return m_pDataIndex;
    }

void IScalableMeshSourceCreatorWorker::Impl::FreeDataIndex()
    {
    m_smDb = nullptr;
    m_smSisterDb = nullptr;
    m_mainFilePtr = nullptr;
    m_sisterFilePtr = nullptr;
    m_pDataIndex = nullptr;
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
    bool              m_requireMeshingFiltering = true;
    bool              m_isNodeIdsSorted = false;
    bool              m_isNodeStitchIdsSorted = false;
    //uint64_t          m_levelId;
    bvector<uint64_t> m_nodeIds; //Node IDs to mesh and filter or just stitching depending on m_requireMeshingFiltering's value.
    bvector<uint64_t> m_nodeStitchIds; //Node ID to stitch. Should be a subset of m_nodeId when m_requireMeshingFiltering is true.

    bool FindNode(uint64_t nodeId) 
        {
        if (m_isNodeIdsSorted == false)
            { 
            std::sort(m_nodeIds.begin(), m_nodeIds.end());
            m_isNodeIdsSorted = true;
            }  

        return std::binary_search(m_nodeIds.begin(), m_nodeIds.end(), nodeId);
        }
    
    bool FindStitchNode(uint64_t stitchNodeId) 
        {
        if (m_isNodeStitchIdsSorted == false)
            {
            std::sort(m_nodeStitchIds.begin(), m_nodeStitchIds.end());
            m_isNodeStitchIdsSorted = true;
            }  

        return std::binary_search(m_nodeStitchIds.begin(), m_nodeStitchIds.end(), stitchNodeId);
        }

    void GetUnstitchedNodes(bvector<uint64_t>& nodeIds) 
        {
        for (auto& nodeId : m_nodeIds)
            {
            if (!FindStitchNode(nodeId))
                {
                nodeIds.push_back(nodeId);
                }
            }
        }    

    bool IsStitchable(IScalableMeshNodePtr& node) 
        {
        bool areAllNeighborFound = true; 

        for (char relativePosX = -1; relativePosX <= 1 && areAllNeighborFound; relativePosX++)
            for (char relativePosY = -1; relativePosY <= 1 && areAllNeighborFound; relativePosY++)
                for (char relativePosZ = -1; relativePosZ <= 1 && areAllNeighborFound; relativePosZ++)
                    {
                    bvector<IScalableMeshNodePtr> neighborNodes = node->GetNeighborAt(relativePosX, relativePosY, relativePosZ);
                    for (auto& neighborNode : neighborNodes)
                        {                                                         
                        ScalableMeshNode<DPoint3d>* smNeighborNode(dynamic_cast<ScalableMeshNode<DPoint3d>*>(neighborNode.get()));

                        if ((smNeighborNode->GetNodePtr()->GetCount() > 0) && !FindNode(neighborNode->GetNodeId()))
                            {
                            areAllNeighborFound = false;
                            break;
                            }
                        }
                    }

        return areAllNeighborFound;
        }

    void Merge(const NodesToGenerate& nodesToGenerate, HFCPtr<MeshIndexType> pDataIndex)
        {        
        m_nodeIds.insert(m_nodeIds.end(), nodesToGenerate.m_nodeIds.begin(), nodesToGenerate.m_nodeIds.end());
        m_nodeStitchIds.insert(m_nodeStitchIds.end(), nodesToGenerate.m_nodeStitchIds.begin(), nodesToGenerate.m_nodeStitchIds.end());
        m_isNodeIdsSorted = false;
        m_isNodeStitchIdsSorted = false;

        for (auto& nodeId : m_nodeIds)
            {
            if (!FindStitchNode(nodeId))
                {
                HPMBlockID blockID(nodeId);

                HFCPtr<SMPointIndexNode<DPoint3d, DRange3d>> meshNode((SMPointIndexNode<DPoint3d, DRange3d>*)pDataIndex->CreateNewNode(blockID, false, true).GetPtr());            
                meshNode->NeedToLoadNeighbors(true);        
                meshNode->Load();

                IScalableMeshNodePtr smNodePtr;  
                smNodePtr = new ScalableMeshNode<DPoint3d>(meshNode);
                
                if (IsStitchable(smNodePtr))
                    {
                    m_nodeStitchIds.push_back(nodeId);
                    m_isNodeStitchIdsSorted = false;
                    }                    
                }
            }
        }
    };

inline HFCPtr<SMMeshIndexNode<DPoint3d, DRange3d>> GetMeshNode(uint64_t nodeId, bool needNeighbors, HFCPtr<MeshIndexType>& pDataIndex)
    {    
    HPMBlockID blockID(nodeId);
    HFCPtr<SMMeshIndexNode<DPoint3d, DRange3d>> meshNode((SMMeshIndexNode<DPoint3d, DRange3d>*)pDataIndex->CreateNewNode(blockID, false, true).GetPtr());

    if (!meshNode->IsLoaded())
        {
        meshNode->NeedToLoadNeighbors(needNeighbors);
        meshNode->Load();
        return meshNode;
        }
    
    if (!meshNode->IsNeighborsLoaded() && needNeighbors)
        {
        meshNode->Unload();
        meshNode->NeedToLoadNeighbors(needNeighbors);
        meshNode->Load();
        }
    
    return meshNode;
    }

void GetNeighborNodeIds(bvector<uint64_t>& neighborNodeIds, uint64_t nodeId, HFCPtr<MeshIndexType>& pDataIndex)
    {        
    IScalableMeshNodePtr smNodePtr;  
    HFCPtr<SMPointIndexNode<DPoint3d, DRange3d>> nodePtr(GetMeshNode(nodeId, true, pDataIndex).GetPtr());
    smNodePtr = new ScalableMeshNode<DPoint3d>(nodePtr);
                    
    for (char relativePosX = -1; relativePosX <= 1; relativePosX++)
        for (char relativePosY = -1; relativePosY <= 1; relativePosY++)
            for (char relativePosZ = -1; relativePosZ <= 1; relativePosZ++)
                {
                bvector<IScalableMeshNodePtr> neighborNodes = smNodePtr->GetNeighborAt(relativePosX, relativePosY, relativePosZ);

                for (auto& neighborNode : neighborNodes)
                    {
                    neighborNodeIds.push_back(neighborNode->GetNodeId());                    
                    }
                }    
    }

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

struct NodeTask;

typedef RefCountedPtr<NodeTask> NodeTaskPtr;

struct NodeTask : public RefCountedBase
{

    NodeTask(int nbResolutions)
        {             
        m_orderId = 0;
        m_totalNbPoints = 0;        
        m_resolutionToGenerate.resize(nbResolutions);        
        }

    NodeTask(bvector<NodeTaskPtr>& currentTasks, int nbResolutions, IScalableMeshNodePtr& rootNode)
    {
        m_orderId = 0;
        m_totalNbPoints = 0;
        m_groupRootNodes.push_back(rootNode);
        m_resolutionToGenerate.resize(nbResolutions);

    }

    void AccumulateNodes(bvector<IScalableMeshNodePtr>& groupNodes, bvector<NodeTaskPtr>& currentTasks, IScalableMeshNodePtr& currentNode)
        {        
        for (auto& task : currentTasks)
            {
            for (auto& rootNode : task->m_groupRootNodes)
                {
                if (rootNode->GetNodeId() == currentNode->GetNodeId())
                    {                
                    m_orderId = max(m_orderId, task->m_orderId + 1);
                    return;
                    }
                }
            }


        /*TBD_G : Currently empty node needs to be return to take into account empty neighbor node when computing stitchable nodes.
        ScalableMeshNode<DPoint3d>* smNode(dynamic_cast<ScalableMeshNode<DPoint3d>*>(currentNode.get()));

        if (smNode->GetNodePtr()->GetCount() == 0)
            return;
			*/

        groupNodes.push_back(currentNode);
        
        bvector<IScalableMeshNodePtr> childrenNodes(currentNode->GetChildrenNodes());

        for (auto& childNode : childrenNodes)
            {            
            AccumulateNodes(groupNodes, currentTasks, childNode);
            }
        }


    void AddNode(IScalableMeshNodePtr& currentNode)
        {                
        ScalableMeshNode<DPoint3d>* smNode(dynamic_cast<ScalableMeshNode<DPoint3d>*>(currentNode.get()));

        uint64_t nbObjects = GetNbObjectsEstimate(smNode->GetNodePtr(), m_resolutionToGenerate.size());

		m_resolutionToGenerate[currentNode->GetLevel()].m_nodeIds.push_back(currentNode->GetNodeId());
		m_totalNbPoints += nbObjects;
        }  


    bvector<IScalableMeshNodePtr> m_groupRootNodes; 
    bvector<NodesToGenerate> m_resolutionToGenerate;
    uint32_t                 m_orderId;
    uint64_t                 m_totalNbPoints;
};


#define MAX_COMMON_ANCESTOR 3


static bool s_doTaskMerging = true;

struct GenerationTask;

typedef RefCountedPtr<GenerationTask> GenerationTaskPtr;

struct GenerationTask : public NodeTask
    {
    GenerationTask(int nbResolutions)
        : NodeTask(nbResolutions)
        {
        }

    GenerationTask(bvector<NodeTaskPtr>& currentTasks, int nbResolutions, IScalableMeshNodePtr& rootNode)
        : NodeTask(currentTasks, nbResolutions, rootNode)
        { 

        bvector<IScalableMeshNodePtr> groupNodes;
   
        AccumulateNodes(groupNodes, currentTasks, rootNode);

        for (auto& node : groupNodes)
        {
            AddNode(node);
            //m_resolutionToGenerate[node->GetLevel()].m_nodeIds.push_back(node->GetNodeId());
        }
        for (auto& node : groupNodes)
            {
            /*
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
*/

            if (m_resolutionToGenerate[node->GetLevel()].IsStitchable(node)) 
                m_resolutionToGenerate[node->GetLevel()].m_nodeStitchIds.push_back(node->GetNodeId());
            }
        }
		
    void MergeGenerationTask(const GenerationTaskPtr& newGenerationTask, HFCPtr<MeshIndexType> pDataIndex)
        {                        
        assert(newGenerationTask->m_resolutionToGenerate.size() == m_resolutionToGenerate.size());
        assert(newGenerationTask->m_orderId == m_orderId);

        for (uint32_t levelInd = 0; levelInd < m_resolutionToGenerate.size(); levelInd++)
            {
            m_resolutionToGenerate[levelInd].Merge(newGenerationTask->m_resolutionToGenerate[levelInd], pDataIndex);
            }          

        m_totalNbPoints += newGenerationTask->m_totalNbPoints;        
        m_groupRootNodes.insert(m_groupRootNodes.end(), newGenerationTask->m_groupRootNodes.begin(), newGenerationTask->m_groupRootNodes.end());
        }  
    };

size_t ComputeCommonAncestorLevel(const GenerationTaskPtr& generationTask1, const GenerationTaskPtr& generationTask2)
{
    IScalableMeshNodePtr parentNode1 = generationTask1->m_groupRootNodes.front()->GetParentNode();
    IScalableMeshNodePtr parentNode2 = generationTask2->m_groupRootNodes.front()->GetParentNode();

    assert(parentNode1 != nullptr || parentNode2 != nullptr);

    if (parentNode1 == nullptr)
    {
        size_t commonAncestorLevel = generationTask2->m_groupRootNodes.front()->GetLevel() - generationTask1->m_groupRootNodes.front()->GetLevel();
        return commonAncestorLevel;
    }

    if (parentNode2 == nullptr)
    {
        size_t commonAncestorLevel = generationTask1->m_groupRootNodes.front()->GetLevel() - generationTask2->m_groupRootNodes.front()->GetLevel();
        return commonAncestorLevel;
    }

    while (parentNode1->GetLevel() > parentNode2->GetLevel())
    {
        parentNode1 = parentNode1->GetParentNode();
    }

    while (parentNode2->GetLevel() > parentNode1->GetLevel())
    {
        parentNode2 = parentNode2->GetParentNode();
    }

    while (parentNode1->GetNodeId() != parentNode2->GetNodeId())
    {
        parentNode1 = parentNode1->GetParentNode();
        parentNode2 = parentNode2->GetParentNode();
    }

    size_t commonAncestorLevel1 = generationTask1->m_groupRootNodes.front()->GetLevel() - parentNode1->GetLevel();
    size_t commonAncestorLevel2 = generationTask2->m_groupRootNodes.front()->GetLevel() - parentNode2->GetLevel();

    return max(commonAncestorLevel1, commonAncestorLevel2);
}

enum NodeTaskType
{
    GENERATION = 0,
    TEXTURE,
    QTY
};

struct TextureTask;

typedef RefCountedPtr<TextureTask> TextureTaskPtr;

struct TextureTask : public NodeTask
{
    TextureTask(bvector<NodeTaskPtr>& currentTasks, int nbResolutions, IScalableMeshNodePtr& rootNode)
        : NodeTask(currentTasks, nbResolutions, rootNode)
    {
        bvector<IScalableMeshNodePtr> groupNodes;

        AccumulateNodes(groupNodes, currentTasks, m_groupRootNodes[0]);

        for (auto& node : groupNodes)
        {
            AddNode(node);
        }
    }
};

void CreateNodeTask(bvector<NodeTaskPtr>& toExecuteTasks, IScalableMeshNodePtr& groupRootNode, int nbResolutions, uint64_t pointThreshold, HFCPtr<MeshIndexType> pDataIndex, NodeTaskType t)
    {           
    NodeTaskPtr newGenerationTask;
    
    if(NodeTaskType::GENERATION == t)
        newGenerationTask = new GenerationTask(toExecuteTasks, nbResolutions, groupRootNode);
    else if (NodeTaskType::TEXTURE == t)
        newGenerationTask = new TextureTask(toExecuteTasks, nbResolutions, groupRootNode);
    else
    {
        assert(false);
        return;
    }
    
    if (newGenerationTask->m_totalNbPoints > 0)    
        if(NodeTaskType::GENERATION == t)
		{
		 bool isNewTaskMerged = false;
        //Deep first accumulation means that closer nodes should be at the end of toExecuteTasks.
        for (auto taskToExecute = toExecuteTasks.rbegin(); taskToExecute != toExecuteTasks.rend() && s_doTaskMerging; ++taskToExecute) 
            {
            if (newGenerationTask->m_orderId == (*taskToExecute)->m_orderId &&
                newGenerationTask->m_totalNbPoints + (*taskToExecute)->m_totalNbPoints < pointThreshold)
                {
                size_t commonAncestorLevel = ComputeCommonAncestorLevel(dynamic_cast<GenerationTask*>(&*(*taskToExecute)), dynamic_cast<GenerationTask*>(&*(newGenerationTask)));

                if (commonAncestorLevel <= MAX_COMMON_ANCESTOR)
                    {                    
                    dynamic_cast<GenerationTask*>(&*(*taskToExecute))->MergeGenerationTask(dynamic_cast<GenerationTask*>(&*(newGenerationTask)), pDataIndex);
                    isNewTaskMerged = true;
                    break;
                    }               
                }            
            }

        if (!isNewTaskMerged)
            {
            toExecuteTasks.push_back(newGenerationTask);
            }
		}
        else
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


void GroupNodes(bvector<NodeTaskPtr>& toExecuteTasks, IScalableMeshNodePtr& currentNode, uint64_t pointThreshold, int& childrenGroupingSize, int nbResolutions, HFCPtr<MeshIndexType>& pDataIndex, NodeTaskType t)
    {    
    ScalableMeshNode<DPoint3d>* smNode(dynamic_cast<ScalableMeshNode<DPoint3d>*>(currentNode.get()));
    assert(smNode != nullptr);

    assert(childrenGroupingSize == 0);

    //if (smNode->GetNodePtr()->GetCount() < pointThreshold)
    if (GetTotalCountWithSubResolutions(smNode->GetNodePtr(), nbResolutions, nbResolutions - 1) < pointThreshold)
        {                
        childrenGroupingSize = smNode->GetNodePtr()->GetCount();
        //Dont add empty branch.
        if (childrenGroupingSize > 0)
            {
            CreateNodeTask(toExecuteTasks, currentNode, nbResolutions, pointThreshold, pDataIndex, t);
            }
        return;
        }

    bvector<IScalableMeshNodePtr> childrenNodes(currentNode->GetChildrenNodes());
    
    if (childrenNodes.size() > 0)
        {                
        for (auto& node : childrenNodes)
            {
            int childGroupingSize = 0;
            GroupNodes(toExecuteTasks, node, pointThreshold, childGroupingSize, nbResolutions, pDataIndex, t);
            childrenGroupingSize += childGroupingSize;            
            }        
        }

    const HFCPtr<SMPointIndexNode<DPoint3d, DRange3d>>& parentNode(smNode->GetNodePtr()->GetParentNode());

    if (parentNode == nullptr)
        {
        //Create final group
        CreateNodeTask(toExecuteTasks, currentNode, nbResolutions, pointThreshold, pDataIndex, t);
        //childrenGroupingSize = smNode->GetNodePtr()->GetCount();
        childrenGroupingSize = GetTotalCountWithSubResolutions(smNode->GetNodePtr(), nbResolutions, nbResolutions - 1);
        return;
        }
        
    uint64_t totalCountCurrentNode = GetTotalCountWithSubResolutions(smNode->GetNodePtr(), nbResolutions, nbResolutions - 1);

    if (totalCountCurrentNode - childrenGroupingSize > pointThreshold)
        {        
        //Will lead to group bigger than threshold but preferable than creating a group for each children, which can results in smaller group.
        assert(totalCountCurrentNode - childrenGroupingSize <= pointThreshold * 4);

        CreateNodeTask(toExecuteTasks, currentNode, nbResolutions, pointThreshold, pDataIndex, t);        
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


void CreateGroupStitchingTasks(bvector<NodeTaskPtr>& toExecuteTasks, uint32_t maxGroupSize, HFCPtr<MeshIndexType>& pDataIndex)
    {
    assert(toExecuteTasks.size() > 0);
    bvector<NodeTaskPtr> remainingGroupTasks;
    remainingGroupTasks.insert(remainingGroupTasks.end(), toExecuteTasks.begin(), toExecuteTasks.end());

    bvector<GenerationTaskPtr> groupStitchingTasks;

    GenerationTaskPtr currentTask(new GenerationTask((uint32_t)toExecuteTasks[0]->m_resolutionToGenerate.size()));

    uint32_t maxOrderId = 0;

    for (auto& remaningGroupTask : remainingGroupTasks)
        {
        maxOrderId = max(maxOrderId, remaningGroupTask->m_orderId);        
        }

    maxOrderId += 1;
    
    auto remainingTaskIter = remainingGroupTasks.begin();

    while (remainingTaskIter != remainingGroupTasks.end())
        {
        maxOrderId = max(maxOrderId, (*remainingTaskIter)->m_orderId);

        for (size_t resInd = 0; resInd < (*remainingTaskIter)->m_resolutionToGenerate.size(); resInd++)        
            {            
            bvector<uint64_t> toStitchNodeIds;
            (*remainingTaskIter)->m_resolutionToGenerate[resInd].GetUnstitchedNodes(toStitchNodeIds);

            bset<uint64_t> allNeighborNodeIds;            

            uint64_t totalPointsCount = 0;

            for (auto& nodeId : toStitchNodeIds)
                {
                bvector<uint64_t> neighborNodeIds;
                GetNeighborNodeIds(neighborNodeIds, nodeId, pDataIndex);
                allNeighborNodeIds.insert(neighborNodeIds.begin(), neighborNodeIds.end());

                HFCPtr<SMMeshIndexNode<DPoint3d, DRange3d>> stitchNode = GetMeshNode(nodeId, false, pDataIndex);
                totalPointsCount += GetNbObjectsEstimate(stitchNode, (*remainingTaskIter)->m_resolutionToGenerate.size());                
                }

            //Don't count neighbor nodes already required by previous node to stitch.
            bvector<uint64_t> foundNodeIds; 
            bvector<uint64_t> notFoundNodeIds; 

            for (auto& neighborNodeId : allNeighborNodeIds)
                {
                if (currentTask->m_resolutionToGenerate[resInd].FindNode(neighborNodeId))
                    {
                    foundNodeIds.push_back(neighborNodeId);
                    }
                else
                    {
                    HFCPtr<SMMeshIndexNode<DPoint3d, DRange3d>> neighborNode = GetMeshNode(neighborNodeId, false, pDataIndex);                
                    totalPointsCount += GetNbObjectsEstimate(neighborNode, (*remainingTaskIter)->m_resolutionToGenerate.size());                
                    notFoundNodeIds.push_back(neighborNodeId);
                    }
                }
            
            if (currentTask->m_totalNbPoints + totalPointsCount > maxGroupSize)
                {
                //MST - Edge case, when the ratio split threshold/group size is big handling the stiching of one group can required more points 
                //than the generation group itself.
                if (currentTask->m_totalNbPoints > 0)
                    {
                    toExecuteTasks.push_back(currentTask);
                    currentTask = new GenerationTask((uint32_t)toExecuteTasks[0]->m_resolutionToGenerate.size());
                    }

                for (auto& foundNodeId : foundNodeIds)
                    {
                    HFCPtr<SMMeshIndexNode<DPoint3d, DRange3d>> neighborNode = GetMeshNode(foundNodeId, false, pDataIndex);                
                    totalPointsCount += GetNbObjectsEstimate(neighborNode, (*remainingTaskIter)->m_resolutionToGenerate.size());                                    
                    }

                notFoundNodeIds.insert(notFoundNodeIds.end(), foundNodeIds.begin(), foundNodeIds.end());
                }

            currentTask->m_resolutionToGenerate[resInd].m_requireMeshingFiltering = false;            
            currentTask->m_resolutionToGenerate[resInd].m_nodeStitchIds.insert(currentTask->m_resolutionToGenerate[resInd].m_nodeStitchIds.end(), toStitchNodeIds.begin(), toStitchNodeIds.end());            
            currentTask->m_resolutionToGenerate[resInd].m_nodeIds.insert(currentTask->m_resolutionToGenerate[resInd].m_nodeIds.end(), notFoundNodeIds.begin(), notFoundNodeIds.end());

            std::sort(notFoundNodeIds.begin(), notFoundNodeIds.end());

            for (auto& stitchId : toStitchNodeIds)
                {
                if (!std::binary_search(notFoundNodeIds.begin(), notFoundNodeIds.end(), stitchId))
                    currentTask->m_resolutionToGenerate[resInd].m_nodeIds.push_back(stitchId);
                }
                                            
            currentTask->m_totalNbPoints += totalPointsCount;
            currentTask->m_orderId = maxOrderId;
            }    

        remainingTaskIter++;
        }
    
    if (currentTask->m_totalNbPoints > 0)
        toExecuteTasks.push_back(currentTask);
    
    /*

    size_t minAncestorLevel; 
    size_t commonAncestorLevel = ComputeCommonAncestorLevel(*taskToExecute, newGenerationTask);

    bool              m_requireMeshingFiltering = true;
    bool              m_isNodeIdsSorted = false;
    bool              m_isNodeStitchIdsSorted = false;
    //uint64_t          m_levelId;
    bvector<uint64_t> m_nodeIds; //Node IDs to mesh and filter or just stitching depending on m_requireMeshingFiltering's value.
    bvector<uint64_t> m_nodeStitchIds; //Node ID to stitch. Should be a subset of m_nodeId when m_requireMeshingFiltering is true.    
    */

    /*
     bvector<IScalableMeshNodePtr> m_groupRootNodes; 
    bvector<NodesToGenerate>      m_resolutionToGenerate;
    uint32_t                      m_orderId;
    uint64_t                      m_totalNbPoints;
    */
    }

void IScalableMeshSourceCreatorWorker::Impl::GetTextureTasks(bvector<NodeTaskPtr>& toExecuteTasks, uint32_t maxGroupSize)
{
    HFCPtr<MeshIndexType> pDataIndex(GetDataIndex());

    HFCPtr<SMPointIndexNode<DPoint3d, DRange3d>> rootNode(pDataIndex->GetRootNode());

    IScalableMeshNodePtr meshRootNode(new ScalableMeshNode<DPoint3d>(rootNode));

    int childrenGroupingSize = 0;
    int nbResolutions = (int)(pDataIndex->GetDepth() + 1);

    GroupNodes(toExecuteTasks, meshRootNode, maxGroupSize, childrenGroupingSize, nbResolutions, pDataIndex, NodeTaskType::TEXTURE);
}


void IScalableMeshSourceCreatorWorker::Impl::GetGenerationTasks(bvector<NodeTaskPtr>& toExecuteTasks, uint32_t maxGroupSize)
    {   
    HFCPtr<MeshIndexType> pDataIndex(GetDataIndex());

    HFCPtr<SMPointIndexNode<DPoint3d, DRange3d>> rootNode(pDataIndex->GetRootNode());
    
    IScalableMeshNodePtr meshRootNode(new ScalableMeshNode<DPoint3d>(rootNode));
                
    int childrenGroupingSize = 0;     
    int nbResolutions = (int)(pDataIndex->GetDepth() + 1);

    GroupNodes(toExecuteTasks, meshRootNode, maxGroupSize, childrenGroupingSize, nbResolutions, pDataIndex, NodeTaskType::GENERATION);
             
#if defined(_WIN32)                            

    size_t totalNbNodes = 0;
    size_t totalNbNodesToStitch = 0;

    for (auto& task : toExecuteTasks)
        {
        for (auto& resToGen : task->m_resolutionToGenerate)
            {
            totalNbNodes += resToGen.m_nodeIds.size();
            totalNbNodesToStitch += resToGen.m_nodeStitchIds.size();
            }
        }
        
    double independentStitchingPercentage = (double)totalNbNodesToStitch / totalNbNodes;

    wchar_t text_buffer[1000] = { 0 }; //temporary buffer
    swprintf(text_buffer, _countof(text_buffer), L"Nb Nodes To Mesh : %zd    Nb Nodes To Stich : %zd   Ratio : %.2f \r\n", totalNbNodes, totalNbNodesToStitch, independentStitchingPercentage); 
    OutputDebugStringW(text_buffer); // print

#ifdef NDEBUG
    wprintf(text_buffer);
#endif

#endif

    CreateGroupStitchingTasks(toExecuteTasks, maxGroupSize, pDataIndex);
    maxGroupSize = maxGroupSize;
    }


StatusInt IScalableMeshSourceCreatorWorker::Impl::CreateGenerationTasks(uint32_t maxGroupSize, const WString& jobName, const BeFileName& smFileName)
    {
    BeFileName taskDirectory(m_scmFileName);

    taskDirectory = taskDirectory.GetDirectoryName();

    bvector<uint64_t> nodesToMesh;
    
    bvector<NodeTaskPtr> generationTasks;

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
    uint32_t maxPriority = 0;
    
    for (size_t ind = 0; ind < generationTasks.size(); ind++)    
        {                                              
        BeFileName meshTaskFile(taskDirectory);

        /*
        if (generationTasks[ind]->m_orderId > 0)
            continue;
            */
                
        maxPriority = max(maxPriority, generationTasks[ind]->m_orderId);
        
        if (generationTasks[ind]->m_orderId > 0)
            {            
            swprintf(stringBuffer, L"\\%s%i\\", TASK_PRIORITY_FOLDER_NAME, generationTasks[ind]->m_orderId);
            meshTaskFile.AppendToPath(stringBuffer);            

            if (!meshTaskFile.DoesPathExist())
                {
                BeFileNameStatus status = BeFileName::CreateNewDirectory(meshTaskFile.c_str());
                assert(status == BeFileNameStatus::Success);                
                }
            }

        swprintf(stringBuffer, L"Generate%zi.xml", ind);        
        meshTaskFile.AppendString(stringBuffer);
        BeXmlDomPtr xmlDomPtr(BeXmlDom::CreateEmpty());
        
        BeXmlNodeP workerNode(xmlDomPtr->AddNewElement("workerTask", nullptr, nullptr));
        workerNode->AddAttributeStringValue("type", "generate");                
        workerNode->AddAttributeStringValue("jobName", jobName.c_str());
        workerNode->AddAttributeStringValue("smName", smFileName.c_str());

        /*
         IScalableMeshNodePtr     m_groupRootNodes;
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


                bool needMeshingFiltering = generationTasks[ind]->m_resolutionToGenerate[resInd].m_requireMeshingFiltering;
                bool needFiltering = false;

                if (resInd != (int)generationTasks[ind]->m_resolutionToGenerate.size() - 1 && needMeshingFiltering)
                    needFiltering = true;                

                tileNode->AddAttributeBooleanValue("needFiltering", needFiltering);
                tileNode->AddAttributeBooleanValue("needMeshing", needMeshingFiltering);
                
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

    bvector<IDTMSource*> rasterSources;

    GetRasterSources(rasterSources);
    
    if (rasterSources.size() > 0)
        {
        maxPriority++;
                        
        StatusInt statusTexturing = CreateTextureTasks(maxGroupSize, jobName, smFileName, maxPriority);

        assert(statusTexturing == SUCCESS);
        }

    generationTasks.clear();
    FreeDataIndex();

    CreateTaskPlanForTaskGrouping(maxPriority, jobName, smFileName);
    
    return SUCCESS;
    }

StatusInt IScalableMeshSourceCreatorWorker::Impl::CreateTextureTasks(uint32_t maxGroupSize, const WString& jobName, const BeFileName& smFileName, int basePriority)
    {
    BeFileName taskDirectory(m_scmFileName);

    taskDirectory = taskDirectory.GetDirectoryName();

    bvector<uint64_t> initialTextureNodes;

    bvector<NodeTaskPtr> textureTasks;

    //Explicitly open the sql file to improve speed when 3SM is located on network drive
    StatusInt status = OpenSqlFiles(true, false);
    assert(status == SUCCESS);

    GetTextureTasks(textureTasks, maxGroupSize);

    CloseSqlFiles();

    wchar_t stringBuffer[100000];
    uint32_t nbBitsForGrouping = ceil(log(textureTasks.size()) / log(2));
    
    for (size_t ind = 0; ind < textureTasks.size(); ind++)
        {
        BeFileName meshTaskFile(taskDirectory);
        
        //assert(textureTasks[ind]->m_orderId == 0);        
        /*
        if (textureTasks[ind]->m_orderId > 0)
            continue;
        */

        if (basePriority > 0)
            {            
            swprintf(stringBuffer, L"\\%s%i\\", TASK_PRIORITY_FOLDER_NAME, basePriority);
            meshTaskFile.AppendToPath(stringBuffer);            

            if (!meshTaskFile.DoesPathExist())
                {
                BeFileNameStatus status = BeFileName::CreateNewDirectory(meshTaskFile.c_str());
                assert(status == BeFileNameStatus::Success);                
                }
            }

        swprintf(stringBuffer, L"Texture%zi.xml", ind);
        meshTaskFile.AppendString(stringBuffer);
        BeXmlDomPtr xmlDomPtr(BeXmlDom::CreateEmpty());

        BeXmlNodeP workerNode(xmlDomPtr->AddNewElement("workerTask", nullptr, nullptr));
        workerNode->AddAttributeStringValue("type", "texture");
        workerNode->AddAttributeStringValue("jobName", jobName.c_str());
        workerNode->AddAttributeStringValue("smName", smFileName.c_str());    

        //Node IDs are saved as signed 32 bits in 3sm file, so cannot go above 31 bits.
        uint64_t baseNodeId = (ind + 1) << (31 - nbBitsForGrouping);
        assert(baseNodeId >= (((uint64_t)1) << (31 - nbBitsForGrouping)) && baseNodeId < std::numeric_limits<int32_t>::max());

        workerNode->AddAttributeUInt64Value("newBaseNodeId", baseNodeId);

        for (int resInd = (int)textureTasks[ind]->m_resolutionToGenerate.size() - 1; resInd >= 0; resInd--)
        {
            if (textureTasks[ind]->m_resolutionToGenerate[resInd].m_nodeIds.size() > 0)
            {
                BeXmlNodeP tileNode(xmlDomPtr->AddNewElement("TextureTiles", nullptr, workerNode));
                WString tileList;

                for (auto& nodeId : textureTasks[ind]->m_resolutionToGenerate[resInd].m_nodeIds)
                {
                    WPrintfString tileId(L"%u,", nodeId);
                    tileList.append(tileId);
                }

                tileNode->AddAttributeStringValue("ids", tileList.c_str());             
            }

        }

        BeXmlDom::ToStringOption toStrOption = (BeXmlDom::ToStringOption)(BeXmlDom::TO_STRING_OPTION_Formatted | BeXmlDom::TO_STRING_OPTION_Indent);

        BeXmlStatus status = xmlDomPtr->ToFile(meshTaskFile, toStrOption, BeXmlDom::FILE_ENCODING_Utf8);
        assert(status == BEXML_Success);
    }

        return SUCCESS;
    }

void IScalableMeshSourceCreatorWorker::Impl::CreateTaskPlanForTaskGrouping(uint32_t maxPriority, const WString& jobName, const BeFileName& smFileName)
    {    
    BeXmlDomPtr xmlDomPtr(BeXmlDom::CreateEmpty());
    BeXmlNodeP taskPlanNode(xmlDomPtr->AddNewElement("taskPlan", nullptr, nullptr));    
    taskPlanNode->AddAttributeStringValue("jobName", jobName.c_str());
    taskPlanNode->AddAttributeStringValue("smName", smFileName.c_str());
        
    for (uint32_t currentPriority = 1; currentPriority <= maxPriority; currentPriority++)
        {
        BeXmlNodeP stitchTaskNode(xmlDomPtr->AddNewElement("copyNextPriorityTasks", nullptr, taskPlanNode));
        stitchTaskNode->AddAttributeUInt32Value("priority", currentPriority);        
        }

    BeFileName taskPlanFileName;

    GetTaskPlanFileName(taskPlanFileName);
    
    BeXmlDom::ToStringOption toStrOption = (BeXmlDom::ToStringOption)(BeXmlDom::TO_STRING_OPTION_Formatted | BeXmlDom::TO_STRING_OPTION_Indent);

    BeXmlStatus status = xmlDomPtr->ToFile(taskPlanFileName, toStrOption, BeXmlDom::FILE_ENCODING_Utf8);
    assert(status == BEXML_Success);    
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

StatusInt IScalableMeshSourceCreatorWorker::Impl::CopyNextPriorityTasks(uint32_t priority)
    {            
    BeFileName taskDirectory(m_scmFileName);
    taskDirectory = taskDirectory.GetDirectoryName();
        
    BeFileName nextPriorityTaskDir(taskDirectory);

    wchar_t stringBuffer[1000];
    swprintf(stringBuffer, L"\\%s%i\\", TASK_PRIORITY_FOLDER_NAME, priority);
    nextPriorityTaskDir.AppendToPath(stringBuffer);
    assert(nextPriorityTaskDir.DoesPathExist());

    BeDirectoryIterator dirIter(nextPriorityTaskDir);

    BeFileName name;
    bool isDir;    
        
    for (; SUCCESS == dirIter.GetCurrentEntry(name, isDir); dirIter.ToNext())
        {   
        BeFileName newFileName(taskDirectory);
        newFileName.AppendToPath(name.GetFileNameAndExtension().c_str());
        BeFileNameStatus status = BeFileName::BeMoveFile(name, newFileName);
        assert(BeFileNameStatus::Success == status);
        }    

    BeFileNameStatus status = BeFileName::EmptyAndRemoveDirectory(nextPriorityTaskDir.c_str());
    assert(BeFileNameStatus::Success == status);

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

static bool s_doMultiThreadFiltering = true;
static bool s_doMultiThreadMeshing = true;
static bool s_doMultiThreadStitching = true;

StatusInt IScalableMeshSourceCreatorWorker::Impl::ProcessGenerateTask(BeXmlNodeP pXmlTaskNode)
    {
    BeXmlNodeP pChildNode = pXmlTaskNode->GetFirstChild();

    if (pChildNode == nullptr)
        return SUCCESS;
    assert(m_pDataIndex.GetPtr() == nullptr);



    GetScalableMeshFileLock(true);
    
#ifndef NDEBUG

    if (m_mainFilePtr == nullptr)
        m_mainFilePtr = GetFile(true);
                
    if (m_mainFilePtr != nullptr)
        m_mainFilePtr->GetDb()->SetCanReopenShared(true);

    if (m_sisterFilePtr != nullptr)
        m_sisterFilePtr->GetDb()->SetCanReopenShared(true);    


#endif


    HFCPtr<MeshIndexType> pDataIndex(GetDataIndex());

    ReleaseScalableMeshFileLock();
    
#ifndef NDEBUG
    if (m_mainFilePtr != nullptr)
        m_mainFilePtr->GetDb()->SetCanReopenShared(false);

    if (m_sisterFilePtr != nullptr)
        m_sisterFilePtr->GetDb()->SetCanReopenShared(false);    
#endif


    ScalableMeshQuadTreeBCLIBMeshFilter1<DPoint3d, DRange3d>* filter = dynamic_cast<ScalableMeshQuadTreeBCLIBMeshFilter1<DPoint3d, DRange3d>*>(pDataIndex->GetFilter());
    if (filter != nullptr)
        {
        filter->SetIsMultiProcessGeneration(true);
        }
        
    bvector<HFCPtr<SMMeshIndexNode<DPoint3d, DRange3d>>>     generatedNodes;
    bvector<RefCountedPtr<SMMemoryPoolVectorItem<DPoint3d>>> generatedPtsNeighbors;

    bvector<HFCPtr<SMMeshIndexNode<DPoint3d, DRange3d>>>     nodesToMesh;
    bvector<HFCPtr<SMPointIndexNode<DPoint3d, DRange3d>>>    childrenNodesForFiltering;
    bvector<RefCountedPtr<SMMemoryPoolVectorItem<DPoint3d>>> ptsNeighbors;

    SMMeshDataToLoad meshDataToLoad;
    meshDataToLoad.m_features = true;
    meshDataToLoad.m_graph = true;

    SMMeshDataToLoad meshDataToFiltering;
    meshDataToFiltering.m_ptIndices = false;
    meshDataToFiltering.m_features = true;
                        
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

        //TRACEPOINT(EventType::WORKER_MESH_TASK, tileId, (uint64_t)-1, -1, -1, 0, 0)

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

            //TRACEPOINT(EventType::WORKER_MESH_TASK, tileId, (uint64_t)-1, -1, -1, 0, 0)

            idAttrs.clear();
            BeStringUtilities::ParseArguments(idAttrs, attrStr.c_str(), L",");

            for (auto& idStr : idAttrs)
                {
                stitchTileIds.push_back(BeStringUtilities::ParseUInt64(Utf8String(idStr).c_str()));                
                }
            }

        bool needFiltering; 
        bool needMeshing; 

        xmlStatus = pChildNode->GetAttributeBooleanValue(needFiltering, "needFiltering");
        assert(xmlStatus == BEXML_Success);

        xmlStatus = pChildNode->GetAttributeBooleanValue(needMeshing, "needMeshing");
        assert(xmlStatus == BEXML_Success);

        
        GetScalableMeshFileLock(true);
        
 
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
        
            //assert(!meshNode->IsLoaded());
            //meshNode->Unload();
            meshNode->Load();            
            meshNode->LoadData(&meshDataToLoad);

            //
            if (needFiltering)
                {
                HFCPtr<SMPointIndexNode<DPoint3d, DRange3d>> childNode(meshNode->GetSubNodeNoSplit());

                if (childNode != nullptr)
                    {
                    bool found = false;
                    for (auto& node : generatedNodes)
                        {       
                        if (childNode->GetBlockID().m_integerID == node->GetBlockID().m_integerID)
                            {
                            found = true;
                            break;
                            }
                        }

                    if (!found)
                        {
                        childNode->NeedToLoadNeighbors(false);
                        childNode->Load();
                        childNode->LoadData(&meshDataToFiltering);
                        childrenNodesForFiltering.push_back(childNode);
                        }
                    }
                else
                    {
                    vector<HFCPtr<SMPointIndexNode<DPoint3d, DRange3d>>> childrenNodes(meshNode->GetSubNodes());
                    for (auto& childNode : childrenNodes)
                        {             
                        bool found = false;
                        for (auto& node : generatedNodes)
                            {            
                            if (childNode->GetBlockID().m_integerID == node->GetBlockID().m_integerID)
                                {
                                found = true;
                                break;
                                }
                            }

                        if (!found)
                            {
                            childNode->NeedToLoadNeighbors(false);
                            childNode->Load();
                            childNode->LoadData(&meshDataToFiltering);
                            childrenNodesForFiltering.push_back(childNode);
                            }
                        }
                    }
                }

            ptsNeighbors.push_back(meshNode->GetPointsPtr());
                        
            nodesToMesh.push_back(meshNode);            
            }
        
        dbOpResult = m_smDb->CommitTransaction();
        assert(dbOpResult == true);    
        
        dbOpResult = m_smSisterDb->CommitTransaction();
        assert(dbOpResult == true);

        CloseSqlFiles();

        ReleaseScalableMeshFileLock();

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
                if (s_doMultiThreadFiltering)
                    {
                    RunOnNextAvailableThread(std::bind([] (SMPointIndexNode<DPoint3d, DRange3d>* node, size_t threadId) ->void
                        {
                        node->Filter((int)node->GetLevel(), nullptr);                
                        node->SetDirty(true);                            
                        SetThreadAvailableAsync(threadId);
                        }, node,std::placeholders::_1));
                    }
                else
                    {

#ifndef NDEBUG
                    //uint32_t pointCount = node->GetNbObjects();
#endif
                    node->Filter((int)node->GetLevel(), nullptr);                
                    node->SetDirty(true);

#ifndef NDEBUG
/*
                    if (!s_useThreadsInFiltering)
                        {
                        uint32_t newPointCount = node->GetNbObjects();
                        assert(pointCount <= newPointCount);
                        assert(newPointCount > 0);
                        assert(node->m_nodeHeader.m_nodeCount == newPointCount);
                        }
    */
#endif

                    }
                }

            if (s_doMultiThreadFiltering)
                {
                WaitForThreadStop();
                }
            }
            
        //Mesh tiles
        if (needMeshing)
            {
            for (auto& node : nodesToMesh)
                {
                assert(!node->m_nodeHeader.m_arePoints3d);

                if (s_doMultiThreadMeshing)
                    {
                    RunOnNextAvailableThread(std::bind([] (SMMeshIndexNode<DPoint3d, DRange3d>* node, HFCPtr<MeshIndexType> pDataIndex, size_t threadId ) ->void
                        {                        
                        bool isMeshed = pDataIndex->GetMesher2_5d()->Mesh(node);

                        if (isMeshed)
                            {
                            node->SetDirty(true);
                            }

                        SetThreadAvailableAsync(threadId);
                        }, node, pDataIndex, std::placeholders::_1));
                    }
                else
                    {
                    bool isMeshed = pDataIndex->GetMesher2_5d()->Mesh(node);

                    if (isMeshed)
                        {
                        node->SetDirty(true);
                        }
                    }
                }

             if (s_doMultiThreadMeshing)
                {
                WaitForThreadStop();
                }
            }

        //Stitch tiles
        vector<SMMeshIndexNode<DPoint3d, DRange3d>*> nodesToStitch;

        for (auto& tileId : stitchTileIds)
            {
            for (auto& node : nodesToMesh)
                {
                assert(!node->m_nodeHeader.m_arePoints3d);            

                if (node->GetBlockID().m_integerID == tileId)
                    {        
                    if (s_doMultiThreadStitching)
                        {                  
                        if (node->m_nodeHeader.m_level > 0)
                            nodesToStitch.push_back(node);
                        }
                    else
                        {                                    
    #ifndef NDEBUG
                        uint32_t pointCount = node->GetNbObjects();
                        assert(node->m_nodeHeader.m_nodeCount == pointCount);
    #endif

                        bool isStitched = pDataIndex->GetMesher2_5d()->Stitch(node);
                        assert(isStitched == true);
                    
                        if (isStitched)
                            {
                            node->SetDirty(true);
                            }

                         
    #ifndef NDEBUG
                        uint32_t newPointCount = node->GetNbObjects();
                        assert(pointCount <= newPointCount);
                        assert(newPointCount > 0 || pointCount == 0);
                        assert(node->m_nodeHeader.m_nodeCount == newPointCount);
    #endif
                        }

                    break;
                    }
                }
            }

        if (s_doMultiThreadStitching && nodesToStitch.size() > 0)
            {            
            if (nodesToStitch.size() <= 72)
                {                
                SetThreadingOptions(false, false, false);        

                for (auto& node : nodesToStitch)
                    {
#ifndef NDEBUG
                    uint32_t pointCount = node->GetNbObjects();
                    assert(node->m_nodeHeader.m_nodeCount == pointCount);
#endif

                    bool isStitched = pDataIndex->GetMesher2_5d()->Stitch(node);
                    assert(isStitched == true);
                    
                    if (isStitched)
                        {
                        node->SetDirty(true);
                        }
                         
    #ifndef NDEBUG
                    uint32_t newPointCount = node->GetNbObjects();
                    assert(pointCount <= newPointCount);
                    assert(newPointCount > 0 || pointCount == 0);
                    assert(node->m_nodeHeader.m_nodeCount == newPointCount);
    #endif
                    }
                
                SetThreadingOptions(false, true, false);                
                
                }
            else
                {
                assert(nodesToStitch.size() == stitchTileIds.size());
                m_pDataIndex->DoParallelStitching(nodesToStitch);
                }

#ifndef NDEBUG
            wchar_t text_buffer[1000] = { 0 }; //temporary buffer

            swprintf(text_buffer, _countof(text_buffer), L"NODES TO STITCH\r\n"); 
            OutputDebugStringW(text_buffer); // print
            
            for (auto& node : nodesToStitch)
                {                
                swprintf(text_buffer, _countof(text_buffer), L"Node ID %zd\r\n", node->GetBlockID().m_integerID); 
                OutputDebugStringW(text_buffer); // print
                }

            swprintf(text_buffer, _countof(text_buffer), L"NODES TO STITCH END\r\n"); 
            OutputDebugStringW(text_buffer); // print

#endif
         
            }

#ifndef NDEBUG
        if (!needFiltering && !needMeshing)
            {
            bool found = false;

            for (auto& node : nodesToMesh)
                {
                for (auto& tileId : stitchTileIds)
                    {                    
                    if (node->GetBlockID().m_integerID == tileId)
                        {
                        found = true;
                        break;
                        }
                    }

                if (!found)
                    assert(node->IsDirty() == false);
                /*
                else
                    assert(node->IsDirty() == true);
                    */
                }            
            }
#endif

        generatedNodes.insert(generatedNodes.end(), nodesToMesh.begin(), nodesToMesh.end());
        generatedPtsNeighbors.insert(generatedPtsNeighbors.end(), ptsNeighbors.begin(), ptsNeighbors.end());

        nodesToMesh.clear();
        ptsNeighbors.clear();

                 
        pChildNode = pChildNode->GetNextSibling();
        } while (pChildNode != nullptr);
            

    GetScalableMeshFileLock(false);

    
    //Flush all the data on disk    
    OpenSqlFiles(false, true);

    generatedPtsNeighbors.clear();    

    for (auto& node : childrenNodesForFiltering)
        {
        node->Discard();        
        node->Unload();
        }

    childrenNodesForFiltering.clear();

    for (auto& node : generatedNodes)
        {          
        node->Discard();
        node->Unload();
        }

    generatedNodes.clear();
    
    pDataIndex->Store();    

    CloseSqlFiles();

    FreeDataIndex();

    ReleaseScalableMeshFileLock();
        

    //m_pDataIndex->UnloadAllNodes();

    //m_pDataIndex = nullptr;

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


StatusInt IScalableMeshSourceCreatorWorker::Impl::ProcessTextureTask(BeXmlNodeP pXmlTaskNode)
    {
    uint64_t baseNodeId = 0;
    BeXmlStatus xmlStatus = pXmlTaskNode->GetAttributeUInt64Value(baseNodeId, "newBaseNodeId");    
    assert(xmlStatus == BEXML_Success);

    BeXmlNodeP pChildNode = pXmlTaskNode->GetFirstChild();
    
    GetScalableMeshFileLock(true);
    
#ifndef NDEBUG
    if (m_mainFilePtr == nullptr)
        m_mainFilePtr = GetFile(true);
                
    if (m_mainFilePtr != nullptr)
        m_mainFilePtr->GetDb()->SetCanReopenShared(true);

    if (m_sisterFilePtr != nullptr)
        m_sisterFilePtr->GetDb()->SetCanReopenShared(true);
#endif

    HFCPtr<MeshIndexType> pDataIndex(GetDataIndex());
    pDataIndex->SetNextID(baseNodeId);

    ReleaseScalableMeshFileLock();    

    bvector<HFCPtr<SMMeshIndexNode<DPoint3d, DRange3d>>> nodesToMesh;
    bvector<HFCPtr<SMMeshIndexNode<DPoint3d, DRange3d>>> generatedNodes;

    SMMeshDataToLoad meshDataToLoad;

    meshDataToLoad.m_features = false;
    meshDataToLoad.m_graph = false;
    meshDataToLoad.m_ptIndices = true;
    meshDataToLoad.m_textureIndices = true;
    meshDataToLoad.m_texture = true;
    
   /* ITextureProviderPtr textureStreamProviderPtr;
    WString url(L"http://www.bing.com/maps/Aerial");
    IScalableMeshPtr nullSmPtr;
    GetStreamedTextureProvider(textureStreamProviderPtr, nullSmPtr, pDataIndex, url);*/

    ITextureProviderPtr textureProviderPtr;
    IScalableMeshPtr nullSmPtr;
    GetTextureProvider(textureProviderPtr, nullSmPtr, pDataIndex);   

    bvector<HFCPtr<SMMeshIndexNode<DPoint3d, DRange3d>>> nodesCreated;
    do
        {
        assert(pChildNode != nullptr);
        assert(Utf8String(pChildNode->GetName()).CompareTo("TextureTiles") == 0);

        bvector<uint64_t> tileIds;
        WString           attrStr;

        BeXmlStatus xmlStatus = pChildNode->GetAttributeStringValue(attrStr, "ids");
        assert(xmlStatus == BEXML_Success);

        bvector<WString> idAttrs;
        BeStringUtilities::ParseArguments(idAttrs, attrStr.c_str(), L",");

        for (auto& idStr : idAttrs)
        {
            tileIds.push_back(BeStringUtilities::ParseUInt64(Utf8String(idStr).c_str()));
        }



        GetScalableMeshFileLock(true);

        assert(tileIds.size() > 0);

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

            meshNode->NeedToLoadNeighbors(false);

            meshNode->Load();
            meshNode->LoadData(&meshDataToLoad);

            nodesToMesh.push_back(meshNode);
            }

        dbOpResult = m_smDb->CommitTransaction();
        assert(dbOpResult == true);

        dbOpResult = m_smSisterDb->CommitTransaction();
        assert(dbOpResult == true);

        CloseSqlFiles();

        ReleaseScalableMeshFileLock();    

        for (auto& node : nodesToMesh)
        {
            bool hasNoChildren = false;
            node->DisableMultiThreadTexturingCutting();
            //node->TextureFromRaster(textureStreamProviderPtr);
            if (node->IsLeaf())
                hasNoChildren = true;
            node->TextureFromRaster(textureProviderPtr);
            if (hasNoChildren && !node->IsLeaf()) 
            {
                //add newly created nodes
                HFCPtr<SMPointIndexNode<DPoint3d, DRange3d>> subNodeNoSplit(node->GetSubNodeNoSplit());

                if (subNodeNoSplit != nullptr)
                {
                    nodesCreated.push_back((SMMeshIndexNode<DPoint3d, DRange3d>*)(&*subNodeNoSplit));
                }
                else
                {
                    vector<HFCPtr<SMPointIndexNode<DPoint3d, DRange3d>>> childrenNodes(node->GetSubNodes());

                    for (auto& childNode : childrenNodes)
                        nodesCreated.push_back((SMMeshIndexNode<DPoint3d, DRange3d>*)(&*childNode));
                }

            }
        }

        generatedNodes.insert(generatedNodes.end(), nodesToMesh.begin(), nodesToMesh.end());

        nodesToMesh.clear();
        pChildNode = pChildNode->GetNextSibling();
    } while (pChildNode != nullptr);





    do
    { 
        //at the moment just process newly created nodes in the same task
        for (auto& node : nodesToMesh)
        {
            bool hasNoChildren = false;
            node->DisableMultiThreadTexturingCutting();
            //node->TextureFromRaster(textureStreamProviderPtr);
            if (node->IsLeaf())
                hasNoChildren = true;
            node->TextureFromRaster(textureProviderPtr);
            if (hasNoChildren && !node->IsLeaf())
            {
                //add newly created nodes
                HFCPtr<SMPointIndexNode<DPoint3d, DRange3d>> subNodeNoSplit(node->GetSubNodeNoSplit());

                if (subNodeNoSplit != nullptr)
                {
                    nodesCreated.push_back((SMMeshIndexNode<DPoint3d, DRange3d>*)(&*subNodeNoSplit));
                }
                else
                {
                    vector<HFCPtr<SMPointIndexNode<DPoint3d, DRange3d>>> childrenNodes(node->GetSubNodes());

                    for (auto& childNode : childrenNodes)
                        nodesCreated.push_back((SMMeshIndexNode<DPoint3d, DRange3d>*)(&*childNode));
                }

            }
        }

        generatedNodes.insert(generatedNodes.end(), nodesToMesh.begin(), nodesToMesh.end());

        nodesToMesh.clear();

        GetScalableMeshFileLock(false);

        //Flush all the data on disk    
        OpenSqlFiles(false, true);

        for (auto& node : generatedNodes)
        {
            node->Discard();
            // pDataIndex->ClearNodeMap();
            node->Unload();
        }

        pDataIndex->Store();

        generatedNodes.clear();

        for (auto& node : nodesCreated)
        {
            node->NeedToLoadNeighbors(false);

            node->Load();
            node->LoadData(&meshDataToLoad);
            nodesToMesh.push_back(node);
        }

        nodesCreated.clear();
        
        CloseSqlFiles();

        ReleaseScalableMeshFileLock();    

    } while (!nodesToMesh.empty());

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

        TRACEPOINT(EventType::WORKER_MESH_TASK, tileId, (uint64_t)-1, -1, -1, 0, 0)

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

    SMSQLiteStore<PointIndexExtentType>* pSqliteStore(static_cast<SMSQLiteStore<PointIndexExtentType>*>(GetDataIndex()->GetDataStore().get()));
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

        TRACEPOINT(EventType::WORKER_MESH_TASK, tileId, (uint64_t)-1, -1, -1, 0, 0)
       
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

    SMSQLiteStore<PointIndexExtentType>* pSqliteStore(static_cast<SMSQLiteStore<PointIndexExtentType>*>(GetDataIndex()->GetDataStore().get()));
    assert(pSqliteStore != nullptr);
    pSqliteStore->SaveSisterFiles();

    return SUCCESS;    
    }

#endif

StatusInt IScalableMeshSourceCreatorWorker::Impl::OpenSqlFiles(bool readOnly, bool needSisterMainLockFile)
    {            
    if (m_smDb == nullptr || m_smSisterDb == nullptr)
        {
        m_mainFilePtr = GetFile(true);
        assert(m_mainFilePtr->IsShared() == true);
        
       
        if (m_smDb == nullptr)
            {
            m_smDb = m_mainFilePtr->GetDb();
            assert(m_smDb != nullptr);
            }
        
        if (m_smSisterDb == nullptr)
            {                        
            SMSQLiteStore<PointIndexExtentType>* pSqliteStore(static_cast<SMSQLiteStore<PointIndexExtentType>*>(GetDataIndex()->GetDataStore().get()));
            assert(pSqliteStore != nullptr);


            m_sisterFilePtr = pSqliteStore->GetSisterSQLiteFile(SMStoreDataType::LinearFeature, true, false);

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
                
#ifndef NDEBUG
    m_smDb->SetCanReopenShared(true);
    m_smSisterDb->SetCanReopenShared(true);    
#endif

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

#ifndef NDEBUG
    m_smDb->SetCanReopenShared(false);
    m_smSisterDb->SetCanReopenShared(false);    
#endif


    return SUCCESS;
    }

void IScalableMeshSourceCreatorWorker::Impl::SetShareable(bool isShareable)
    {
    if (isShareable)
        {
//#if 0
        SMSQLiteFilePtr sqlFilePtr(GetFile(true));
        assert(sqlFilePtr.IsValid() && sqlFilePtr->GetDb() != nullptr);

        if (sqlFilePtr->GetDb()->IsDbOpen())
            sqlFilePtr->GetDb()->CloseDb();

        sqlFilePtr->SetIsShared(true);
        
        SMSQLiteStore<PointIndexExtentType>* pSqliteStore(static_cast<SMSQLiteStore<PointIndexExtentType>*>(GetDataIndex()->GetDataStore().get()));
        assert(pSqliteStore != nullptr);

        SMSQLiteFilePtr sisterFilePtr(pSqliteStore->GetSisterSQLiteFile(SMStoreDataType::LinearFeature, true, false));
        assert(sisterFilePtr.IsValid() && sisterFilePtr->GetDb() != nullptr);
            
        if (sisterFilePtr->GetDb()->IsDbOpen())
            sisterFilePtr->GetDb()->CloseDb();

        sisterFilePtr->SetIsShared(true);
//#endif

        __super::SetShareable(true);
        m_isShareable = true;        
        ScalableMeshDb::SetEnableSharedDatabase(true);    
        }
    else
        {
        __super::SetShareable(false);
        ScalableMeshDb::SetEnableSharedDatabase(false);    
        }
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

        TRACEPOINT(EventType::WORKER_STITCH_TASK, tileId, (uint64_t)-1, -1, -1, 0, 0)
                
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
            TRACEPOINT(EventType::WORKER_STITCH_TASK_NEIGHBOR, node->GetBlockID().m_integerID, (uint64_t)-1, -1, -1, 0, 0)

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
                    
        TRACEPOINT(EventType::WORKER_FILTER_TASK, tileId, (uint64_t)-1, -1, -1, 0, 0)    

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
    
    if (Utf8String(pTaskNode->GetName()).CompareTo("stitch") == 0 || Utf8String(pTaskNode->GetName()).CompareTo("filter") == 0)
        {
        uint32_t resInd;

        BeXmlStatus xmlStatus = pTaskNode->GetAttributeUInt32Value(resInd, "res");
        assert(xmlStatus == BEXML_Success);

        if (Utf8String(pTaskNode->GetName()).CompareTo("filter") == 0)
            {
            CreateFilterTasks(resInd);
            }
        else
            {
            CreateStitchTasks(resInd);
            }    
        }
    else
    if (Utf8String(pTaskNode->GetName()).CompareTo("copyNextPriorityTasks") == 0)
        {
        uint32_t priority;

        BeXmlStatus xmlStatus = pTaskNode->GetAttributeUInt32Value(priority, "priority");
        assert(xmlStatus == BEXML_Success);        

        CopyNextPriorityTasks(priority);
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
