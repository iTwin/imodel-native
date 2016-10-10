/*-------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/ThreeMxHandler.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ThreeMxSchemaInternal.h"

#include <WebServices/Client/WSClient.h>
#include <WebServices/Connect/SamlToken.h>
#include <WebServices/Connect/Authentication.h>
#include <WebServices/Connect/Connect.h>
#include <WebServices/Connect/ConnectSetup.h>
#include <WebServices/Connect/ConnectSpaces.h>

#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Licensing/UsageTracking.h>


USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_THREEMX_SCHEMA
USING_NAMESPACE_BENTLEY_WEBSERVICES

HANDLER_DEFINE_MEMBERS(ThreeMxModelHandler)

//========================================================================================
// @bsiclass                                                        Ray.Bentley     09/2015
//========================================================================================
struct ThreeMxSchema::ThreeMxProgressiveDisplay : Dgn::IProgressiveDisplay, NonCopyableClass
{
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS

protected:
    ThreeMxModelR        m_model;

    ThreeMxProgressiveDisplay (ThreeMxModelR model) : m_model (model) {DEFINE_BENTLEY_REF_COUNTED_MEMBER_INIT }

public:
    virtual bool _WantTimeoutSet(uint32_t& limit)   {return false; }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
virtual Completion _Process(ViewContextR viewContext) override
    {
    switch (MRMeshCacheManager::GetManager().ProcessRequests())
        {
        default:
        case MRMeshCacheManager::RequestStatus::None:
        case MRMeshCacheManager::RequestStatus::Processed:
            return Completion::HealRequired;

        case MRMeshCacheManager::RequestStatus::Finished:
            return Completion::Finished;
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
static void Schedule (ThreeMxModelR model, ViewContextR viewContext)
    {
    ThreeMxProgressiveDisplay*   progressiveDisplay = new ThreeMxProgressiveDisplay (model);

    viewContext.GetViewport()->ScheduleProgressiveDisplay (*progressiveDisplay);
    }

};  //  ThreeMxProgressiveDisplay

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
ThreeMxScenePtr      ThreeMxModel::ReadScene (BeFileNameR fileName, DgnDbR db, Utf8StringCR fileId)
    {
    BentleyStatus status = T_HOST.GetPointCloudAdmin()._ResolveFileName(fileName, fileId, db);
    if (status != SUCCESS)
        return nullptr;

    S3SceneInfo         sceneInfo;
    std::string         err;
    ThreeMxScenePtr     scene;
    Transform           transform;
    DRange3d            range;
    Utf8String          authToken;


    Utf8String tempFileName;
    BeStringUtilities::WCharToUtf8(tempFileName, fileName);

    MRMeshFileName meshFileName(tempFileName);
    
    
    // Stream-X connection always requires the CONNECT token of the one logged-in
    if (meshFileName.IsS3MXUrl())
        {
        Utf8String          cacheToken;
        SamlTokenPtr cacheTokenPtr = ConnectAuthenticationPersistence::GetShared()->GetToken();
        if (cacheTokenPtr != nullptr && !cacheTokenPtr->IsEmpty())
            cacheToken = cacheTokenPtr->AsString();

        Utf8String stsToken;
        stsToken = "Token ";

        authToken = stsToken.append(cacheToken).c_str();
        }



	if (SUCCESS != MRMeshUtil::ReadSceneFile(sceneInfo, fileName.c_str(), &authToken) ||
       ! (scene = MRMeshScene::Create (sceneInfo, fileName)).IsValid())
        return nullptr;

    if (SUCCESS == scene->_GetRange (range, Transform::FromIdentity()) &&
        SUCCESS == ThreeMxGCS::GetProjectionTransform (transform, sceneInfo, db, range))
        scene->SetTransform (transform);

    return scene;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
DgnModelId ThreeMxModel::CreateThreeMxModel(DgnDbR db, Utf8StringCR fileId)
    {
    DgnClassId classId (db.Schemas().GetECClassId(BENTLEY_THREEMX_SCHEMA_NAME, "ThreeMxModel"));
    BeAssert(classId.IsValid());

    BeFileName      fileName;


    
    ThreeMxScenePtr scene = ReadScene (fileName, db, fileId);
    if (! scene.IsValid())
        return DgnModelId();
    
    // Create model in DgnDb
    // This is to temporarily support ThreeMX on Stream-X HTTP servers in ConceptStation
    // on the stream DgnDb06 ONLY. Code on newer streams will require other changes.
    // DO NOT PORT!	
    Utf8String tempFileName;
    BeStringUtilities::WCharToUtf8(tempFileName, fileName);

    MRMeshFileName meshFileName(tempFileName);

    Utf8String modelName(meshFileName.GetFileNameWithoutExtension().c_str());
    ThreeMxModelPtr model = new ThreeMxModel (DgnModel::CreateParams(db, classId, CreateModelCode(modelName)));

    model->SetScene (scene);
    model->SetFileId (fileId);
    model->Insert();
    return model->GetModelId();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
ThreeMxScenePtr  ThreeMxModel::GetScene ()
    {
    if (!m_scene.IsValid())
        {
        BeFileName      fileName;

        m_scene = ReadScene (fileName, GetDgnDb(), m_properties.m_fileId);
        }

    return m_scene;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                               Nicholas.Woodfield     01/2016
//----------------------------------------------------------------------------------------
void ThreeMxModel::GetTiles(GetTileCallback callback, double resolution)
  {
  if(callback == nullptr || resolution < 0)
    return;
    
    ThreeMxScenePtr  scene = GetScene();

    if (!scene.IsValid())
        {
        BeAssert (false);
        return;
        }
  
  scene->_GetTiles(callback, resolution);
  }

//----------------------------------------------------------------------------------------
// @bsimethod                                               Nicholas.Woodfield     01/2016
//----------------------------------------------------------------------------------------
void ThreeMxModel::GetTransform(TransformR transform) const
    {
    if (!m_scene.IsValid())
        {
        BeAssert(false);
        return;
        }

    m_scene->GetTransform(transform);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                               Nicholas.Woodfield     01/2016
//----------------------------------------------------------------------------------------
double ThreeMxModel::GetDefaultExportResolution() const
    {
    return 0.0;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
AxisAlignedBox3d ThreeMxModel::_QueryModelRange() const
    {
    if (!m_scene.IsValid())
        {
        BeAssert(false);
        return AxisAlignedBox3d();
        }

    DRange3d range;
    Transform transform;
    m_scene->GetTransform(transform);

    m_scene->_GetRange(range, transform);

    return AxisAlignedBox3d(range);
    }   

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
void ThreeMxModel::_AddGraphicsToScene (ViewContextR viewContext)
    {
    ThreeMxScenePtr  scene = GetScene();

    if (!scene.IsValid())
        {
        BeAssert (false);
        return;
        }

    MRMeshContext       meshContext (Transform::FromIdentity(), viewContext, 0.0);
    ViewFlags           viewFlags = *viewContext.GetViewFlags(), saveViewFlags = viewFlags;
    bool                childrenScheduled = false;

    viewFlags.ignoreLighting = true;
    
    viewContext.GetViewport()->GetIViewOutput()->SetRenderMode (viewFlags);
    scene->_Draw (childrenScheduled, viewContext, meshContext);
    viewContext.GetViewport()->GetIViewOutput()->SetRenderMode (saveViewFlags);

    if (childrenScheduled)
        ThreeMxProgressiveDisplay::Schedule (*this, viewContext);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
void ThreeMxModel::Properties::ToJson(Json::Value& v) const
    {
    v["FileId"] = m_fileId.c_str();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
void ThreeMxModel::Properties::FromJson(Json::Value const& v)
    {
    m_fileId = v["FileId"].asString();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
void ThreeMxModel::_WriteJsonProperties(Json::Value& v) const
    {
    T_Super::_WriteJsonProperties(v);
    m_properties.ToJson(v);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
void ThreeMxModel::_ReadJsonProperties(Json::Value const& v)
    {
    T_Super::_ReadJsonProperties(v);
    m_properties.FromJson(v);
    }

