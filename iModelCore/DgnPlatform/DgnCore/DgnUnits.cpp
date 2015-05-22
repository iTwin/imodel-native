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
    m_azimuth = m_latitude = m_longitude = 0.0;
    m_geoOriginBasis.Zero();
    m_hasGeoOriginBasis = false;
    m_hasCheckedForGCS = false;
    m_geoServices = NULL;
    m_gcs = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/14
+---------------+---------------+---------------+---------------+---------------+------*/
GeoCoordinates::DgnGCS* DgnUnits::GetDgnGCS()
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
BentleyStatus DgnUnits::UorsFromLatLong(DPoint3dR outUors, GeoPointCR inLatLong)
    {
    if (NULL == GetDgnGCS())
        return BSIERROR;

    return m_geoServices->UorsFromLatLong(outUors, inLatLong, *m_gcs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnUnits::LatLongFromUors(GeoPointR outLatLong, DPoint3dCR inUors)
    {
    if (NULL == GetDgnGCS())
        return BSIERROR;
    return m_geoServices->LatLongFromUors(outLatLong, inUors, *m_gcs);
    }

static Utf8CP DGNPROPERTYJSON_GlobalOrigin    = "globalOrigin";
static Utf8CP DGNPROPERTYJSON_OriginLatitude  = "originLatitude";
static Utf8CP DGNPROPERTYJSON_OriginLongitude = "originLongitude";
static Utf8CP DGNPROPERTYJSON_Azimuth         = "azimuth";
static Utf8CP DGNPROPERTYJSON_GeoOriginBasis  = "geoOriginBasis";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFileStatus DgnUnits::Load()
    {
    Json::Value  jsonObj;
    Utf8String value;

    DbResult  result = m_dgndb.QueryProperty(value, DgnProjectProperty::Units());
    if (BE_SQLITE_ROW != result || !Json::Reader::Parse(value, jsonObj))
        {
        BeAssert(false);
        return DGNDB_ERROR_UnitsMissing;
        }

    JsonUtils::DPoint3dFromJson(m_globalOrigin, jsonObj[DGNPROPERTYJSON_GlobalOrigin]);

    m_latitude  = jsonObj[DGNPROPERTYJSON_OriginLatitude].asDouble();
    m_longitude = jsonObj[DGNPROPERTYJSON_OriginLongitude].asDouble();
    m_azimuth = jsonObj[DGNPROPERTYJSON_Azimuth].asDouble();
    if (jsonObj.isMember(DGNPROPERTYJSON_GeoOriginBasis))
        {
        JsonUtils::DPoint2dFromJson(m_geoOriginBasis, jsonObj[DGNPROPERTYJSON_GeoOriginBasis]);
        m_hasGeoOriginBasis = true;
        }

    LoadProjectExtents();
    return  DGNFILE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnUnits::Save()
    {
    Json::Value jsonObj;

    JsonUtils::DPoint3dToJson(jsonObj[DGNPROPERTYJSON_GlobalOrigin], m_globalOrigin);

    jsonObj[DGNPROPERTYJSON_OriginLatitude]  = m_latitude;
    jsonObj[DGNPROPERTYJSON_OriginLongitude] = m_longitude;
    jsonObj[DGNPROPERTYJSON_Azimuth] = m_azimuth;

    if (m_hasGeoOriginBasis)
        JsonUtils::DPoint2dToJson(jsonObj[DGNPROPERTYJSON_GeoOriginBasis], m_geoOriginBasis);

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
    Statement stmt;
    DbResult rc = stmt.Prepare(m_dgndb, "SELECT 1 FROM " DGN_VTABLE_RTree3d " WHERE ElementId MATCH rTreeMatch(1)");
    bounds.m_bounds.Invalidate();
    rc=bounds.StepRTree(stmt);
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
void DgnUnits::GeoTransform::Init(DgnUnits const& units)
    {
    if (m_isValid)
        return;

    static double s_wgs84Major = 6371837.0;

    m_xRadius = s_wgs84Major * cos(units.GetOriginLatitude() * msGeomConst_radiansPerDegree);
    m_yRadius = s_wgs84Major * cos(units.GetOriginLatitude() * msGeomConst_radiansPerDegree);

    m_matrix = RotMatrix::FromAxisAndRotationAngle(2, -1.0 * units.GetAzimuth() * msGeomConst_radiansPerDegree);
    m_isValid = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     12/13
//---------------------------------------------------------------------------------------
bool DgnUnits::IsGeoPointWithinCoordinateSystem(GeoPointCR point) const
    {
    static const double s_coordSysGeoTolerance = 0.01;  // fudged at 1/10 of degree now (doesn't account for longitude change)
    double longitudeDiff = fabs(point.longitude - m_longitude);

    //  For the sake of this function the following logic only matters near 180 and -180.
    if (longitudeDiff > 180)
        longitudeDiff = fabs(360 - longitudeDiff);

    DPoint2d distanceFromOrigin = DPoint2d::From(longitudeDiff, fabs(point.latitude - m_latitude));

    return distanceFromOrigin.MagnitudeSquared() < s_coordSysGeoTolerance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
BentleyStatus DgnUnits::ConvertToWorldPoint(DPoint3dR worldPoint, GeoPointCR geoPoint) const
    {
    if (!CanConvertBetweenGeoAndWorld() || !IsGeoPointWithinCoordinateSystem(geoPoint))
        return ERROR;

    m_geoTransform.Init(*this);

    DPoint3d delta;
    delta.x = (geoPoint.longitude - m_longitude) * m_geoTransform.m_xRadius * msGeomConst_radiansPerDegree;
    delta.y = (geoPoint.latitude -  m_latitude)  * m_geoTransform.m_yRadius * msGeomConst_radiansPerDegree;
    delta.z = (geoPoint.elevation);

    m_geoTransform.m_matrix.MultiplyTranspose(delta);
    delta.Scale(1000.); // meters
    worldPoint.SumOf(DPoint3d::From(m_geoOriginBasis), delta);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
BentleyStatus DgnUnits::ConvertToGeoPoint(GeoPointR geoPoint, DPoint3dCR worldPoint) const
    {
    if (!CanConvertBetweenGeoAndWorld() /*|| !IsWorldPointWithinCoordinateSystem (worldPoint)*/)
        return ERROR;

    m_geoTransform.Init(*this);

    DPoint3d delta;
    delta.DifferenceOf(worldPoint, DPoint3d::From(m_geoOriginBasis));
    delta.Scale(1.0 / 1000.);
    m_geoTransform.m_matrix.Multiply(delta);

    geoPoint.longitude = m_longitude + msGeomConst_degreesPerRadian * delta.x / m_geoTransform.m_xRadius;
    geoPoint.latitude  = m_latitude  + msGeomConst_degreesPerRadian * delta.y / m_geoTransform.m_yRadius;
    geoPoint.elevation = delta.z;

    return SUCCESS;
    }
