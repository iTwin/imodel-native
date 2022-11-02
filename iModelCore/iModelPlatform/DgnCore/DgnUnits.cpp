/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnGeoCoord.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void EcefLocation::ToJson(BeJsValue val) const
    {
    val.SetNull();
    if (m_isValid)
        {
         BeJsGeomUtils::DPoint3dToJson(val[json_origin()], m_origin);
         BeJsGeomUtils::YawPitchRollToJson(val[json_orientation()], m_angles);

        if (m_haveCartographicOrigin)
            {
            auto latLongVal = val[json_cartographicOrigin()];
            latLongVal["longitude"] = Angle::DegreesToRadians(m_cartographicOrigin.longitude); // NOTE: GeoPoint angles are degrees, LatAndLong angles are radians...
            latLongVal["latitude"] = Angle::DegreesToRadians(m_cartographicOrigin.latitude);
            latLongVal["height"] = m_cartographicOrigin.elevation;
            }
        if (m_haveVectors)
            {
            BeJsGeomUtils::DPoint3dToJson(val[json_xVector()], m_xVector);
            BeJsGeomUtils::DPoint3dToJson(val[json_yVector()], m_yVector);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
EcefLocation::EcefLocation(GeoPointCR originLL, DPoint3dCR originIn, GeoCoordinates::BaseGCSCP baseGcs)
    {
    m_isValid = false; // Assume invalid in case something goes wrong below

    if (baseGcs == nullptr)
        {
        // Must provide a valid BaseGCS, otherwise we can't compute ECEF location properly
        return;
        }

    DPoint3d origin = originIn;
    DPoint3d x = DPoint3d::FromSumOf(origin, DPoint3d::From(1.0, 0.0, 0.0));  // Arbitrarily 1 meters in X direction will be used to calculate x axis of transform.
    DPoint3d y = DPoint3d::FromSumOf(origin, DPoint3d::From(0.0, 1.0, 0.0));  // Arbitrarily 1 meters in Y direction will be used to calculate y axis of transform.
    auto dgnGcs = dynamic_cast< DgnGCSCP >( baseGcs );
    if (dgnGcs != nullptr)
        {
        // If the GCS passed in is a DgnGCS, we need to take into account Global Origin and Uors
        dgnGcs->CartesianFromUors(origin, origin);
        dgnGcs->CartesianFromUors(x, x);
        dgnGcs->CartesianFromUors(y, y);
        }

    m_cartographicOrigin = originLL;
    m_haveCartographicOrigin = true;

    DPoint3d ecefOrigin, ecefX, ecefY;
    baseGcs->ECEFFromCartesian(ecefOrigin, origin);
    baseGcs->ECEFFromCartesian(ecefX, x);
    baseGcs->ECEFFromCartesian(ecefY, y);

    m_xVector.DifferenceOf(ecefX, ecefOrigin);                    // Do not normalize - allow non-orthonormal transforms to handle any reprojection.
    m_yVector.DifferenceOf(ecefY, ecefOrigin);
    m_haveVectors = true;

    DVec3d zVector = DVec3d::FromNormalizedCrossProduct(m_xVector, m_yVector);
    RotMatrix rMatrix = RotMatrix::FromColumnVectors(m_xVector, m_yVector, zVector);

    auto ecefTrans = Transform::From(rMatrix, ecefOrigin);
    ecefTrans.TranslateInLocalCoordinates(ecefTrans, -originIn.x, -originIn.y, -originIn.z);

    m_origin = ecefTrans.Origin();

    // Always normalize to compute yaw, pitch and roll angles
    rMatrix.SquareAndNormalizeColumns(rMatrix, 0, 1);
    YawPitchRollAngles::TryFromRotMatrix(m_angles, rMatrix);

    m_isValid = true;

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/18
+---------------+---------------+---------------+---------------+---------------+------*/
void EcefLocation::FromJson(BeJsConst val)
    {
    m_isValid = false;

    if (!val.isMember(json_origin()) || !val.isMember(json_orientation()))
        return;

    m_origin = BeJsGeomUtils::ToDPoint3d(val[json_origin()]);

    // Due to a bug in a previous version, we sometimes see an EcefLocation with a null orientation.
    // Due to an optimization in BeJsGeomUtils::YawPitchRollToJson, we create a null orientation property when all angles are 0.
    // We cannot distinguish between these two cases, so we interpret null to mean to 0 rotation.
    if (val[json_orientation()].isNull())
        m_angles.FromDegrees(0, 0, 0);
    else
        m_angles = BeJsGeomUtils::YawPitchRollFromJson(val[json_orientation()]);

    if (m_haveCartographicOrigin = (val.isMember(json_cartographicOrigin())))
        {
        auto coVal = val[json_cartographicOrigin()];
        m_cartographicOrigin.longitude = Angle::RadiansToDegrees(coVal["longitude"].asDouble()); // NOTE: LatAndLong angles are radians, GeoPoint angles are degrees...
        m_cartographicOrigin.latitude = Angle::RadiansToDegrees(coVal["latitude"].asDouble());
        m_cartographicOrigin.elevation = coVal["height"].asDouble();
        }

    if (m_haveVectors = (val.isMember(json_xVector()) && val.isMember(json_yVector())))
        {
        m_xVector =  BeJsGeomUtils::ToDVec3d(val[json_xVector()]);
        m_yVector =  BeJsGeomUtils::ToDVec3d(val[json_yVector()]);
        }

    m_isValid = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGeoLocation::DgnGeoLocation(DgnDbR project) : m_dgndb(project)
    {
    m_initialProjectCenter.Zero();
    m_globalOrigin.Zero();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGeoLocation::~DgnGeoLocation()
    {
    // If GCS has been set but not stored then the pointed GCS
    // is not held by DgnAppData and must be freed (unless some other refers to it).
    if (m_GCSHasBeenSet && m_gcs != nullptr)
        m_gcs->Release();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
* Private
+---------------+---------------+---------------+---------------+---------------+------*/
GeoCoordinates::BaseGCS* DgnGeoLocation::GetWGS84GCS() const
    {
    if (!m_wgs84GCS.IsValid())
        {
        Utf8String warningMsg;
        StatusInt warning;
        m_wgs84GCS = GeoCoordinates::BaseGCS::CreateGCS();        // WGS84 - used to convert Long/Latitude to ECEF.
        m_wgs84GCS->InitFromEPSGCode(&warning, &warningMsg, 4326); // We do not care about warnings. This GCS exists in the dictionary
        m_wgs84GCS->SetReprojectElevation(true);
        m_wgs84GCS->SetVerticalDatumCode(GeoCoordinates::vdcEllipsoid);
        }

    return m_wgs84GCS.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS* DgnGeoLocation::GetDgnGCS() const
    {
    // If the GCS has been set we use the set one even it has not been saved yet
    if (m_GCSHasBeenSet)
        return m_gcs;

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeoLocation::SetGCS(GeoCoordinates::BaseGCSCP newGCS)
    {
    // If it had been set we must release the unstored GCS
    if (m_GCSHasBeenSet && m_gcs != nullptr)
        m_gcs->Release();

    m_gcs = nullptr;

    m_GCSHasBeenSet = true;

    if (nullptr != newGCS)
        {
        DgnGCSPtr newDGNGCS = DgnGCS::CreateGCS(newGCS, m_dgndb);
        if (newDGNGCS.IsValid())
            {
            newDGNGCS->SetGlobalOrigin(m_globalOrigin);
            m_gcs = newDGNGCS.get();
            m_gcs->AddRef();
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeoLocation::SetGlobalOrigin(DPoint3dCR origin)
    {
    m_globalOrigin=origin;
    if (m_gcs)
        m_gcs->SetGlobalOrigin(origin);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnGeoLocation::XyzFromLatLongWGS84(DPoint3dR outXyz, GeoPointCR wgs84LatLong) const
    {
    auto* dgnGCS = GetDgnGCS();
    if (nullptr == dgnGCS)
        return BSIERROR;

    GeoPoint dgnLatLong;
    GetWGS84GCS()->LatLongFromLatLong(dgnLatLong, wgs84LatLong, *dgnGCS);
    return m_geoServices->UorsFromLatLong(outXyz, dgnLatLong, *dgnGCS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnGeoLocation::XyzFromLatLong(DPoint3dR outUors, GeoPointCR inLatLong) const
    {
    if (nullptr == GetDgnGCS())
        return BSIERROR;

    return m_geoServices->UorsFromLatLong(outUors, inLatLong, *m_gcs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnGeoLocation::LatLongFromXyz(GeoPointR outLatLong, DPoint3dCR inUors) const
    {
    if (nullptr == GetDgnGCS())
        return BSIERROR;

    return m_geoServices->LatLongFromUors(outLatLong, inUors, *m_gcs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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

        // The default extents (centered at 0,0) are used when no geometry is in the file. In this case we really have no idea where the project is
        // located and the defaults, centered at zero may be far away from the project for GCS with False Easting/False northing.
        // If the range is default and recomputed still returns default then don't set ECEF - defer it until we have geometry or reality models
        // to indicate location.
        if (extents.IsEqual(GetDefaultProjectExtents()) &&
            (extents = ComputeProjectExtents()).IsEqual(GetDefaultProjectExtents()))
            return m_ecefLocation;

        GeoPoint cartographicOrigin;
        auto projectCenter = extents.GetCenter();

        if (SUCCESS == LatLongFromXyz(cartographicOrigin, projectCenter))
            {
            m_ecefLocation = EcefLocation(cartographicOrigin, projectCenter, dgnGCS);
            }
        m_ecefLocation.m_isValid = true;
        }

    return m_ecefLocation;
    }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnGeoLocation::GetEcefTransformAtPoint(TransformR ecefTrans, DPoint3dCR  originIn) const
    {
    auto* dgnGCS = GetDgnGCS();
    if (nullptr == dgnGCS)
        return ERROR;

    GeoPoint originLatLong;
    if (REPROJECT_Success != dgnGCS->LatLongFromUors(originLatLong, originIn))
        return ERROR;

    EcefLocation location(originLatLong, originIn, dgnGCS);
    DVec3d zVector = DVec3d::FromNormalizedCrossProduct(location.m_xVector, location.m_yVector);
    auto rMatrix = RotMatrix::FromColumnVectors(location.m_xVector, location.m_yVector, zVector);
    ecefTrans = Transform::From(rMatrix, location.m_origin);


    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnGeoLocation::Load()
    {
    Utf8String value;
    DbResult result = m_dgndb.QueryProperty(value, DgnProjectProperty::Units());

    if (BE_SQLITE_ROW != result)
        {
        BeAssert(false);
        return DgnDbStatus::UnitsMissing;
        }

    BeJsDocument jsonObj(value);
    BeJsGeomUtils::DPoint3dFromJson(m_globalOrigin, jsonObj[json_globalOrigin()]);
    LoadProjectExtents();

    if (jsonObj.isMember(json_ecefLocation()))
        m_ecefLocation.FromJson(jsonObj[json_ecefLocation()]);

    if (jsonObj.isMember(json_initialProjectCenter()))
        BeJsGeomUtils::DPoint3dFromJson(m_initialProjectCenter, jsonObj[json_initialProjectCenter()]);

    if (jsonObj.isMember(json_extentsSource()))
        m_extentSource = (ProjectExtentsSource) jsonObj[json_extentsSource()].asInt();

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeoLocation::Save()
    {
    BeJsDocument jsonObj;
    BeJsGeomUtils::DPoint3dToJson(jsonObj[json_globalOrigin()], m_globalOrigin);
    BeJsGeomUtils::DPoint3dToJson(jsonObj[json_initialProjectCenter()], m_initialProjectCenter);

    // save the EcefLocation, if valid
    if (m_ecefLocation.m_isValid)
        m_ecefLocation.ToJson(jsonObj[json_ecefLocation()]);

    if (ProjectExtentsSource::Calculated != m_extentSource)
        jsonObj[json_extentsSource()] = (int) m_extentSource;

    // Save GCS if it has been set
    if (m_GCSHasBeenSet && m_gcs && m_gcs->IsValid())
        {
        m_gcs->Store(m_dgndb);

        // This will force reloading the GCS to share the same pointer as the DgnDB AppData
        m_GCSHasBeenSet = false;

        // Decrement ref (will get destroyed is no more ref)
        m_hasCheckedForGCS = false;
        m_gcs->Release();
        m_gcs = nullptr;
        }

    m_dgndb.SavePropertyString(DgnProjectProperty::Units(), jsonObj.Stringify());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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

    // DO NOT CHANGE TO RAPID JSON.
    // JsonCpp and RapidJson differ slightly in precision of floating point numbers.
    // Tile content Ids include a hash of the project extents.
    // Differing precision => different content Ids => invalidate every cached tile in existence.
    Json::Value jsonObj;
    BeJsGeomUtils::DRange3dToJson(jsonObj, m_extent);
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
* @bsimethod
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
* @bsimethod
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeoLocation::LoadProjectExtents() const
    {
    // DO NOT CHANGE TO RAPID JSON.
    // JsonCpp and RapidJson differ slightly in precision of floating point numbers.
    // Tile content Ids include a hash of the project extents.
    // Differing precision => different content Ids => invalidate every cached tile in existence.
    Json::Value jsonObj;
    Utf8String value;
    if (BE_SQLITE_ROW == m_dgndb.QueryProperty(value, DgnProjectProperty::Extents()) && Json::Reader::Parse(value, jsonObj))
        BeJsGeomUtils::DRange3dFromJson(m_extent, jsonObj);

    // if we can't get valid extents from the property, use default values
    if (m_extent.IsEmpty())
        m_extent = GetDefaultProjectExtents();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d DgnGeoLocation::GetProjectExtents() const
    {
    if (m_extent.IsEmpty())
        LoadProjectExtents();

    return m_extent;
    }

