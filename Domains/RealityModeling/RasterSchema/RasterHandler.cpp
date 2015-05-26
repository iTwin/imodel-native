/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterHandler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RasterSchemaInternal.h>
#include <RasterSchema/RasterHandler.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_RASTERSCHEMA

HANDLER_DEFINE_MEMBERS(RasterModelHandler)

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
DgnModelId RasterModelHandler::CreateRasterModel(DgnDbR db, BeFileName fileName)
    {
    DgnClassId classId(db.Schemas().GetECClassId(BENTLEY_RASTER_SCHEMA_NAME, "RasterModel"));
    BeAssert(classId.IsValid());

    WString dev;
    WString dir;
    WString name;
    WString ext;
    fileName.ParseName(&dev, &dir, &name, &ext);

    // Just keep model name (no path or extension) and convert it to Utf8CP
    Utf8String utf8Name;
    BeStringUtilities::WCharToUtf8 (utf8Name, name.c_str());
    Utf8CP modelName = utf8Name.c_str();

    RasterModelPtr model = new RasterModel(DgnModel::CreateParams(db, classId, modelName));
    if (model->SetProperties(fileName) != SUCCESS)
        // Can't create model; probably that file name is invalid. Return an invalid model id.
        return DgnModelId();

    db.Models().Insert(*model);
    return model->GetModelId();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterModel::RasterModel(CreateParams const& params) : T_Super (params)
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterModel::~RasterModel()
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void RasterModel::_AddGraphicsToScene (ViewContextR context)
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
AxisAlignedBox3d RasterModel::_QueryModelRange() const
    {
    return m_properties.m_range;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
BentleyStatus RasterModel::SetProperties (BeFileName fileName)
    {
    WString fileNameWithPath;
    BeFileName::BuildName (fileNameWithPath, fileName.GetDevice().c_str(), fileName.GetDirectoryWithoutDevice().c_str(), fileName.GetFileNameWithoutExtension().c_str(), fileName.GetExtension().c_str());
    BeStringUtilities::WCharToUtf8 (m_properties.m_URL, fileNameWithPath.c_str());

//    m_properties.m_range = GetSceneRange();

    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void RasterModel::_ToPropertiesJson(Json::Value& v) const
    {
    T_Super::_ToPropertiesJson(v);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void RasterModel::_FromPropertiesJson(Json::Value const& v)
    {
    T_Super::_FromPropertiesJson(v);
    }

