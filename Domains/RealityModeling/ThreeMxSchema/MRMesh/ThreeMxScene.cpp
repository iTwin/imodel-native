/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/MRMesh/ThreeMxScene.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ThreeMxSchemaInternal.h"

#if defined (BENTLEYCONFIG_OS_WINDOWS)
#include <windows.h>
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
Scene::Scene(DgnDbR db, TransformCR location, Utf8CP realityCacheName, Utf8CP rootUrl, Render::SystemP renderSys) : m_db(db), m_rootUrl(rootUrl), m_location(location), m_renderSystem(renderSys)
    {
    m_scale = location.ColumnXMagnitude();

    m_localCacheName = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectoryBaseName();
    m_localCacheName.AppendToPath(BeFileName(realityCacheName));
    m_localCacheName.AppendExtension(L"3mxCache");

    m_cache = RealityDataCache::Create(100);
    m_cache->RegisterStorage(*BeSQLiteRealityDataStorage::Create(m_localCacheName));
    m_cache->RegisterSource(m_rootUrl.IsUrl() ? (IRealityDataSourceBase&) *HttpRealityDataSource::Create(4) : *FileRealityDataSource::Create(1));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Scene::DeleteRealityCache()
    {
    m_cache = nullptr;
    return BeFileNameStatus::Success == m_localCacheName.BeDeleteFile() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Scene::ReadRoot(SceneInfo& sceneInfo)
    {
    MxStreamBuffer rootStream;
    RealityDataCacheResult status = RequestData(nullptr, m_rootUrl, true, &rootStream);
    return (RealityDataCacheResult::Success != status) ? ERROR : sceneInfo.Read(rootStream);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Scene::LoadScene()
    {
    SceneInfo sceneInfo;
    if (SUCCESS != ReadRoot(sceneInfo))
        return ERROR;

    BeFileName rootPath(BeFileName::DevAndDir, m_rootUrl.c_str());
    for (auto const& child : sceneInfo.m_meshChildren)
        {
        NodePtr childNode = new Node(NodeInfo(), nullptr);

        if (SUCCESS != SynchronousRead(*childNode, ConstructNodeName(child, &rootPath)))
            return ERROR;

        childNode->LoadUntilDisplayable(*this);
        m_rootNodes.push_back(childNode);
        }

    SetProjectionTransform(sceneInfo);
    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
BentleyStatus Scene::SetProjectionTransform(SceneInfo const& sceneInfo)
    {
    DRange3d  range = GetRange(Transform::FromIdentity());
    if (range.IsNull())
        return ERROR;

    DgnGCSPtr databaseGCS = m_db.Units().GetDgnGCS();
    if (!databaseGCS.IsValid())
        return ERROR;

    if (sceneInfo.m_reprojectionSystem.empty())
        return SUCCESS;         // No GCS...

    DPoint3d reprojectionOrigin;
    if (3 == sceneInfo.m_origin.size())
        reprojectionOrigin.InitFromArray(&sceneInfo.m_origin.front());
    else
        reprojectionOrigin.Zero();

    Transform transform = Transform::From(reprojectionOrigin);
    WString    warningMsg;
    StatusInt  status, warning;

    DgnGCSPtr acute3dGCS = DgnGCS::CreateGCS(m_db);

    int epsgCode;
    if (1 == sscanf(sceneInfo.m_reprojectionSystem.c_str(), "EPSG:%d", &epsgCode))
        status = acute3dGCS->InitFromEPSGCode(&warning, &warningMsg, epsgCode);
    else
        status = acute3dGCS->InitFromWellKnownText(&warning, &warningMsg, DgnGCS::wktFlavorEPSG, WString(sceneInfo.m_reprojectionSystem.c_str(), false).c_str());

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

    // 0 == SUCCESS, 1 == Warning, 2 == Severe Warning,  Negative values are severe errors.
    if (status == 0 || status == 1)
        {
//        m_location = Transform::FromProduct(localTransform, transform);
        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawBoundingSpheres(NodeCR node, RenderContextR context, SceneCR scene) 
    {
    if (!node.IsLoaded())
        return;

    if (node.IsDisplayable())
        node.DrawBoundingSphere(context, scene);

    for (auto const& child : node.GetChildren())
        drawBoundingSpheres(*child, context, scene);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Scene::DrawBoundingSpheres(RenderContextR context)
    {
    for (auto const& child : m_rootNodes)
        drawBoundingSpheres(*child, context, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Scene::GetMeshMemorySize() const
    {
    size_t memorySize = 0;

    for (auto const& child : m_rootNodes)
        memorySize += child->GetMeshMemorySize();

    return memorySize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Scene::GetTextureMemorySize() const
    {
    size_t memorySize = 0;

    for (auto const& child : m_rootNodes)
        memorySize += child->GetTextureMemorySize();

    return memorySize;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Scene::GetNodeCount() const
    {
    size_t  count = 1;
    for (auto const& child : m_rootNodes)
        count += child->GetNodeCount();
    return count;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Scene::GetMeshCount() const
    {
    size_t  count = 0;
    for (auto const& child : m_rootNodes)
        count += child->GetMeshCount();
    return count;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Scene::FlushStale(uint64_t staleTime)
    {
    if (!m_requests.empty())
        return;

    for (auto& child : m_rootNodes)
        child->FlushStale(staleTime);
    }

/*-----------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Scene::GetMaxDepth() const
    {
    size_t maxChildDepth = 0;

    for (auto const& child : m_rootNodes)
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
DRange3d Scene::GetRange(TransformCR trans) const
    {
    DRange3d range;
    range.Init();

    for (auto const& child : m_rootNodes)
        range.UnionOf(range, child->GetRange(trans));

    return range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool Scene::Draw(RenderContextR context)
    {
    bool childrenScheduled=false;
    for (auto const& child : m_rootNodes)
        {
        childrenScheduled |= child->Draw(context, *this);
        if (context.CheckStop())
            break;
        }
    return childrenScheduled;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName Scene::ConstructNodeName(Utf8StringCR childName, BeFileNameCP parentName)
    {
    BeFileName nodeFileName(childName.c_str());

    if (nodeFileName.IsAbsolutePath() || nodeFileName.IsUrl())
        return nodeFileName;

    BeFileName fullNodeFileName = (NULL == parentName) ? BeFileName() : *parentName;
    fullNodeFileName.AppendToPath(nodeFileName.c_str());

    return fullNodeFileName;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
#if defined (BENTLEYCONFIG_OS_WINDOWS) && !defined (BENTLEYCONFIG_OS_WINRT)
void Scene::GetMemoryStatistics(size_t& memoryLoad, size_t& total, size_t& available)
    {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof (statex);
    ::GlobalMemoryStatusEx(&statex);

    memoryLoad = (size_t) statex.dwMemoryLoad;
    total      = (size_t) statex.ullTotalPhys;
    available  = (size_t) statex.ullAvailPhys;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double Scene::CalculateResolutionRatio()
    {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof (statex);
    ::GlobalMemoryStatusEx(&statex);

    static uint32_t s_memoryThresholdPercent = 90;       // Start limiting usage at 70% memory usage.

    if (statex.dwMemoryLoad < s_memoryThresholdPercent)
        return 1.0;

    if (statex.dwMemoryLoad > 99)
        return 100.0;

    return (100.0 - (double) s_memoryThresholdPercent) / (100.0 - (double) statex.dwMemoryLoad);
    }
#endif

