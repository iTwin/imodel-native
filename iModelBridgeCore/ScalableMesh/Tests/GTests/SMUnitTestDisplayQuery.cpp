/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/GTests/SMUnitTestDisplayQuery.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "SMUnitTestDisplayQuery.h"
#include <ScalableMesh\IScalableMeshProgressiveQuery.h>

USING_NAMESPACE_BENTLEY_SCALABLEMESH

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct SmCachedDisplayTexture
    {
    SmCachedDisplayTexture()
        {    
        }    
    };

struct SmCachedDisplayMesh
    {    
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE

struct ScalableMeshDisplayCacheManager : public IScalableMeshDisplayCacheManager
    {   
private:

    int m_nbCreatedMesh; 
    int m_nbDestroyedMesh;
    int m_nbCreatedTexture;
    int m_nbDestroyedTexture;
    
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
    return SUCCESS;
}

BentleyStatus ScalableMeshDisplayCacheManager::_DeleteFromVideoMemory(SmCachedDisplayTexture* cachedDisplayTex)
{
    return SUCCESS;
}

bool ScalableMeshDisplayCacheManager::_IsUsingVideoMemory()
{
    return false;
}

ScalableMeshDisplayCacheManager::ScalableMeshDisplayCacheManager(ViewContextR viewContext)
{
#if 0 
    m_qvCache = T_HOST.GetGraphicsAdmin()._CreateQvCache();
#endif

    m_nbCreatedMesh = 0;
    m_nbDestroyedMesh = 0;
    m_nbCreatedTexture = 0;
    m_nbDestroyedTexture = 0;
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
        /*
        if (!((ScalableMeshDisplayCacheManager*)m_displayNodesCache.get())->CanDisplay())
            {
            return nullptr;
            }
            */
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
* @bsimethod                                    Mathieu.St-Pierre                 11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayQueryTester::DoQuery()
    {

#if 0 
    DMatrix4d localToView(context.GetLocalToView());

    DMatrix4d smToUOR = DMatrix4d::From(m_smToModelUorTransform);

    bsiDMatrix4d_multiply(&localToView, &localToView, &smToUOR);

    //DPoint3d viewBox[8];

    //NEEDS_WORK_SM : Remove from query
    //GetViewBoxFromContext(viewBox, _countof(viewBox), context, drawingInfo);        
    DMatrix4d rootToStorage;

    //Convert the view box in storage.
    bool inverted = bsiDMatrix4d_invertQR(&rootToStorage, &m_storageToUorsTransfo);

    BeAssert(inverted != 0);
#endif
    status = SUCCESS;

   

    IScalableMeshViewDependentMeshQueryParamsPtr viewDependentQueryParams(IScalableMeshViewDependentMeshQueryParams::CreateParams());
    
    viewDependentQueryParams->SetMinScreenPixelsPerPoint(m_minScreenPixelsPerPoint);
    viewDependentQueryParams->SetMaxPixelError(m_maxPixelError);
    

#if 0 
    ClipVectorCP clip;
    clip = context.GetTransformClipStack().GetClip();
    //NEEDS_WORK_SM : Need to keep only SetViewBox or SetViewClipVector for visibility
    //viewDependentQueryParams->SetViewBox(viewBox);
    
    viewDependentQueryParams->SetRootToViewMatrix(m_rootToViewMatrix);

    //NEEDS_WORK_SM : Needed?
    /*
    if (s_progressiveDraw)
    {
    viewDependentQueryParams->SetProgressiveDisplay(true);
    viewDependentQueryParams->SetStopQueryCallback(CheckStopQueryCallback);
    }
    */

    ClipVectorPtr clipVectorCopy(ClipVector::CreateCopy(*clip));
    clipVectorCopy->TransformInPlace(m_modelUorToSmTransform);
#endif


    viewDependentQueryParams->SetViewClipVector(m_clipVector);
    
#if 0 
    m_currentDrawingInfoPtr->m_overviewNodes.clear();

    queryId = (int)((GetModelId().GetValue() - GetModelId().GetBriefcaseId().GetValue()) & 0xFFFF);//nextDrawingInfoPtr->GetViewNumber();                 
    m_currentDrawingInfoPtr->m_currentQuery = queryId;
#endif 

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

    ASSERT_TRUE(status == SUCCESS);

    if (m_waitQueryComplete)
        {
        while (!GetProgressiveQueryEngine()->IsQueryComplete(queryId))
            {
            BeThreadUtilities::BeSleep(200);
            }
        }

    ASSERT_TRUE(GetProgressiveQueryEngine()->IsQueryComplete(queryId));


    bvector<IScalableMeshCachedDisplayNodePtr> meshNodes;
    
    status = GetProgressiveQueryEngine()->GetRequiredNodes(meshNodes, queryId);
    ASSERT_TRUE(status == SUCCESS);    
    }

