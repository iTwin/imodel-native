/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "SMUnitTestDisplayQuery.h"
#include <Bentley/BeThread.h>
#include <ScalableMesh/IScalableMeshProgressiveQuery.h>



USING_NAMESPACE_BENTLEY_SCALABLEMESH

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct SmCachedDisplayTexture
    {
    SmCachedDisplayTexture()
        {    
        }    

    int dummy; 
    };

struct SmCachedDisplayMesh
    {    
    int dummy;
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE

struct ScalableMeshDisplayCacheManager : public IScalableMeshDisplayCacheManager
    {   
private:

    int m_nbCreatedMesh; 
    int m_nbDestroyedMesh;
    int m_nbCreatedTexture;
    int m_nbDestroyedTexture;    
    int m_nbDestroyedVideoMesh;
    int m_nbDestroyedVideoTexture;

public:

    int GetNbCreatedMesh()
        {
        return m_nbCreatedMesh;
        }

    int GetNbDestroyedMesh()
        {
        return m_nbDestroyedMesh;
        }

    int GetNbCreatedTexture()
        {
        return m_nbCreatedTexture;
        }

    int GetNbDestroyedTexture()
        {
        return m_nbDestroyedTexture;
        }    
 
    int GetNbDestroyedVideoMesh()
        {
        return m_nbDestroyedVideoMesh;
        }

    int GetNbDestroyedVideoTexture()
        {
        return m_nbDestroyedVideoTexture;
        }


    //Inherited from IScalableMeshDisplayCacheManager
    virtual BentleyStatus _CreateCachedMesh(SmCachedDisplayMesh*&   cachedDisplayMesh,
        size_t&                 usedMemInBytes,
        bool&                   isStoredOnGpu,
        size_t                  nbVertices,
        DPoint3d const*         positionOrigin,
        float*                  positions,
        float*                  normals,
        int                     nbTriangles,
        int*                    indices,
        float*                  params,
        SmCachedDisplayTexture* cachedTexture,
        uint64_t                nodeId,
        uint64_t                smId) override;

    virtual BentleyStatus _DestroyCachedMesh(SmCachedDisplayMesh* cachedDisplayMesh) override;

    virtual BentleyStatus _CreateCachedTexture(SmCachedDisplayTexture*& cachedDisplayTexture,
        size_t&                  usedMemInBytes,
        bool&                    isStoredOnGpu,
        int                      xSize,
        int                      ySize,
        int                      enableAlpha,
        int                      format,      // => see QV_*_FORMAT definitions above
        unsigned char const *    texels) override; // => texel image)

    virtual BentleyStatus _DestroyCachedTexture(SmCachedDisplayTexture* cachedDisplayTexture) override;

    virtual BentleyStatus _DeleteFromVideoMemory(SmCachedDisplayMesh* cachedDisplayMesh) override;

    virtual BentleyStatus _DeleteFromVideoMemory(SmCachedDisplayTexture* cachedDisplayTex) override;

    virtual bool _IsUsingVideoMemory() override;

    virtual bool _HasCompatibleSettings(SmCachedDisplayMesh* cachedDisplayMesh) override;

    ScalableMeshDisplayCacheManager(/*ViewContextR viewContext*/);

    ~ScalableMeshDisplayCacheManager();
};


//Inherited from IScalableMeshDisplayCacheManager
BentleyStatus ScalableMeshDisplayCacheManager::_CreateCachedMesh(SmCachedDisplayMesh*&   cachedDisplayMesh,
                                                                 size_t&                 usedMemInBytes,
                                                                 bool&                   isStoredOnGpu,
                                                                 size_t                  nbVertices,
                                                                 DPoint3d const*         positionOrigin,
                                                                 float*                  positions,
                                                                 float*                  normals,
                                                                 int                     nbTriangles,
                                                                 int*                    indices,
                                                                 float*                  params,
                                                                 SmCachedDisplayTexture* cachedTexture,
                                                                 uint64_t                nodeId,
                                                                 uint64_t                smId)
    {

    m_nbCreatedMesh++;
    isStoredOnGpu = false;
    usedMemInBytes = sizeof(SmCachedDisplayMesh);
    cachedDisplayMesh = new SmCachedDisplayMesh;
    
#if 0
    QvTextureID textureId = 0;

    if (cachedTexture != 0)
    {
        textureId = cachedTexture->m_textureID;
    }

    std::unique_ptr<SmCachedDisplayMesh> qvCachedDisplayMesh(new SmCachedDisplayMesh);

    qvCachedDisplayMesh->m_qvElem = qv_beginElement(m_qvCache, 0, NULL);

    if (nbTriangles > 0) qv_addQuickTriMesh(qvCachedDisplayMesh->m_qvElem, 3 * nbTriangles, indices, (int)nbVertices, positionOrigin, reinterpret_cast <FloatXYZ*> (positions),
        reinterpret_cast <FloatXYZ*> (normals),
        reinterpret_cast <FloatXY*> (params), textureId, QV_QTMESH_GENNORMALS);
    qv_endElement(qvCachedDisplayMesh->m_qvElem);

    if (nbTriangles == 0) qvCachedDisplayMesh->m_qvElem = 0;

    cachedDisplayMesh = qvCachedDisplayMesh.release();

    isStoredOnGpu = false;
    usedMemInBytes = nbVertices * sizeof(float) * 3 + nbTriangles * 3 * sizeof(int32_t) + sizeof(float) * 2 * nbVertices;
#endif

    return SUCCESS;
}

BentleyStatus ScalableMeshDisplayCacheManager::_DestroyCachedMesh(SmCachedDisplayMesh* cachedDisplayMesh)
{
    m_nbDestroyedMesh++;
 
#if 0 
    // shutting down
    if (nullptr == DgnPlatformLib::QueryHost())
        return SUCCESS;

    if (NULL != cachedDisplayMesh->m_qvElem)
    {
        T_HOST.GetGraphicsAdmin()._DeleteQvElem(cachedDisplayMesh->m_qvElem);
        cachedDisplayMesh->m_qvElem = 0;
    }

    delete cachedDisplayMesh;
#endif

    return SUCCESS;
}

BentleyStatus ScalableMeshDisplayCacheManager::_CreateCachedTexture(SmCachedDisplayTexture*& cachedDisplayTexture,
    size_t&                  usedMemInBytes,
    bool&                    isStoredOnGpu,
    int                      xSize,
    int                      ySize,
    int                      enableAlpha,
    int                      format,      // => see QV_*_FORMAT definitions above
    unsigned char const *    texels)      // => texel image)
{
     
    m_nbCreatedTexture++;    
    isStoredOnGpu = false;
    usedMemInBytes = sizeof(SmCachedDisplayTexture);
    cachedDisplayTexture = new SmCachedDisplayTexture;


#if 0
    std::unique_ptr<SmCachedDisplayTexture> qvCachedDisplayTexture(new SmCachedDisplayTexture);

    qv_defineTileSection(qvCachedDisplayTexture->m_textureID, nullptr, 0, xSize, ySize, enableAlpha, format, texels);

    //qv_defineTexture(qvCachedDisplayTexture->m_textureID, NULL, (int)xSize, (int)ySize, enableAlpha, format, texels);

    cachedDisplayTexture = qvCachedDisplayTexture.release();

    isStoredOnGpu = false;
    usedMemInBytes = xSize * ySize * 6;
#endif
    return SUCCESS;
}

BentleyStatus ScalableMeshDisplayCacheManager::_DestroyCachedTexture(SmCachedDisplayTexture* cachedDisplayTexture)
{

    m_nbDestroyedTexture++;

#if 0 
    if (cachedDisplayTexture->m_textureID != 0)
    {
        qv_deleteTexture(cachedDisplayTexture->m_textureID);
        cachedDisplayTexture->m_textureID = 0;
    }

    delete cachedDisplayTexture;
#endif

    return SUCCESS;
}


BentleyStatus ScalableMeshDisplayCacheManager::_DeleteFromVideoMemory(SmCachedDisplayMesh* cachedDisplayMesh)
{
    m_nbDestroyedVideoMesh++;    
    return SUCCESS;
}

BentleyStatus ScalableMeshDisplayCacheManager::_DeleteFromVideoMemory(SmCachedDisplayTexture* cachedDisplayTex)
{
    m_nbDestroyedVideoTexture++;
    return SUCCESS;
}

bool ScalableMeshDisplayCacheManager::_IsUsingVideoMemory()
{
    return false;
}

bool ScalableMeshDisplayCacheManager::_HasCompatibleSettings(SmCachedDisplayMesh* cachedDisplayMesh) 
    {
    return true;
    }

ScalableMeshDisplayCacheManager::ScalableMeshDisplayCacheManager()
{
#if 0 
    m_qvCache = T_HOST.GetGraphicsAdmin()._CreateQvCache();
#endif

    m_nbCreatedMesh = 0;
    m_nbDestroyedMesh = 0;
    m_nbCreatedTexture = 0;
    m_nbDestroyedTexture = 0;

    m_nbDestroyedVideoMesh = 0;
    m_nbDestroyedVideoTexture = 0;
}


ScalableMeshDisplayCacheManager::~ScalableMeshDisplayCacheManager()
{

}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                 11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IScalableMeshProgressiveQueryEnginePtr DisplayQueryTester::GetProgressiveQueryEngine()
    {
    if (m_progressiveQueryEngine == nullptr)
        {
        m_displayCacheManager = new ScalableMeshDisplayCacheManager();
        
        m_progressiveQueryEngine = IScalableMeshProgressiveQueryEngine::Create(m_smPtr, m_displayCacheManager, m_displayTexture);                
        }

    return m_progressiveQueryEngine;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                 11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayQueryTester::DisplayQueryTester()
    {    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                 11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayQueryTester::~DisplayQueryTester()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                 12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayQueryTester::VerifyDisplayNodeFunctions(bvector<IScalableMeshCachedDisplayNodePtr>& meshNodes) const
    {    
    for (auto& node : meshNodes)
        { 
        node->LoadNodeHeader();
        ASSERT_EQ(node->IsHeaderLoaded(), true);

        bvector<SmCachedDisplayMesh*>  cachedMeshes;
        bvector<bpair<bool, uint64_t>> textureIds;
                
        StatusInt status = node->GetCachedMeshes(cachedMeshes, textureIds);
        
        ASSERT_EQ(status == SUCCESS || node->GetPointCount() == 0, true);        
                
        if (textureIds.size() > 0)
            {
            bvector<SmCachedDisplayTexture*> cachedTextures; 
            bvector<uint64_t>                textureRequestIds;

            size_t nbValidTextures = 0;

            for (auto& id : textureIds)
                {
                if (id.first)
                    nbValidTextures++;
                }
            
            if (nbValidTextures > 0)
                {             
                StatusInt statusGetTextures = node->GetCachedTextures(cachedTextures, textureRequestIds);
                ASSERT_EQ(statusGetTextures == SUCCESS, true);                
                EXPECT_EQ(textureIds.size() == textureRequestIds.size(), true);
                }                
            
            EXPECT_EQ(cachedTextures.size() == nbValidTextures, true);           
            }

        node->SetIsInVideoMemory(false);
        
        IScalableMeshCachedDisplayNodePtr displayNodePtr(IScalableMeshCachedDisplayNode::Create(node->GetNodeId(), m_smPtr.get()));
        }    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                 12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayQueryTester::VerifyCurrentlyViewedNodesFunctions(bvector<IScalableMeshCachedDisplayNodePtr>& meshNodes) 
    {
    bvector<IScalableMeshNodePtr> viewedNodes;

    m_smPtr->GetCurrentlyViewedNodes(viewedNodes);

    EXPECT_EQ(viewedNodes.size() == 0, true);

    for (auto& node : meshNodes)
        {
        viewedNodes.push_back(node.get());
        }
    
    m_smPtr->SetCurrentlyViewedNodes(viewedNodes);

    viewedNodes.clear();

    m_smPtr->GetCurrentlyViewedNodes(viewedNodes);

    EXPECT_EQ(viewedNodes.size() == meshNodes.size(), true);    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                 03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayQueryTester::SetActiveClips(const bset<uint64_t>& clips)
    {
    GetProgressiveQueryEngine()->SetActiveClips(clips, m_smPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                 03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayQueryTester::GetActiveClips(bset<uint64_t>& clips)
    {
    GetProgressiveQueryEngine()->GetActiveClips(clips, m_smPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                 11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool StopQueryCallback()
    {
    return false;
    }

void DisplayQueryTester::DoQuery()
    {

    InitializeProgressiveQueries();       

    IScalableMeshViewDependentMeshQueryParamsPtr viewDependentQueryParams(IScalableMeshViewDependentMeshQueryParams::CreateParams());
    
    viewDependentQueryParams->SetMinScreenPixelsPerPoint(m_minScreenPixelsPerPoint);
    viewDependentQueryParams->SetMaxPixelError(m_maxPixelError);
    viewDependentQueryParams->SetRootToViewMatrix(m_rootToViewMatrix);
    viewDependentQueryParams->SetViewClipVector(m_clipVector);

    DPoint3d dummyViewBox[8]; 
    viewDependentQueryParams->SetViewBox(dummyViewBox);
    viewDependentQueryParams->SetProgressiveDisplay(true);    
    viewDependentQueryParams->SetStopQueryCallback(&StopQueryCallback);


    int queryId = 0;
    bvector<bool> clips;
    const bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshCachedDisplayNodePtr> startingNodes;

    /*NEEDS_WORK_SM : Get clips
    m_DTMDataRef->GetVisibleClips(clips);
    */
    
    StatusInt status = GetProgressiveQueryEngine()->StartQuery(queryId,
                                                               viewDependentQueryParams,
                                                               startingNodes,
                                                               m_displayTexture, //No wireframe mode, so always load the texture.
                                                               clips,
                                                               m_smPtr);

    ASSERT_EQ(status == SUCCESS, true);

    if (m_waitQueryComplete)
        {
        while (!GetProgressiveQueryEngine()->IsQueryComplete(queryId))
            {
            BeThreadUtilities::BeSleep(200);
            }
        }
    
    ASSERT_EQ(GetProgressiveQueryEngine()->IsQueryComplete(queryId), true);
    
    bvector<IScalableMeshCachedDisplayNodePtr> meshNodes;
    bvector<IScalableMeshCachedDisplayNodePtr> overviewMeshNodes;


    bvector<IScalableMeshNodePtr> currentlyViewedNodes;

    m_smPtr->GetCurrentlyViewedNodes(currentlyViewedNodes);
    EXPECT_EQ(currentlyViewedNodes.size() == 0, true);
        
    status = GetProgressiveQueryEngine()->GetRequiredNodes(meshNodes, queryId);
    EXPECT_EQ(status == SUCCESS, true);
    
    status = GetProgressiveQueryEngine()->GetOverviewNodes(overviewMeshNodes, queryId);
    EXPECT_EQ(status == SUCCESS, true);
                                
    int nbReturnedNodes = (int)meshNodes.size();

    VerifyDisplayNodeFunctions(meshNodes);
    VerifyDisplayNodeFunctions(overviewMeshNodes);
    VerifyCurrentlyViewedNodesFunctions(meshNodes);

    overviewMeshNodes.clear();
    meshNodes.clear();
    
    status = GetProgressiveQueryEngine()->StopQuery(queryId);
    EXPECT_EQ(status == SUCCESS, true);

    GetProgressiveQueryEngine()->ClearOverviews(m_smPtr.get());
        
    status = GetProgressiveQueryEngine()->GetRequiredNodes(meshNodes, queryId);
    EXPECT_EQ(status != SUCCESS, true);

    status = GetProgressiveQueryEngine()->GetOverviewNodes(overviewMeshNodes, queryId);
    EXPECT_EQ(status != SUCCESS, true);
    
    EXPECT_EQ(overviewMeshNodes.size() == 0 && meshNodes.size() == 0, true);
   
    bool isTerrain = m_smPtr->IsTerrain();
    bool isTextured = m_smPtr->IsTextured();
        
    m_progressiveQueryEngine = nullptr;
    m_smPtr = nullptr;
    ClearProgressiveQueriesInfo();

    int nbExpectedNodes = (int)m_expectedResults[0];
    
    EXPECT_EQ(nbReturnedNodes == nbExpectedNodes, true);    
    EXPECT_EQ(((ScalableMeshDisplayCacheManager*)m_displayCacheManager.get())->GetNbCreatedMesh() >= nbReturnedNodes, true);
    EXPECT_EQ(((ScalableMeshDisplayCacheManager*)m_displayCacheManager.get())->GetNbDestroyedMesh() >= nbReturnedNodes, true);
    
    if (!isTextured)
        {
        EXPECT_EQ(((ScalableMeshDisplayCacheManager*)m_displayCacheManager.get())->GetNbCreatedTexture() == 0, true);
        EXPECT_EQ(((ScalableMeshDisplayCacheManager*)m_displayCacheManager.get())->GetNbDestroyedTexture() == 0, true);
        }
    else        
    if (isTerrain) //3D 3SM are sharing textures amongst multiple leaf with common ancestor.
        {
        EXPECT_EQ(((ScalableMeshDisplayCacheManager*)m_displayCacheManager.get())->GetNbCreatedTexture() >= nbReturnedNodes, true);
        EXPECT_EQ(((ScalableMeshDisplayCacheManager*)m_displayCacheManager.get())->GetNbDestroyedTexture() >= nbReturnedNodes, true);
        }
        
    EXPECT_EQ(((ScalableMeshDisplayCacheManager*)m_displayCacheManager.get())->GetNbCreatedMesh() == ((ScalableMeshDisplayCacheManager*)m_displayCacheManager.get())->GetNbDestroyedMesh(), true);
    EXPECT_EQ(((ScalableMeshDisplayCacheManager*)m_displayCacheManager.get())->GetNbCreatedTexture() == ((ScalableMeshDisplayCacheManager*)m_displayCacheManager.get())->GetNbDestroyedTexture(), true);    

    EXPECT_EQ(((ScalableMeshDisplayCacheManager*)m_displayCacheManager.get())->GetNbDestroyedVideoMesh() == 0, true);
    EXPECT_EQ(((ScalableMeshDisplayCacheManager*)m_displayCacheManager.get())->GetNbDestroyedVideoTexture() == 0, true);    

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                 11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayQueryTester::SetQueryParams(const BeFileName& smFileName, const DMatrix4d& rootToView, const bvector<DPoint4d>& clipPlanes, const bvector<double>& expectedResults)
    {
    StatusInt status;
    m_smPtr = ScalableMesh::IScalableMesh::GetFor(smFileName, true, true, status);
            
    if (m_smPtr == nullptr)
        return false;
            
    memcpy(&m_rootToViewMatrix, &rootToView.coff, sizeof(rootToView.coff));

    bvector<ClipPlane> clips(clipPlanes.size());

    for (size_t ind = 0; ind < clipPlanes.size(); ind++)
        {        
        DVec3d normal(DVec3d::From(clipPlanes[ind].x, clipPlanes[ind].y, clipPlanes[ind].z));

        clips[ind] = ClipPlane(normal, clipPlanes[ind].w);
        }
       
    ConvexClipPlaneSet convexClipPlaneSet(&clips[0], clips.size());

    ClipPlaneSet clipPlaneSet(convexClipPlaneSet);

    ClipPrimitivePtr clipPrimitive(ClipPrimitive::CreateFromClipPlanes(clipPlaneSet));

    m_clipVector = ClipVector::CreateFromPrimitive(clipPrimitive.get());

    m_expectedResults.resize(expectedResults.size());
    memcpy(&m_expectedResults[0], &expectedResults[0], sizeof(double) * expectedResults.size());

    return true;
    /*
    DMatrix4d& rootToView;
    bvector<DPoint4d>& clipPlanes;
    bvector<double>& expectedResults
    */

        

    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mathieu.St-Pierre   11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestDisplayQuery, ProgressiveQuery)
    {
    DisplayQueryTester queryTester;

    bool result = queryTester.SetQueryParams(GetFileName(), GetRootToViewMatrix(), GetClipPlanes(), GetExpectedResults());

    EXPECT_EQ(result == true, true);

    if (result)
        queryTester.DoQuery();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Richard.Bois   03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestDisplayQuery, ProgressiveQuery_SetGetActiveClips)
    {
    DisplayQueryTester queryTester;

    bool result = queryTester.SetQueryParams(GetFileName(), GetRootToViewMatrix(), GetClipPlanes(), GetExpectedResults());

    EXPECT_EQ(result == true, true);

    if (result)
        {
        bset<uint64_t> activeClips;
        activeClips.insert(0);
        activeClips.insert(1);
        activeClips.insert(2);
        activeClips.insert(3);
        queryTester.SetActiveClips(activeClips);

        bset<uint64_t> storedClips;
        queryTester.GetActiveClips(storedClips);

        EXPECT_TRUE(std::equal(storedClips.begin(), storedClips.end(), activeClips.begin()));
        }
    }

INSTANTIATE_TEST_CASE_P(ScalableMesh, ScalableMeshTestDisplayQuery, ::testing::ValuesIn(ScalableMeshGTestUtil::GetListOfDisplayQueryValues(BeFileName(SM_DISPLAY_QUERY_TEST_CASES))));
