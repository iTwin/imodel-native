/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/MRMesh/MRMeshScene.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ThreeMxSchemaInternal.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Scene::Scene(DgnDbR db, SceneInfo const& sceneInfo, BeFileNameCR fileName) : m_db(db), m_fileName(fileName)
    {
    m_placement.FromIdentity();
    m_sceneName = sceneInfo.m_sceneName;
    m_srs       = sceneInfo.m_SRS;

   if (3 == sceneInfo.m_SRSOrigin.size())
        m_srsOrigin.InitFromArray(&sceneInfo.m_SRSOrigin.front());
    else
        m_srsOrigin.Zero();

    m_cache = RealityDataCache::Create(100);

    BeFileName storageFileName = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectoryBaseName();
    storageFileName.AppendToPath(BeFileName(GetSceneName()));

    m_cache->RegisterStorage(*BeSQLiteRealityDataStorage::Create(storageFileName));
    m_cache->RegisterSource(*FileRealityDataSource::Create(4));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Scene::Load(SceneInfo const& sceneInfo)
    {
    BeFileName scenePath(BeFileName::DevAndDir, m_fileName.c_str());
    for (auto const& child : sceneInfo.m_meshChildren)
        {
        NodePtr childNode = new Node(NodeInfo(), nullptr);

        if (SUCCESS != SynchronousRead(*childNode, Util::ConstructNodeName(child, &scenePath)))
            return ERROR;

        childNode->LoadUntilDisplayable(*this);
        m_children.push_back(childNode);
        }

    GetProjectionTransform();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawBoundingSpheres(NodeCR node, RenderContextR context) 
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
void Scene::DrawBoundingSpheres(RenderContextR context)
    {
    for (auto const& child : m_children)
        drawBoundingSpheres(*child, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Scene::GetMeshMemorySize() const
    {
    size_t memorySize = 0;

    for (auto const& child : m_children)
        memorySize += child->GetMeshMemorySize();

    return memorySize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Scene::GetTextureMemorySize() const
    {
    size_t memorySize = 0;

    for (auto const& child : m_children)
        memorySize += child->GetTextureMemorySize();

    return memorySize;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Scene::GetNodeCount() const
    {
    size_t  count = 1;
    for (auto const& child : m_children)
        count += child->GetNodeCount();
    return count;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Scene::GetMeshCount() const
    {
    size_t  count = 0;
    for (auto const& child : m_children)
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

    for (auto& child : m_children)
        child->FlushStale(staleTime);
    }

/*-----------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Scene::GetMaxDepth() const
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
DRange3d Scene::GetRange() const
    {
    DRange3d range;
    range.Init();

    for (auto const& child : m_children)
        range.UnionOf(range, child->GetRange());

    return range;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
DRange3d Scene::GetSphereRange() const
    {
    DRange3d range;
    range.Init();

    for (auto const& child : m_children)
        range.UnionOf(range, child->GetSphereRange());

    return range;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
BentleyStatus Scene::GetProjectionTransform()
    {
    DRange3d  range = GetSphereRange();
    if (range.IsNull())
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
        // Support for 3MX specific ENU (East North, Up) GCS specification
    else if (2 == sscanf (sceneInfo.SRS.c_str(), "ENU:%lf,%lf", &latitude, &longitude))
        {
        // ENU specification does not impose any projection method so we use the first azimuthal available using values that will
        // mimick the intent (North is Y positive, no offset)
        // Note that we could have injected the origin here but keeping it in the transform as for other GCS specs
        if (latitude < 90.0 && latitude > -90.0 && longitude < 180.0 && longitude > -180.0)
            status = acute3dGCS->InitAzimuthalEqualArea(&warningMsg, L"WGS84", L"METER", longitude, latitude, 0.0, 1.0, 0.0, 0.0, 1);
        else
            status = ERROR;
        }
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

    // 0 == SUCCESS, 1 == Warning, 2 == Severe Warning,  Negative values are severe errors.
    if (status == 0 || status == 1)
        {
        m_placement = Transform::FromProduct(localTransform, transform);
        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool Scene::Draw(RenderContextR context)
    {
    bool childrenScheduled=false;
    for (auto const& child : m_children)
        {
        childrenScheduled |= child->Draw(context, *this);
        if (context.CheckStop())
            break;
        }
    return childrenScheduled;
    }

