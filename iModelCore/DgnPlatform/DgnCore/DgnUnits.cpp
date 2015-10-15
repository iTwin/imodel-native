/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnUnits.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnUnits::DgnUnits(DgnDbR project) : m_dgndb(project)
    {
    m_globalOrigin.Zero();
    m_extent = AxisAlignedBox3d();
    m_azimuth = 0.0;
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
static Utf8CP DGNPROPERTYJSON_Azimuth         = "azimuth";

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

    m_azimuth = jsonObj[DGNPROPERTYJSON_Azimuth].asDouble();
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
    jsonObj[DGNPROPERTYJSON_Azimuth] = m_azimuth;

    m_dgndb.SavePropertyString(DgnProjectProperty::Units(), Json::FastWriter::ToString(jsonObj));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnUnits::SaveProjectExtents(AxisAlignedBox3dCR newExtents)
    {
    m_extent = newExtents;
    Json::Value jsonObj;
    JsonUtils::DRange3dToJson(jsonObj, m_extent);
    return m_dgndb.SavePropertyString(DgnProjectProperty::Extents(), Json::FastWriter::ToString(jsonObj));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d DgnUnits::ComputeProjectExtents()
    {
    RTree3dBoundsTest bounds(m_dgndb);
    Statement stmt(m_dgndb, "SELECT 1 FROM " DGN_VTABLE_RTree3d " WHERE ElementId MATCH rTreeMatch(1)");
    bounds.m_bounds.Invalidate();
    auto rc=bounds.StepRTree(stmt);
    BeAssert(rc==BE_SQLITE_DONE);
    bounds.m_bounds.ToRange(m_extent);
    return m_extent;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnUnits::LoadProjectExtents()
    {
    Json::Value  jsonObj;
    Utf8String value;

    if (BE_SQLITE_ROW == m_dgndb.QueryProperty(value, DgnProjectProperty::Extents()) && Json::Reader::Parse(value, jsonObj))
        {
        JsonUtils::DRange3dFromJson(m_extent, jsonObj);
        if (!m_extent.IsEmpty())
            return; // it's valid, keep it
        }
    
    // we can't get valid extents from the property, get from volume of range tree.
    ComputeProjectExtents();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d DgnUnits::GetProjectExtents()
    {
    if (m_extent.IsEmpty())
        LoadProjectExtents();

    return m_extent;
    }

