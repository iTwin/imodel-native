/*-------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/PointCloudBaseModel.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void PointCloudBaseModel::_AddGraphicsToScene (ViewContextR context)
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
AxisAlignedBox3d PointCloudBaseModel::_QueryModelRange() const
    {
    return m_properties.m_range;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void PointCloudBaseModel::Properties::ToJson(Json::Value& v) const
    {
    JsonUtils::DPoint3dToJson(v["RangeLow"], m_range.low);
    JsonUtils::DPoint3dToJson(v["RangeHigh"], m_range.high);
    v["PointCloudURL"] = m_URL;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void PointCloudBaseModel::Properties::FromJson(Json::Value const& v)
    {
    JsonUtils::DPoint3dFromJson(m_range.low, v["RangeLow"]);
    JsonUtils::DPoint3dFromJson(m_range.high, v["RangeHigh"]);
    m_URL = v["PointCloudURL"].asString();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void PointCloudBaseModel::_ToPropertiesJson(Json::Value& v) const
    {
    T_Super::_ToPropertiesJson(v);
    m_properties.ToJson(v);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void PointCloudBaseModel::_FromPropertiesJson(Json::Value const& v)
    {
    T_Super::_FromPropertiesJson(v);
    m_properties.FromJson(v);
    }