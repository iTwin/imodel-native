/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnUnits.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <BeSQLite/RTreeMatch.h>

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
DbResult DgnUnits::SetProjectExtents(AxisAlignedBox3dCR newExtents)
    {
    m_extent = newExtents;
    Json::Value jsonObj;
    JsonUtils::DRange3dToJson(jsonObj, m_extent);
    return m_dgndb.SavePropertyString(DgnProjectProperty::Extents(), Json::FastWriter::ToString(jsonObj));
    }

BEGIN_UNNAMED_NAMESPACE
//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/11
//=======================================================================================
struct SpatialBounds : QueryViewController::SpatialQuery
{
    BeSQLite::RTree3dVal  m_bounds;
    SpatialBounds() : QueryViewController::SpatialQuery(nullptr,nullptr) {m_bounds.Invalidate();}
    int _TestRTree(BeSQLite::RTreeMatchFunction::QueryInfo const& info) override
        {
        BeAssert(6 == info.m_nCoord);
        info.m_within = BeSQLite::RTreeMatchFunction::Within::Outside; // we only want the top level nodes
        RTree3dValCP pt = (RTree3dValCP) info.m_coords;
         if (!m_bounds.IsValid())
            m_bounds = *pt;
        else
            m_bounds.Union(*pt);
        return  BeSQLite::BE_SQLITE_OK;
        }
};
END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d DgnUnits::ComputeProjectExtents() const
    {
    Statement stmt(m_dgndb, "SELECT 1 FROM " DGN_VTABLE_SpatialIndex " WHERE ElementId MATCH DGN_rtree(?)");
    SpatialBounds bounds;
    stmt.BindInt64(1, (int64_t) &bounds);

    auto rc=stmt.Step();
    BeAssert(rc==BE_SQLITE_DONE);
    bounds.m_bounds.ToRangeR(m_extent);
    return m_extent;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnUnits::LoadProjectExtents() const
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
AxisAlignedBox3d DgnUnits::GetProjectExtents() const
    {
    if (m_extent.IsEmpty())
        LoadProjectExtents();

    return m_extent;
    }

