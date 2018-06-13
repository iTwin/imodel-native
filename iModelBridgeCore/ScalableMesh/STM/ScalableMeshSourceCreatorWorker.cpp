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

#include <BeXml/BeXml.h>
#include "ImagePPHeaders.h"
#include "ScalableMeshSourceCreatorWorker.h"

USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

StatusInt IScalableMeshSourceCreatorWorker::CreateMeshTasks() const
    {    
    return static_cast<IScalableMeshSourceCreatorWorker::Impl*>(m_implP.get())->CreateMeshTasks();
    }

StatusInt IScalableMeshSourceCreatorWorker::ProcessMeshTask(BeXmlNodeP pXmlTaskNode) const
    {
    return static_cast<IScalableMeshSourceCreatorWorker::Impl*>(m_implP.get())->ProcessMeshTask(pXmlTaskNode);
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
    }

IScalableMeshSourceCreatorWorker::Impl::Impl(const IScalableMeshPtr& scmPtr)
    : IScalableMeshSourceCreator::Impl(scmPtr)
    {      
    }

IScalableMeshSourceCreatorWorker::Impl::~Impl()
    {    
    m_pDataIndex = nullptr;
    }

HFCPtr<MeshIndexType> IScalableMeshSourceCreatorWorker::Impl::GetDataIndex()
    {
    if (m_pDataIndex.GetPtr() == nullptr)
        {       
        StatusInt status = IScalableMeshCreator::Impl::CreateDataIndex(m_pDataIndex, true, SM_ONE_SPLIT_THRESHOLD);

        assert(m_pDataIndex.GetPtr() != nullptr);   
        assert(status == SUCCESS);
        }

    return m_pDataIndex;
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
        
#if 0 
        FILE* pTaskFile = _wfopen(meshTaskFile.c_str(), L"wb+");

        int nbChars = swprintf(stringBuffer, L"<?xml version =\"1.0\" encoding=\"utf-8\"?>\r\n");
        fwrite(stringBuffer, sizeof(WChar), nbChars, pTaskFile);

        nbChars = swprintf(stringBuffer, L"<workerTask type =\"mesh\">\r\n");
        fwrite(stringBuffer, sizeof(WChar), nbChars, pTaskFile);

        nbChars = swprintf(stringBuffer, L"<tile id =\"%zi\"/>\r\n", nodeId);
        fwrite(stringBuffer, sizeof(WChar), nbChars, pTaskFile);

        nbChars = swprintf(stringBuffer, L"</workerTask>\r\n");
        fwrite(stringBuffer, sizeof(WChar), nbChars, pTaskFile);

        fclose(pTaskFile);
#endif

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


StatusInt IScalableMeshSourceCreatorWorker::Impl::ProcessMeshTask(BeXmlNodeP pXmlTaskNode)
    {
    BeXmlNodeP pChildNode = pXmlTaskNode->GetFirstChild();

    assert(pChildNode != nullptr);
    assert(Utf8String(pChildNode->GetName()).CompareTo("tile") == 0);
    
    uint64_t tileId;

    BeXmlStatus xmlStatus = pChildNode->GetAttributeUInt64Value(tileId, "id");    
    assert(xmlStatus == BEXML_Success);
    assert(pChildNode->GetNextSibling() == nullptr);

    HFCPtr<MeshIndexType> pDataIndex(GetDataIndex());
    
    HPMBlockID blockID(tileId);    

    HFCPtr<SMMeshIndexNode<DPoint3d, DRange3d>> meshNode((SMMeshIndexNode<DPoint3d, DRange3d>*)pDataIndex->CreateNewNode(blockID, false).GetPtr());

    meshNode->Load();

    assert(!meshNode->m_nodeHeader.m_arePoints3d);

    bool isMeshed = pDataIndex->GetMesher2_5d()->Mesh(meshNode);

    if (isMeshed)
        {
        meshNode->SetDirty(true);
        }

    meshNode = nullptr;

    return SUCCESS;    
    }



END_BENTLEY_SCALABLEMESH_NAMESPACE