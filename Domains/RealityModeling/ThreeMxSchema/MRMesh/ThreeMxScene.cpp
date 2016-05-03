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
    m_isUrl = (0 == strncmp("http:", rootUrl, 5) || 0 == strncmp("https:", rootUrl, 6));

    m_scale = location.ColumnXMagnitude();

    m_localCacheName = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectoryBaseName();
    m_localCacheName.AppendToPath(BeFileName(realityCacheName));
    m_localCacheName.AppendExtension(L"3mxCache");

    m_cache = RealityDataCache::Create(100);
    m_cache->RegisterStorage(*BeSQLiteRealityDataStorage::Create(m_localCacheName));
    m_cache->RegisterSource(IsUrl() ? (IRealityDataSourceBase&) *HttpRealityDataSource::Create(1) : *FileRealityDataSource::Create(1));
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

    m_rootNode = new Node(NodeInfo(), nullptr);
    m_rootNode->m_info.m_childPath = sceneInfo.m_rootNodePath;

    if (SUCCESS != SynchronousRead(*m_rootNode, ConstructNodeName(sceneInfo.m_rootNodePath, m_rootUrl)))
        return ERROR;

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    static uint32_t s_sleepMillis = 3;
    while (!m_rootNode->LoadedUntilDisplayable(*this))
        {
        ProcessRequests();
        BeThreadUtilities::BeSleep(s_sleepMillis);
        }
#endif

    m_rootNode->Dump("");
    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
BentleyStatus Scene::ReadGeoLocation(SceneInfo const& sceneInfo)
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
        m_location = Transform::FromProduct(localTransform, transform);
        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawBoundingSpheres(NodeCR node, RenderContextR context, SceneCR scene) 
    {
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
    drawBoundingSpheres(*m_rootNode, context, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Scene::GetMeshMemorySize() const
    {
    return m_rootNode->GetMeshMemorySize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Scene::GetTextureMemorySize() const
    {
    return m_rootNode->GetTextureMemorySize();
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Scene::GetNodeCount() const
    {
    return m_rootNode->GetNodeCount();
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Scene::FlushStale(uint64_t staleTime)
    {
    if (!m_requests.empty())
        return;

    m_rootNode->FlushStale(staleTime);
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Scene::GetMaxDepth() const
    {
    return 1 + m_rootNode->GetMaxDepth();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
DRange3d Scene::GetRange(TransformCR trans) const
    {
    return m_rootNode->GetRange(trans);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool Scene::Draw(RenderContextR context)
    {
    return m_rootNode->Draw(context, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Scene::ConstructNodeName(Utf8StringCR childName, Utf8StringCR parentPath)
    {
    if (IsUrl())
        {
        Utf8String parentDir = parentPath.substr(0, parentPath.find_last_of("/"));
        return parentDir + "/" + childName;
        }

    BeFileName nodeFileName(BeFileName::DevAndDir, BeFileName(parentPath));
    nodeFileName.AppendToPath(BeFileName(childName));
    return nodeFileName.GetNameUtf8();
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
#endif

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double Scene::CalculateResolutionRatio()
    {
#if defined (BENTLEYCONFIG_OS_WINDOWS) && !defined (BENTLEYCONFIG_OS_WINRT)
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof (statex);
    ::GlobalMemoryStatusEx(&statex);

    static uint32_t s_memoryThresholdPercent = 90;       // Start limiting usage at 70% memory usage.

    if (statex.dwMemoryLoad < s_memoryThresholdPercent)
        return 1.0;

    if (statex.dwMemoryLoad > 99)
        return 100.0;

    return (100.0 - (double) s_memoryThresholdPercent) / (100.0 - (double) statex.dwMemoryLoad);
#else
    return 1.0;
#endif
    }
