/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ThreeMxInternal.h"
#include <GeomJsonWireFormat/JsonUtils.h>

DOMAIN_DEFINE_MEMBERS(ThreeMxDomain)
HANDLER_DEFINE_MEMBERS(ModelHandler)

USING_NAMESPACE_DGN_CESIUM

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Scene::ReadSceneFile()
    {
    StreamBuffer rootStream;

    if (IsHttp())
        {
        HttpDataQuery query(m_sceneFile, nullptr);
        query.Perform().wait();

        rootStream = StreamBuffer(std::move(query.GetData()));
        }
    else
        {
        FileDataQuery query(m_sceneFile, nullptr);
        rootStream = StreamBuffer(std::move(query.Perform().get()));
        }

    return rootStream.HasData() ? m_sceneInfo.Read(rootStream) : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Scene::LoadScene(OutputR output)
    {
    if (SUCCESS != ReadSceneFile())
        return ERROR;

    Utf8String  cacheFileString = Utf8String (m_sceneFile.c_str());

    // Use full file path (without special characters) for cache key (was just using scene name).
    cacheFileString.ReplaceAll("\\", "_");
    cacheFileString.ReplaceAll("/", "_");
    cacheFileString.ReplaceAll(":", "_");
    cacheFileString.ReplaceAll(".", "_");

    Node* root = new Node(*this, nullptr);
    root->m_childPath = m_sceneInfo.m_rootNodePath;
    m_rootTile = root;

    auto result = _RequestTile(*root, *LoadState::Create(*root), output);
    result.wait(BeDuration::Seconds(2)); // only wait for 2 seconds
    return result.isReady() ? SUCCESS : ERROR;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
BentleyStatus Scene::LocateFromSRS()
    {
    DgnGCSPtr bimGCS = m_db.GeoLocation().GetDgnGCS();
    if (!bimGCS.IsValid())
        return ERROR; // BIM is not geolocated, can't use geolocation in 3mx scene

    if (m_sceneInfo.m_reprojectionSystem.empty())
        return SUCCESS;  // scene has no spatial reference system, give up.

    WString    warningMsg;
    StatusInt  status, warning;

    DgnGCSPtr threeMxGCS = DgnGCS::CreateGCS(m_db);

    int epsgCode;
    double latitude, longitude;
    if (1 == Utf8String::Sscanf_safe(m_sceneInfo.m_reprojectionSystem.c_str(), "EPSG:%d", &epsgCode))
        status = threeMxGCS->InitFromEPSGCode(&warning, &warningMsg, epsgCode);
    else if (2 == Utf8String::Sscanf_safe(m_sceneInfo.m_reprojectionSystem.c_str(), "ENU:%lf,%lf", &latitude, &longitude))
        {
        // ENU specification does not impose any projection method so we use the first azimuthal available using values that will
        // mimic the intent (North is Y positive, no offset)
        // Note that we could have injected the origin here but keeping it in the transform as for other GCS specs
        if (latitude < 90.0 && latitude > -90.0 && longitude < 180.0 && longitude > -180.0)
            status = threeMxGCS->InitAzimuthalEqualArea(&warningMsg, L"WGS84", L"METER", longitude, latitude, 0.0, 1.0, 0.0, 0.0, 1);
        else
            status = ERROR;
        }
    else
        status = threeMxGCS->InitFromWellKnownText(&warning, &warningMsg, DgnGCS::wktFlavorEPSG, WString(m_sceneInfo.m_reprojectionSystem.c_str(), false).c_str());

    if (SUCCESS != status)
        {
        BeAssert(false && warningMsg.c_str());
        return ERROR;
        }

    // Compute a linear transform that approximates the reprojection transformation at the origin.
    Transform localTransform;
    status = threeMxGCS->GetLocalTransform(&localTransform, m_sceneInfo.m_origin, nullptr, true/*doR.otate*/, true/*doScale*/, *bimGCS);

    // 0 == SUCCESS, 1 == Warning, 2 == Severe Warning,  Negative values are severe errors.
    if (status == 0 || status == 1)
        {
        m_location = localTransform;
        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
ThreeMxDomain::ThreeMxDomain() : DgnDomain(THREEMX_SCHEMA_NAME, "3MX Domain", 1)
    {
    RegisterHandler(ModelHandler::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId ModelHandler::CreateModel(RepositoryLinkCR modeledElement, Utf8CP sceneFile, TransformCP trans, ClipVectorCP clip, ModelSpatialClassifiersCP classifiers)
    {
    DgnDbR db = modeledElement.GetDgnDb();
    DgnClassId classId(db.Schemas().GetClassId(THREEMX_SCHEMA_NAME, "ThreeMxModel"));
    BeAssert(classId.IsValid());

    ThreeMxModelPtr model = new ThreeMxModel(DgnModel::CreateParams(db, classId, modeledElement.GetElementId()));

    model->SetSceneFile(sceneFile);
    if (trans)
        model->SetLocation(*trans);

    if (clip)
        model->SetClip(ClipVector::CreateCopy(*clip).get());

    if (nullptr != classifiers)
        model->SetClassifiers(*classifiers);

    model->Insert();
    return model->GetModelId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreeMxModel::SetClip(Dgn::ClipVectorCP clip)
    {
    m_clip = clip;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Cesium::RootPtr ThreeMxModel::_CreateCesiumTileTree(Cesium::OutputR output, Utf8StringCR)
    {
    ScenePtr scene = new Scene(*this, m_location, m_sceneFile.c_str());
    if (SUCCESS != scene->LoadScene(output))
        return nullptr;

    scene->SetClip(m_clip.get());
    return scene.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreeMxModel::_OnSaveJsonProperties()
    {
    T_Super::_OnSaveJsonProperties();

    Json::Value val;
    val[json_sceneFile()] = m_sceneFile;
    if (!m_location.IsIdentity())
        JsonUtils::TransformToJson(val[json_location()], m_location);

    if (m_clip.IsValid())
        val[json_clip()] = m_clip->ToJson();

    if (!m_classifiers.empty())
        SetJsonProperties(json_classifiers(), m_classifiers.ToJson());

    SetJsonProperties(json_threemx(), val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreeMxModel::_OnLoadedJsonProperties()
    {
    T_Super::_OnLoadedJsonProperties();

    JsonValueCR val = GetJsonProperties(json_threemx());
    m_sceneFile = val[json_sceneFile()].asString();

    if (val.isMember(json_location()))
        JsonUtils::TransformFromJson(m_location, val[json_location()]);
    else
        m_location.InitIdentity();

    if (val.isMember(json_clip()))
        m_clip = ClipVector::FromJson(val[json_clip()]);

    Json::Value     classifiers = GetJsonProperties(json_classifiers());
    if (classifiers.isNull())
        classifiers = m_classifiers.FromJson(val[json_classifiers()]);       // Old location.

    if (!classifiers.isNull())
        m_classifiers.FromJson(classifiers);
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/18
//=======================================================================================
struct NullOutput : Dgn::Cesium::Output
{
    virtual void _AddBatch(DRange3dCR range, Render::FeatureTable&& features) { }
    virtual void _AddTriMesh(Render::TriMeshArgsCR) { }
    virtual void _AddPointCloud(Render::PointCloudArgsCR) {}
};
//----------------------------------------------------------------------------------------
// @bsimethod                                                       Ray.Bentley     10/2019
//----------------------------------------------------------------------------------------
AxisAlignedBox3d ThreeMxModel::_QueryNonElementModelRange() const
    {
    AxisAlignedBox3d    range;
    auto                nullOutput = new NullOutput();

    ScenePtr scene = new Scene(const_cast<ThreeMxModel&> (*this), m_location, m_sceneFile.c_str());
    if (SUCCESS == scene->LoadScene(*nullOutput))
        scene->GetLocation().Multiply(range, scene->GetRange());

    return  range;
    }