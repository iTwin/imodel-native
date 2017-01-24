/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnUnits.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <BeSQLite/RTreeMatch.h>
#include <DgnPlatform/DgnGeoCoord.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnUnits::DgnUnits(DgnDbR project) : m_dgndb(project)
    {
    m_globalOrigin.Zero();
    m_extent = AxisAlignedBox3d();
    m_hasCheckedForGCS = false;
    m_geoServices = NULL;
    m_gcs = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS* DgnUnits::GetDgnGCS() const
    {
    if (!m_hasCheckedForGCS)
        {
        m_geoServices = T_HOST.GetGeoCoordinationAdmin()._GetServices();
        m_gcs = m_geoServices? m_geoServices->GetGCSFromProject(m_dgndb): NULL;
        m_hasCheckedForGCS = true;
        }
    return m_gcs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnUnits::SetGlobalOrigin(DPoint3dCR origin)
    {
    m_globalOrigin=origin;
    if (m_gcs)
        m_gcs->SetGlobalOrigin(origin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnUnits::XyzFromLatLong(DPoint3dR outUors, GeoPointCR inLatLong) const
    {
    if (NULL == GetDgnGCS())
        return BSIERROR;

    return m_geoServices->UorsFromLatLong(outUors, inLatLong, *m_gcs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnUnits::LatLongFromXyz(GeoPointR outLatLong, DPoint3dCR inUors) const
    {
    if (NULL == GetDgnGCS())
        return BSIERROR;
    return m_geoServices->LatLongFromUors(outLatLong, inUors, *m_gcs);
    }

static Utf8CP DGNPROPERTYJSON_GlobalOrigin    = "globalOrigin";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnUnits::Load()
    {
    Json::Value  jsonObj;
    Utf8String value;

    DbResult  result = m_dgndb.QueryProperty(value, DgnProjectProperty::Units());
    if (BE_SQLITE_ROW != result || !Json::Reader::Parse(value, jsonObj))
        {
        BeAssert(false);
        return DgnDbStatus::UnitsMissing;
        }

    JsonUtils::DPoint3dFromJson(m_globalOrigin, jsonObj[DGNPROPERTYJSON_GlobalOrigin]);

    LoadProjectExtents();
    return  DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnUnits::Save()
    {
    Json::Value jsonObj;

    JsonUtils::DPoint3dToJson(jsonObj[DGNPROPERTYJSON_GlobalOrigin], m_globalOrigin);

    m_dgndb.SavePropertyString(DgnProjectProperty::Units(), Json::FastWriter::ToString(jsonObj));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnUnits::SetProjectExtents(AxisAlignedBox3dCR newExtents)
    {
    m_extent = newExtents;
    Json::Value jsonObj;
    JsonUtils::DRange3dToJson(jsonObj, m_extent);
    m_dgndb.SavePropertyString(DgnProjectProperty::Extents(), Json::FastWriter::ToString(jsonObj));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnUnits::InitializeProjectExtents() 
    {
    auto& models = GetDgnDb().Models();

    AxisAlignedBox3d extent;
    for (auto& entry : models.MakeIterator(BIS_SCHEMA(BIS_CLASS_SpatialModel)))
        {
        auto model = models.Get<SpatialModel>(entry.GetModelId());
        if (model.IsValid())
            extent.Extend(model->QueryModelRange());
        }

    if (extent.IsEmpty())
        extent = GetDefaultProjectExtents();

    SetProjectExtents(extent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnUnits::LoadProjectExtents() const
    {
    Json::Value  jsonObj;
    Utf8String value;

    if (BE_SQLITE_ROW == m_dgndb.QueryProperty(value, DgnProjectProperty::Extents()) && Json::Reader::Parse(value, jsonObj))
        JsonUtils::DRange3dFromJson(m_extent, jsonObj);
    
    // if we can't get valid extents from the property, use default values
    if (m_extent.IsEmpty())
        m_extent = GetDefaultProjectExtents();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d DgnUnits::GetProjectExtents() const
    {
    if (m_extent.IsEmpty())
        LoadProjectExtents();

    return m_extent;
    }

