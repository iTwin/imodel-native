/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnUnits.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnGeoCoord.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGeoLocation::DgnGeoLocation(DgnDbR project) : m_dgndb(project)
    {
    m_globalOrigin.Zero();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS* DgnGeoLocation::GetDgnGCS() const
    {
    if (!m_hasCheckedForGCS)
        {
        m_geoServices = T_HOST.GetGeoCoordinationAdmin()._GetServices();
        m_gcs = m_geoServices ? m_geoServices->GetGCSFromProject(m_dgndb) : nullptr;
        m_hasCheckedForGCS = true;
        }
    return m_gcs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeoLocation::SetGlobalOrigin(DPoint3dCR origin)
    {
    m_globalOrigin=origin;
    if (m_gcs)
        m_gcs->SetGlobalOrigin(origin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnGeoLocation::XyzFromLatLong(DPoint3dR outUors, GeoPointCR inLatLong) const
    {
    if (nullptr == GetDgnGCS())
        return BSIERROR;

    return m_geoServices->UorsFromLatLong(outUors, inLatLong, *m_gcs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnGeoLocation::LatLongFromXyz(GeoPointR outLatLong, DPoint3dCR inUors) const
    {
    if (nullptr == GetDgnGCS())
        return BSIERROR;
    return m_geoServices->LatLongFromUors(outLatLong, inUors, *m_gcs);
    }

static Utf8CP DGNPROPERTYJSON_GlobalOrigin = "globalOrigin";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnGeoLocation::Load()
    {
    Utf8String value;
    DbResult result = m_dgndb.QueryProperty(value, DgnProjectProperty::Units());

    Json::Value jsonObj;
    if (BE_SQLITE_ROW != result || !Json::Reader::Parse(value, jsonObj))
        {
        BeAssert(false);
        return DgnDbStatus::UnitsMissing;
        }

    JsonUtils::DPoint3dFromJson(m_globalOrigin, jsonObj[DGNPROPERTYJSON_GlobalOrigin]);
    LoadProjectExtents();
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeoLocation::Save()
    {
    Json::Value jsonObj;
    JsonUtils::DPoint3dToJson(jsonObj[DGNPROPERTYJSON_GlobalOrigin], m_globalOrigin);
    m_dgndb.SavePropertyString(DgnProjectProperty::Units(), jsonObj.ToString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeoLocation::SetProjectExtents(AxisAlignedBox3dCR newExtents)
    {
    if (newExtents.IsEmpty())
        {
        BeAssert(false);
        return;
        }

    m_extent = newExtents;
    Json::Value jsonObj;
    JsonUtils::DRange3dToJson(jsonObj, m_extent);
    m_dgndb.SavePropertyString(DgnProjectProperty::Extents(), jsonObj.ToString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeoLocation::InitializeProjectExtents() 
    {
    SetProjectExtents(ComputeProjectExtents());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d DgnGeoLocation::ComputeProjectExtents() const
    {
    auto& models = GetDgnDb().Models();

    // Set the project extents to the union of the ranges of all of the spatial models.
    // We can't just use the range index for this since some models (e.g. reality models) have volumes of interest
    // that don't have any elements. This is slower, but it should only be called after an "import from external source" operation
    AxisAlignedBox3d extent;
    for (auto& entry : models.MakeIterator(BIS_SCHEMA(BIS_CLASS_SpatialModel)))
        {
        auto model = models.Get<SpatialModel>(entry.GetModelId());
        if (model.IsValid())
            extent.Extend(model->QueryModelRange());
        }

    if (extent.IsEmpty()) // if we found nothing in any models, just set the project extents to a resonable default
        extent = GetDefaultProjectExtents();

    return extent;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeoLocation::LoadProjectExtents() const
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
AxisAlignedBox3d DgnGeoLocation::GetProjectExtents() const
    {
    if (m_extent.IsEmpty())
        LoadProjectExtents();

    return m_extent;
    }

