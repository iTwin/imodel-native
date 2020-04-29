/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnGeoCoord.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/18
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value EcefLocation::ToJson() const
    {
    Json::Value val;
    if (m_isValid)
        {
        val[json_origin()] = JsonUtils::DPoint3dToJson(m_origin);
        val[json_orientation()] = JsonUtils::YawPitchRollToJson(m_angles);

        if (m_haveCartographicOrigin)
            {
            Json::Value latLongVal;

            latLongVal["longitude"] = Json::Value(Angle::DegreesToRadians(m_cartographicOrigin.longitude)); // NOTE: GeoPoint angles are degrees, LatAndLong angles are radians...
            latLongVal["latitude"] = Json::Value(Angle::DegreesToRadians(m_cartographicOrigin.latitude));
            latLongVal["height"] = Json::Value(m_cartographicOrigin.elevation);

            val[json_cartographicOrigin()] = latLongVal;
            }
        }
    return val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/18
+---------------+---------------+---------------+---------------+---------------+------*/
void EcefLocation::FromJson(JsonValueCR val)
    {
    m_isValid = false;

    // due to a bug in a previous version, we sometimes see an EcefLocation with a null orientation. That is never
    // a valid EcefLocation, just ignore it.
    if (!val.isMember(json_origin()) || !val.isMember(json_orientation()) || val[json_orientation()].isNull())
        return;

    m_origin = JsonUtils::ToDPoint3d(val[json_origin()]);
    m_angles = JsonUtils::YawPitchRollFromJson(val[json_orientation()]);

    if (m_haveCartographicOrigin = (val.isMember(json_cartographicOrigin())))
        {
        Json::Value coVal = val[json_cartographicOrigin()];

        m_cartographicOrigin.longitude = Angle::RadiansToDegrees(coVal["longitude"].asDouble()); // NOTE: LatAndLong angles are radians, GeoPoint angles are degrees...
        m_cartographicOrigin.latitude = Angle::RadiansToDegrees(coVal["latitude"].asDouble());
        m_cartographicOrigin.elevation = coVal["height"].asDouble();
        }

    m_isValid = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGeoLocation::DgnGeoLocation(DgnDbR project) : m_dgndb(project)
    {
    m_initialProjectCenter.Zero();
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
        if (m_gcs != nullptr && m_gcs->IsValid())
            m_gcs->SetReprojectElevation(true); // In the bridge we always reproject elevation
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
* @bsimethod                                    Barry.Bentley                   03/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnGeoLocation::XyzFromLatLongWGS84(DPoint3dR outXyz, GeoPointCR wgs84LatLong) const
    {
    auto* dgnGCS = GetDgnGCS();
    if (nullptr == dgnGCS)
        return BSIERROR;

    if (!m_wgs84GCS.IsValid())
        {
        WString warningMsg;
        StatusInt warning;
        m_wgs84GCS = GeoCoordinates::BaseGCS::CreateGCS();        // WGS84 - used to convert Long/Latitude to ECEF.
        m_wgs84GCS->InitFromEPSGCode(&warning, &warningMsg, 4326); // We do not care about warnings. This GCS exists in the dictionary
        }
    GeoPoint dgnLatLong;
    m_wgs84GCS->LatLongFromLatLong(dgnLatLong, wgs84LatLong, *dgnGCS);
    return m_geoServices->UorsFromLatLong(outXyz, dgnLatLong, *dgnGCS);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/18
+---------------+---------------+---------------+---------------+---------------+------*/
EcefLocation DgnGeoLocation::GetEcefLocation() const
    {
    if (!m_ecefLocation.m_isValid)
        {
        // The Ecef Location wasn't saved in the iModel. See if we have a DgnGCS to calculate it.
        auto* dgnGCS = GetDgnGCS();
        if (nullptr == dgnGCS)
            return m_ecefLocation;

        auto extents = GetProjectExtents();
        DPoint3d origin = extents.GetCenter();
        origin.z = 0; // always use ground plane

        // Create a WGS84 GCS to convert the WGS84 Lat/Long to ECEF/XYZ
        WString warningMsg;
        StatusInt warning;
        auto wgs84GCS = GeoCoordinates::BaseGCS::CreateGCS();        // WGS84 - used to convert Long/Latitude to ECEF.
        wgs84GCS->InitFromEPSGCode(&warning, &warningMsg, 4326); // We do not care about warnings. This GCS exists in the dictionary

        // In the bridge we always reproject elevation.
        wgs84GCS->SetReprojectElevation(true);

        GeoPoint originLatLong, yLatLong, tempLatLong;
        dgnGCS->LatLongFromUors(originLatLong, origin);
        dgnGCS->LatLongFromUors(yLatLong, DPoint3d::FromSumOf(origin, DPoint3d::From(0.0, 10.0, 0.0)));         // Arbitrarily 10 meters in Y direction will be used to calculate y axis of transform.

        tempLatLong = originLatLong;
        dgnGCS->LatLongFromLatLong(originLatLong, tempLatLong, *wgs84GCS);
        tempLatLong = yLatLong;
        dgnGCS->LatLongFromLatLong(yLatLong, tempLatLong, *wgs84GCS);

        DPoint3d ecefOrigin, ecefY;
        wgs84GCS->XYZFromLatLong(ecefOrigin, originLatLong);
        wgs84GCS->XYZFromLatLong(ecefY, yLatLong);

        RotMatrix rMatrix = RotMatrix::FromIdentity();
        DVec3d zVector, yVector;
        zVector.Normalize((DVec3dCR) ecefOrigin);
        yVector.NormalizedDifference(ecefY, ecefOrigin);

        rMatrix.SetColumn(yVector, 1);
        rMatrix.SetColumn(zVector, 2);
        rMatrix.SquareAndNormalizeColumns(rMatrix, 1, 2);

        auto ecefTrans = Transform::From(rMatrix, ecefOrigin);
        ecefTrans.TranslateInLocalCoordinates(ecefTrans, -origin.x, -origin.y, -origin.z);

        m_ecefLocation.m_origin = ecefTrans.Origin();
        YawPitchRollAngles::TryFromRotMatrix(m_ecefLocation.m_angles, rMatrix);
        m_ecefLocation.m_isValid = true;
        }

    return m_ecefLocation;
    }

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

    JsonUtils::DPoint3dFromJson(m_globalOrigin, jsonObj[json_globalOrigin()]);
    LoadProjectExtents();

    if (jsonObj.isMember(json_ecefLocation()))
        m_ecefLocation.FromJson(jsonObj[json_ecefLocation()]);

    if (jsonObj.isMember(json_initialProjectCenter()))
        JsonUtils::DPoint3dFromJson(m_initialProjectCenter, jsonObj[json_initialProjectCenter()]);

    if (jsonObj.isMember(json_extentsSource()))
        m_extentSource = (ProjectExtentsSource) jsonObj[json_extentsSource()].asInt();

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeoLocation::Save()
    {
    Json::Value jsonObj;
    JsonUtils::DPoint3dToJson(jsonObj[json_globalOrigin()], m_globalOrigin);
    JsonUtils::DPoint3dToJson(jsonObj[json_initialProjectCenter()], m_initialProjectCenter);

    // save the EcefLocation, if valid
    if (m_ecefLocation.m_isValid)
        jsonObj[json_ecefLocation()] = m_ecefLocation.ToJson();

    if (ProjectExtentsSource::Calculated != m_extentSource)
        jsonObj[json_extentsSource()] = Json::Value((int) m_extentSource);

    m_dgndb.SavePropertyString(DgnProjectProperty::Units(), jsonObj.ToString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeoLocation::SetProjectExtents(AxisAlignedBox3dCR newExtents)
    {
    DgnDb::VerifyClientThread();

    if (newExtents.IsEmpty())
        {
        BeAssert(false);
        return;
        }

    m_extent = newExtents;
    Json::Value jsonObj;
    JsonUtils::DRange3dToJson(jsonObj, m_extent);
    m_dgndb.SavePropertyString(DgnProjectProperty::Extents(), jsonObj.ToString());

    if (!m_ecefLocation.m_isValid)
        {
        GetEcefLocation(); // try to calculate the EcefLocation if it isn't set yet.
        Save();
        }

    for (auto const& kvp : m_dgndb.Models().GetLoadedModels())
        {
        auto spatialModel = kvp.second->ToSpatialModelP();
        if (nullptr != spatialModel)
            spatialModel->OnProjectExtentsChanged(newExtents);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeoLocation::InitializeProjectExtents(DRange3dP rangeWithOutliers, bvector<BeInt64Id>* elementOutliers)
    {
    SetProjectExtents(ComputeProjectExtents(rangeWithOutliers, elementOutliers));

    // We need an immutable origin for Cesium tile publishing that will not
    // change when project extents change.   Set it here only.
    m_initialProjectCenter = m_extent.LocalToGlobal(.5, .5, .5);
    Save();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d DgnGeoLocation::ComputeProjectExtents(DRange3dP rangeWithOutliers, bvector<BeInt64Id>* elementOutliers) const
    {
    if (ProjectExtentsSource::Calculated != GetProjectExtentsSource())
      return GetProjectExtents(); // Don't change extents if defined by user...

    AxisAlignedBox3d extent;
    auto& models = GetDgnDb().Models();

    // Set the project extents to the union of the ranges of all elements - but ignoring outlying (statistically insignificant).
    // Elements so that we do not get huge project extents for "offending elements".
    extent.Extend(GetDgnDb().ComputeGeometryExtentsWithoutOutliers(rangeWithOutliers, elementOutliers));

    // Include non element (reality model) ranges.
    for (auto& entry : models.MakeIterator(BIS_SCHEMA(BIS_CLASS_SpatialModel)))
        {
        auto model = models.Get<SpatialModel>(entry.GetModelId());
        if (model.IsValid())
            extent.Extend(model->QueryNonElementModelRange());
        }

    if (extent.IsEmpty()) // if we found nothing in any models, just set the project extents to a reasonable default
        extent = GetDefaultProjectExtents();

    return extent;
    }

BEGIN_UNNAMED_NAMESPACE
//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/11
//=======================================================================================
struct SpatialBounds : SpatialViewController::SpatialQuery
{
    BeSQLite::RTree3dVal  m_bounds;
    SpatialBounds() : SpatialViewController::SpatialQuery(nullptr,nullptr) {m_bounds.Invalidate();}
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
AxisAlignedBox3d DgnGeoLocation::QueryRTreeExtents() const
    {
    Statement stmt(m_dgndb, "SELECT 1 FROM " DGN_VTABLE_SpatialIndex " WHERE ElementId MATCH DGN_rtree(?)");
    SpatialBounds bounds;
    stmt.BindInt64(1, (int64_t) &bounds);

    auto rc=stmt.Step();
    BeAssert(rc==BE_SQLITE_DONE);
    AxisAlignedBox3d extents;
    bounds.m_bounds.ToRangeR(extents);
    return extents;
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

