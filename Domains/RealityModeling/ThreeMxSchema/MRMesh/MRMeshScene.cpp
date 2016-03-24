/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/MRMesh/MRMeshScene.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "..\ThreeMxSchemaInternal.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
MRMeshScene::MRMeshScene(DgnDbR db, S3SceneInfo const& sceneInfo, BeFileNameCR fileName) : m_db(db), m_fileName(fileName)
    {
    m_transform.InitIdentity();
    m_sceneName = sceneInfo.m_sceneName;
    m_srs       = sceneInfo.m_SRS;

   if (3 == sceneInfo.m_SRSOrigin.size())
        m_srsOrigin.InitFromArray(&sceneInfo.m_SRSOrigin.front());
    else
        m_srsOrigin.Zero();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MRMeshScene::Load(S3SceneInfo const& sceneInfo, SystemP system)
    {
    BeFileName scenePath(BeFileName::DevAndDir, m_fileName.c_str());
    for (auto const& child : sceneInfo.m_meshChildren)
        {
        MRMeshNodePtr childNode = new MRMeshNode(S3NodeInfo(), nullptr, system);

        if (SUCCESS != MRMeshCacheManager::GetManager().SynchronousRead(*childNode, MRMeshUtil::ConstructNodeName(child, &scenePath), system))
            return ERROR;

        childNode->LoadUntilDisplayable();
        m_children.push_back(childNode);
        }

    GetProjectionTransform();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawBoundingSpheres(MRMeshNodeCR node, RenderContextR context) 
    {
    if (!node.IsLoaded())
        return;

    if (node.IsDisplayable())
        node.DrawBoundingSphere(context);

    for (auto const& child : node.GetChildren())
        drawBoundingSpheres(*child, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshScene::DrawBoundingSpheres(RenderContextR context)
    {
    for (auto const& child : m_children)
        drawBoundingSpheres(*child, context);
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshScene::DrawMeshes(Render::GraphicR graphic)
    {
    for (auto const& child : m_children)
        child->DrawMeshes(graphic, m_transform);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t MRMeshScene::GetMeshMemorySize() const
    {
    size_t memorySize = 0;

    for (auto const& child : m_children)
        memorySize += child->GetMeshMemorySize();

    return memorySize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t MRMeshScene::GetTextureMemorySize() const
    {
    size_t memorySize = 0;

    for (auto const& child : m_children)
        memorySize += child->GetTextureMemorySize();

    return memorySize;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t MRMeshScene::GetNodeCount() const
    {
    size_t  count = 1;

    for (auto const& child : m_children)
        count += child->GetNodeCount();

    return count;
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t MRMeshScene::GetMaxDepth() const
    {
    size_t maxChildDepth = 0;

    for (auto const& child : m_children)
        {
        size_t childDepth = child->GetMaxDepth();
        if (childDepth > maxChildDepth)
            maxChildDepth = childDepth;
        }

    return 1 + maxChildDepth;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
BentleyStatus MRMeshScene::_GetRange(DRange3dR range) const
    {
    range.Init();
    for (auto const& child : m_children)
        {
        DRange3d childRange = DRange3d::NullRange();

        if (SUCCESS == child->GetRange(range, m_transform))
            range.UnionOf(range, childRange);
        }

    return range.IsNull() ? ERROR : SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
BentleyStatus MRMeshScene::GetProjectionTransform()
    {
    DRange3d  range;
    if (SUCCESS != _GetRange(range))
        return ERROR;

    DgnGCSPtr databaseGCS = m_db.Units().GetDgnGCS();
    if (!databaseGCS.IsValid())
        return ERROR;

    if (m_srs.empty())
        return SUCCESS;         // No GCS...

    Transform transform = Transform::From(m_srsOrigin);

    int epsgCode;
    WString    warningMsg;
    StatusInt  status, warning;

    DgnGCSPtr acute3dGCS = DgnGCS::CreateGCS(m_db);

    if (1 == sscanf(m_srs.c_str(), "EPSG:%d", &epsgCode))
        status = acute3dGCS->InitFromEPSGCode(&warning, &warningMsg, epsgCode);
    else
        status = acute3dGCS->InitFromWellKnownText(&warning, &warningMsg, DgnGCS::wktFlavorEPSG, WString(m_srs.c_str(), false).c_str());

    if (SUCCESS != status)
        {
        BeAssert(false && warningMsg.c_str());
        return ERROR;
        }

    DRange3d sourceRange;
    transform.Multiply(sourceRange, range);
    
    DPoint3d extent;
    extent.DifferenceOf(sourceRange.high, sourceRange.low);

    // Compute a linear transform that approximate the reprojection transformation.
    Transform localTransform;
    status = acute3dGCS->GetLocalTransform(&localTransform, sourceRange.low, &extent, true/*doRotate*/, true/*doScale*/, *databaseGCS);

    // 0 == SUCCESS, 1 == Wajrning, 2 == Severe Warning,  Negative values are severe errors.
    if (status == 0 || status == 1)
        {
        m_transform = Transform::FromProduct(localTransform, transform);
        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshScene::Draw(bool& childrenScheduled, RenderContextR context, MRMeshContextCR meshContext)
    {
    for (auto const& child : m_children)
        {
        child->Draw(childrenScheduled, context, meshContext);
        if (context.CheckStop())
            return;
        }
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                Nicholas.Woodfield     01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshScene::GetTiles(TileCallback& callback, double resolution) const
    {
    for (auto const& child : m_children)
        child->GetTiles(callback, resolution);
    }
