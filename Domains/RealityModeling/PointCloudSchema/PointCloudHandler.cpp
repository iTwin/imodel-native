/*-------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudHandler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <PointCloudSchemaInternal.h>
#include <PointCloudSchema/PointCloudHandler.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_POINTCLOUDSCHEMA
USING_NAMESPACE_BENTLEY_BEPOINTCLOUD

HANDLER_DEFINE_MEMBERS(PointCloudModelHandler)

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
DgnModelId PointCloudModelHandler::CreatePointCloudModel(DgnDbR db, BeFileName fileName)
    {
    DgnClassId classId(db.Schemas().GetECClassId(BENTLEY_POINTCLOUD_SCHEMA_NAME, "PointCloudModel"));
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

    PointCloudModelPtr model = new PointCloudModel(DgnModel::CreateParams(db, classId, modelName));
    if (model->SetProperties(fileName) != SUCCESS)
        // Can't create model; probably that file name is invalid. Return an invalid model id.
        return DgnModelId();

    model->Insert();
    return model->GetModelId();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
PointCloudModel::PointCloudModel(CreateParams const& params) : T_Super (params)
    {
    m_pointCloudScenePtr = nullptr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
PointCloudModel::~PointCloudModel()
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
PointCloudScenePtr PointCloudModel::GetPointCloudScenePtr()
    {
    if (m_pointCloudScenePtr == nullptr)
        {
        // Open point cloud file
        Utf8CP urlUtf8 = m_properties.m_URL.c_str();
        WString fileName(WString(urlUtf8,BentleyCharEncoding::Utf8).c_str());
        m_pointCloudScenePtr = PointCloudScene::Create(fileName.c_str());
        }

    return m_pointCloudScenePtr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void PointCloudModel::_AddGraphicsToScene (ViewContextR context)
    {
    if (GetPointCloudScenePtr() != nullptr)
        {
        RefCountedPtr<PointCloudProgressiveDisplay> display = new PointCloudProgressiveDisplay(*this, *context.GetViewport());
        display->DrawView(context);
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     5/2015
//----------------------------------------------------------------------------------------
DRange3d PointCloudModel::GetSceneRange()
    {
    DRange3d range = DRange3d::From (0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    if (GetPointCloudScenePtr() != nullptr)
        {
        GetPointCloudScenePtr()->GetRange (range, false);
        }
    return DRange3d(range);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
AxisAlignedBox3d PointCloudModel::_QueryModelRange() const
    {
    return AxisAlignedBox3d(m_properties.m_range);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
BentleyStatus PointCloudModel::SetProperties (BeFileName fileName)
    {
    WString fileNameWithPath;
    BeFileName::BuildName (fileNameWithPath, fileName.GetDevice().c_str(), fileName.GetDirectoryWithoutDevice().c_str(), fileName.GetFileNameWithoutExtension().c_str(), fileName.GetExtension().c_str());
    BeStringUtilities::WCharToUtf8 (m_properties.m_URL, fileNameWithPath.c_str());

    if (GetPointCloudScenePtr() == nullptr)
        return ERROR;

    m_properties.m_range = GetSceneRange();
    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void PointCloudModel::JsonUtils::DPoint3dToJson (JsonValueR outValue, DPoint3dCR point)
    {
    outValue[0] = point.x;
    outValue[1] = point.y;
    outValue[2] = point.z;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void PointCloudModel::JsonUtils::DPoint3dFromJson (DPoint3dR point, Json::Value const& inValue)
    {
    point.x = inValue[0].asDouble();
    point.y = inValue[1].asDouble();
    point.z = inValue[2].asDouble();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void PointCloudModel::Properties::ToJson(Json::Value& v) const
    {
    //&&ep - inherit this from PointCloudBaseModel if we keep it
    JsonUtils::DPoint3dToJson(v["RangeLow"], m_range.low);
    JsonUtils::DPoint3dToJson(v["RangeHigh"], m_range.high);

    v["PointCloudURL"] = m_URL;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void PointCloudModel::Properties::FromJson(Json::Value const& v)
    {
    //&&ep - inherit this from PointCloudBaseModel if we keep it
    JsonUtils::DPoint3dFromJson(m_range.low, v["RangeLow"]);
    JsonUtils::DPoint3dFromJson(m_range.high, v["RangeHigh"]);

    m_URL = v["PointCloudURL"].asString();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void PointCloudModel::_ToPropertiesJson(Json::Value& v) const
    {
    T_Super::_ToPropertiesJson(v);
    m_properties.ToJson(v);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void PointCloudModel::_FromPropertiesJson(Json::Value const& v)
    {
    T_Super::_FromPropertiesJson(v);
    m_properties.FromJson(v);
    }

