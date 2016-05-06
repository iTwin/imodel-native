/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/ThreeMxScene.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ThreeMxInternal.h"

#if defined (BENTLEYCONFIG_OS_WINDOWS)
#include <windows.h>
#endif

#define RENDER_LOGGING 1
#if defined (RENDER_LOGGING)
#   define THREADLOG (*NativeLogging::LoggingManager::GetLogger(DgnDb::GetThreadIdName()))
#   define DEBUG_PRINTF THREADLOG.debugv
#   define WARN_PRINTF THREADLOG.debugv
#   define ERROR_PRINTF THREADLOG.errorv
#else
#   define DEBUG_PRINTF(...)
#   define WARN_PRINTF(...)
#   define ERROR_PRINTF(...)
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
Scene::Scene(DgnDbR db, TransformCR location, Utf8CP realityCacheName, Utf8CP rootUrl, Render::SystemP renderSys) : m_db(db), m_rootUrl(rootUrl), m_location(location), m_renderSystem(renderSys)
    {
    m_isUrl = (0 == strncmp("http:", rootUrl, 5) || 0 == strncmp("https:", rootUrl, 6));
    if (m_isUrl)
        m_rootDir = m_rootUrl.substr(0, m_rootUrl.find_last_of("/"));
    else
        {
        BeFileName rootUrl(BeFileName::DevAndDir, BeFileName(m_rootUrl));
        BeFileName::FixPathName(rootUrl, rootUrl, false);
        m_rootDir = rootUrl.GetNameUtf8();
        }

    m_scale = location.ColumnXMagnitude();

    m_localCacheName = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectoryBaseName();
    m_localCacheName.AppendToPath(BeFileName(realityCacheName));
    m_localCacheName.AppendExtension(L"3MXcache");

    m_cache = RealityDataCache::Create();
    m_cache->RegisterStorage(*BeSQLiteRealityDataStorage::Create(m_localCacheName));
    m_cache->RegisterSource(IsUrl() ? (IRealityDataSourceBase&) *HttpRealityDataSource::Create(8) : *FileRealityDataSource::Create(4));
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
    MxStreamBuffer* rootStream = nullptr;
    RealityDataCacheResult status = RequestData(nullptr, true, &rootStream);
    if (RealityDataCacheResult::Success != status)
        return ERROR;

    BeAssert(nullptr != rootStream);
    return sceneInfo.Read(*rootStream);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Scene::LoadScene()
    {
    SceneInfo sceneInfo;
    if (SUCCESS != ReadRoot(sceneInfo))
        return ERROR;

    m_rootNode = new Node(nullptr);
    m_rootNode->m_childPath = sceneInfo.m_rootNodePath;

    return RealityDataCacheResult::Success == RequestData(m_rootNode.get(), true, nullptr) ? SUCCESS : ERROR;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Scene::GetNodeCount() const
    {
    return m_rootNode->GetNodeCount();
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
    DrawArgs args(context, *this);
    m_rootNode->Draw(args);

    if (!args.m_graphics.m_entries.empty())
        {
        DEBUG_PRINTF("drawing %d 3mx nodes", args.m_graphics.m_entries.size());
        auto group = context.CreateGroupNode(Graphic::CreateParams(nullptr, GetLocation()), args.m_graphics, nullptr);
        BeAssert(args.m_graphics.m_entries.empty()); // the CreateGroupNode should have moved them
        context.OutputGraphic(*group, nullptr);
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Scene::ConstructNodeName(Node& node)
    {
    return m_rootDir + node.GetChildFile();
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

    Transform transform = Transform::From(sceneInfo.m_origin);
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

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Scene::GetMemoryStatistics(size_t& memoryLoad, size_t& total, size_t& available)
    {
#if defined (BENTLEYCONFIG_OS_WINDOWS) && !defined (BENTLEYCONFIG_OS_WINRT)
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof (statex);
    ::GlobalMemoryStatusEx(&statex);

    memoryLoad = (size_t) statex.dwMemoryLoad;
    total      = (size_t) statex.ullTotalPhys;
    available  = (size_t) statex.ullAvailPhys;
#endif
    }

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
