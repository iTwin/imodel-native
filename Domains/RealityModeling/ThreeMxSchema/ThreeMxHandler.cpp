/*-------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/ThreeMxHandler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ThreeMxSchemaInternal.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_THREEMX_SCHEMA

HANDLER_DEFINE_MEMBERS(ThreeMxModelHandler)

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
DgnModelId ThreeMxModel::CreateThreeMxModel(DgnDbR db, BeFileNameCR fileName)
    {
    DgnClassId classId (db.Schemas().GetECClassId(BENTLEY_THREEMX_SCHEMA_NAME, "ThreeMxModel"));
    BeAssert(classId.IsValid());

    S3SceneInfo     sceneInfo;
    std::string     err;

    if (SUCCESS != BaseSceneNode::Read3MX (fileName, sceneInfo, err))
        return DgnModelId();

    Utf8String modelName(fileName.GetFileNameWithoutExtension().c_str());
    
    // Create model in DgnDb
    ThreeMxModelPtr model = new ThreeMxModel (DgnModel::CreateParams(db, classId, modelName.c_str()));
    model->Insert();
    return model->GetModelId();
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
AxisAlignedBox3d ThreeMxModel::_QueryModelRange() const
    {
    return AxisAlignedBox3d();          // Needs work...
    }   

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
void ThreeMxModel::_AddGraphicsToScene (ViewContextR context)
    {
#ifdef WIP
    if (GetThreeMxScenePtr() != nullptr)
        {
        RefCountedPtr<ThreeMxProgressiveDisplay> display = new ThreeMxProgressiveDisplay(*this);
        display->DrawView(context);
        }
#endif
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
void ThreeMxModel::_ToPropertiesJson(Json::Value& v) const
    {
    T_Super::_ToPropertiesJson(v);
    m_properties.ToJson(v);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
void ThreeMxModel::_FromPropertiesJson(Json::Value const& v)
    {
    T_Super::_FromPropertiesJson(v);
    m_properties.FromJson(v);
    }

