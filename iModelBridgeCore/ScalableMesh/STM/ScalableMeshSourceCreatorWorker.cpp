/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshSourceCreatorWorker.cpp $
|    $RCSfile: ScalableMeshSourceCreator.cpp,v $
|   $Revision: 1.90 $
|       $Date: 2015/07/15 10:41:29 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>

#include <process.h>

#include <BeXml/BeXml.h>
#include "ImagePPHeaders.h"
#include "ScalableMeshSourceCreatorWorker.h"

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


IScalableMeshSourceCreatorWorkerPtr IScalableMeshSourceCreatorWorker::GetFor(const WChar*  filePath,
                                                                       StatusInt&      status)
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
    
    IScalableMeshSourceCreatorWorkerPtr pCreatorWorker = new IScalableMeshSourceCreatorWorker(new IScalableMeshSourceCreatorWorker::Impl(filePath));
    
    status = pCreatorWorker->m_implP->LoadFromFile();
    if (BSISUCCESS != status)
        return 0;
    
    return pCreatorWorker.get();
}


IScalableMeshSourceCreatorWorkerPtr IScalableMeshSourceCreatorWorker::GetFor(const IScalableMeshPtr& scmPtr,
                                                                             StatusInt&              status)
    {
    using namespace ISMStore;

    RegisterDelayedImporters();

    IScalableMeshSourceCreatorWorkerPtr pCreatorWorker = new IScalableMeshSourceCreatorWorker(new IScalableMeshSourceCreatorWorker::Impl(scmPtr));

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

IScalableMeshSourceCreatorWorker::Impl::Impl(const WChar* scmFileName)
    : IScalableMeshSourceCreator::Impl(scmFileName) 
    {        
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

IScalableMeshSourceCreatorWorker::Impl::Impl(const IScalableMeshPtr& scmPtr)
    : IScalableMeshSourceCreator::Impl(scmPtr)
    {      
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

StatusInt IScalableMeshSourceCreatorWorker::Impl::CreateMeshTasks()
    {
    BeFileName taskDirectory(m_scmFileName);

    taskDirectory = taskDirectory.GetDirectoryName();

    HFCPtr<MeshIndexType> pDataIndex(GetDataIndex());                

    bvector<uint64_t> nodesToMesh;

    wchar_t stringBuffer[100000];

    pDataIndex->Mesh(&nodesToMesh);
    
    for (auto& nodeId : nodesToMesh)
        { 
        BeFileName meshTaskFile(taskDirectory);

        swprintf(stringBuffer, L"Mesh%zi.xml", nodeId);
        meshTaskFile.AppendString(stringBuffer);

        BeXmlDomPtr xmlDomPtr(BeXmlDom::CreateEmpty());
        BeXmlNodeP workerNode(xmlDomPtr->AddNewElement("workerTask", nullptr, nullptr));
        workerNode->AddAttributeStringValue("type", "mesh");

        BeXmlNodeP tileNode(xmlDomPtr->AddNewElement("tile", nullptr, workerNode));
        tileNode->AddAttributeUInt64Value("id", nodeId);

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

    for (auto& meshNode : nodesToStitchInfo)
        {
        BeFileName meshTaskFile(taskDirectory);

        uint64_t nodeId = meshNode->GetBlockID().m_integerID;

        swprintf(stringBuffer, L"Stitch%zi.xml", nodeId);
        meshTaskFile.AppendString(stringBuffer);

        BeXmlDomPtr xmlDomPtr(BeXmlDom::CreateEmpty());
        BeXmlNodeP workerNode(xmlDomPtr->AddNewElement("workerTask", nullptr, nullptr));
        workerNode->AddAttributeStringValue("type", "stitch");

        BeXmlNodeP tileNode(xmlDomPtr->AddNewElement("tile", nullptr, workerNode));
        tileNode->AddAttributeUInt64Value("id", nodeId);

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

    for (auto& meshNode : nodesToFilter)
    {
        BeFileName meshTaskFile(taskDirectory);

        uint64_t nodeId = meshNode->GetBlockID().m_integerID;

        swprintf(stringBuffer, L"Filter%zi.xml", nodeId);
        meshTaskFile.AppendString(stringBuffer);

        BeXmlDomPtr xmlDomPtr(BeXmlDom::CreateEmpty());
        BeXmlNodeP workerNode(xmlDomPtr->AddNewElement("workerTask", nullptr, nullptr));
        workerNode->AddAttributeStringValue("type", "filter");

        BeXmlNodeP tileNode(xmlDomPtr->AddNewElement("tile", nullptr, workerNode));
        tileNode->AddAttributeUInt64Value("id", nodeId);

        BeXmlDom::ToStringOption toStrOption = (BeXmlDom::ToStringOption)(BeXmlDom::TO_STRING_OPTION_Formatted | BeXmlDom::TO_STRING_OPTION_Indent);

        BeXmlStatus status = xmlDomPtr->ToFile(meshTaskFile, toStrOption, BeXmlDom::FILE_ENCODING_Utf8);
        assert(status == BEXML_Success);
        }

   // pDataIndex->ClearNodeMap();

    return SUCCESS;
    }



StatusInt IScalableMeshSourceCreatorWorker::Impl::ProcessMeshTask(BeXmlNodeP pXmlTaskNode)
    {
    BeXmlNodeP pChildNode = pXmlTaskNode->GetFirstChild();

    assert(pChildNode != nullptr);
    assert(Utf8String(pChildNode->GetName()).CompareTo("tile") == 0);
    
    uint64_t tileId;

    BeXmlStatus xmlStatus = pChildNode->GetAttributeUInt64Value(tileId, "id");    
    assert(xmlStatus == BEXML_Success);
    assert(pChildNode->GetNextSibling() == nullptr);

    TRACEPOINT(THREAD_ID(), EventType::WORKER_MESH_TASK, tileId, (uint64_t)-1, -1, -1, 0, 0)

    HFCPtr<MeshIndexType> pDataIndex(GetDataIndex());
    
    HPMBlockID blockID(tileId);    

    HFCPtr<SMMeshIndexNode<DPoint3d, DRange3d>> meshNode((SMMeshIndexNode<DPoint3d, DRange3d>*)pDataIndex->CreateNewNode(blockID, false, true).GetPtr());
    
    meshNode->NeedToLoadNeighbors(true);
    meshNode->Load();

    assert(!meshNode->m_nodeHeader.m_arePoints3d);

    bool isMeshed = pDataIndex->GetMesher2_5d()->Mesh(meshNode);

    if (isMeshed)
        {
        meshNode->SetDirty(true);
        }

    meshNode->Discard();
   // pDataIndex->ClearNodeMap();
    pDataIndex->Store();
    m_smSQLitePtr->Save();

    SMSQLiteStore<PointIndexExtentType>* pSqliteStore(static_cast<SMSQLiteStore<PointIndexExtentType>*>(m_pDataIndex->GetDataStore().get()));
    assert(pSqliteStore != nullptr);
    pSqliteStore->SaveSisterFiles();

    meshNode->Unload();
    meshNode = nullptr;

    return SUCCESS;    
    }





StatusInt IScalableMeshSourceCreatorWorker::Impl::ProcessStitchTask(BeXmlNodeP pXmlTaskNode)
    {
    BeXmlNodeP pChildNode = pXmlTaskNode->GetFirstChild();

    assert(pChildNode != nullptr);
    assert(Utf8String(pChildNode->GetName()).CompareTo("tile") == 0);

    uint64_t tileId;

    BeXmlStatus xmlStatus = pChildNode->GetAttributeUInt64Value(tileId, "id");
    assert(xmlStatus == BEXML_Success);
    assert(pChildNode->GetNextSibling() == nullptr);

    HFCPtr<MeshIndexType> pDataIndex(GetDataIndex());

    TRACEPOINT(THREAD_ID(), EventType::WORKER_STITCH_TASK, tileId, (uint64_t)-1, -1, -1, 0, 0)
                
    HPMBlockID blockID(tileId);


    BeDuration sleeper(BeDuration::FromSeconds(0.5));

    BeFileName lockFileName;
    GetSisterMainLockFileName(lockFileName);

 
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

    SMSQLiteStore<PointIndexExtentType>* pSqliteStore(static_cast<SMSQLiteStore<PointIndexExtentType>*>(m_pDataIndex->GetDataStore().get()));
    assert(pSqliteStore != nullptr);
    


    
    SMSQLiteFilePtr sqliteFilePtr(GetFile(true));

    SMSQLiteFilePtr sisterFilePtr(pSqliteStore->GetSisterSQLiteFile(SMStoreDataType::LinearFeature, false, true));
    
    FILE* lockFile;

    while ((lockFile = _wfsopen(lockFileName, L"ab+", _SH_DENYRW)) == nullptr)
        {
        sleeper.Sleep();
        }

    ScalableMeshDb* smDb(sqliteFilePtr->GetDb());
    assert(smDb != nullptr);
    
    bool dbOpResult = true;
   
    if (!smDb->IsDbOpen() || !smDb->IsReadonly())
        dbOpResult = smDb->ReOpenShared(true, true);
    
    assert(dbOpResult == true);
    

    ScalableMeshDb* smSisterDb(sisterFilePtr->GetDb());
    assert(smSisterDb != nullptr);    

    if (!smSisterDb->IsDbOpen() || !smSisterDb->IsReadonly())
        dbOpResult = smSisterDb->ReOpenShared(true, true);

    assert(dbOpResult == true);

    fclose(lockFile);

        
    vector<SMPointIndexNode<DPoint3d, DRange3d>*> neighborNodes;

    meshNode->GetAllNeighborNodes(neighborNodes);    
    dbOpResult = smDb->StartTransaction();
    assert(dbOpResult == true);

    dbOpResult = smSisterDb->StartTransaction();
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

    bool isMeshed = pDataIndex->GetMesher2_5d()->Stitch(meshNode);

   
    bool wasTransactionAbandoned;

    dbOpResult = smDb->CommitTransaction();
    assert(dbOpResult == true);    
    smDb->CloseShared(wasTransactionAbandoned);
    assert(wasTransactionAbandoned == false);

    dbOpResult = smSisterDb->CommitTransaction();
    assert(dbOpResult == true);
    smSisterDb->CloseShared(wasTransactionAbandoned);
    assert(wasTransactionAbandoned == false);
    

    //Ensure that loaded neighbor node and data are in sync with each other - END    
    /*
    fclose(lockFile);

    while ((lockFile = _wfsopen(lockFileName, L"ab+", _SH_DENYRW)) == nullptr)
        {
        sleeper.Sleep();
        }
      */          


    
    while ((lockFile = _wfsopen(lockFileName, L"ab+", _SH_DENYRW)) == nullptr)
        {
        sleeper.Sleep();
        }

    if (!smDb->IsDbOpen() || !smDb->IsReadonly())
        dbOpResult = smDb->ReOpenShared(false, true);

    if (!smSisterDb->IsDbOpen() || !smSisterDb->IsReadonly())
        dbOpResult = smSisterDb->ReOpenShared(false, true);
        
  
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

    
    if (isMeshed)
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

    meshNode->Discard();    
    meshNode->Unload();
    
   // pDataIndex->ClearNodeMap();
    pDataIndex->Store();
    sqliteFilePtr->Save();
    sisterFilePtr->Save();
  
    pSqliteStore->SaveSisterFiles();
    
    
    meshNode = nullptr;

    if (smDb->IsDbOpen())
        {
        smDb->CloseShared(wasTransactionAbandoned);
        assert(wasTransactionAbandoned == false);
        }

    if (smSisterDb->IsDbOpen())
        {
        smSisterDb->CloseShared(wasTransactionAbandoned);
        assert(wasTransactionAbandoned == false);
        }
    
    fclose(lockFile);
    
    return SUCCESS;
    }


StatusInt IScalableMeshSourceCreatorWorker::Impl::ProcessFilterTask(BeXmlNodeP pXmlTaskNode)
    {
    BeXmlNodeP pChildNode = pXmlTaskNode->GetFirstChild();

    assert(pChildNode != nullptr);
    assert(Utf8String(pChildNode->GetName()).CompareTo("tile") == 0);

    uint64_t tileId;

    BeXmlStatus xmlStatus = pChildNode->GetAttributeUInt64Value(tileId, "id");
    assert(xmlStatus == BEXML_Success);
    assert(pChildNode->GetNextSibling() == nullptr);
            
    TRACEPOINT(THREAD_ID(), EventType::WORKER_FILTER_TASK, tileId, (uint64_t)-1, -1, -1, 0, 0)    

    HFCPtr<MeshIndexType> pDataIndex(GetDataIndex());

    HPMBlockID blockID(tileId);

    HFCPtr<SMMeshIndexNode<DPoint3d, DRange3d>> meshNode((SMMeshIndexNode<DPoint3d, DRange3d>*)pDataIndex->CreateNewNode(blockID, false, true).GetPtr());

    meshNode->Unload();
    meshNode->NeedToLoadNeighbors(true);
    meshNode->Load();

    assert(!meshNode->m_nodeHeader.m_arePoints3d);

    //Ensure that all children nodes are reloaded so that it the contains the latest data which might have been generated by another worker.    
    HFCPtr<SMPointIndexNode<DPoint3d, DRange3d>> subNodeNoSplit(meshNode->GetSubNodeNoSplit());

    if (subNodeNoSplit != nullptr)
        {
        assert(!subNodeNoSplit->IsDirty());
        subNodeNoSplit->Unload();
        subNodeNoSplit->RemoveNonDisplayPoolData();
        meshNode->NeedToLoadNeighbors(true);
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
            meshNode->NeedToLoadNeighbors(true);    
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

    return SUCCESS;
    }


StatusInt IScalableMeshSourceCreatorWorker::Impl::CreateTaskPlan()
    {        
    BeXmlDomPtr xmlDomPtr(BeXmlDom::CreateEmpty());
    BeXmlNodeP taskPlanNode(xmlDomPtr->AddNewElement("taskPlan", nullptr, nullptr));    
        
    HFCPtr<MeshIndexType> pDataIndex(GetDataIndex());    
    uint32_t nbRes = (uint32_t)pDataIndex->GetDepth();

    for (uint32_t resInd = nbRes; resInd > 0; resInd--)
        {                                             
        BeXmlNodeP stitchTaskNode(xmlDomPtr->AddNewElement("stitch", nullptr, taskPlanNode));
        stitchTaskNode->AddAttributeUInt32Value("res", resInd);

        BeXmlNodeP filterTaskNode(xmlDomPtr->AddNewElement("filter", nullptr, taskPlanNode));
        filterTaskNode->AddAttributeUInt32Value("res", resInd - 1);        
        }

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