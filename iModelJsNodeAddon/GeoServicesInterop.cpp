/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "IModelJsNative.h"
#include <Bentley/BeDirectoryIterator.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <ECObjects/ECJsonUtilities.h>
#include <cmath>

using namespace IModelJsNative;

BE_JSON_NAME(geographicCRSDef)
BE_JSON_NAME(geographicCRS)
BE_JSON_NAME(format)
BE_JSON_NAME(status)
BE_JSON_NAME(point)
BE_JSON_NAME(extent)
BE_JSON_NAME(longitude)
BE_JSON_NAME(latitude)
BE_JSON_NAME(includeIntersecting)

static Utf8String GetLegacyVerticalCrsId(GeoCoordinates::VerticalDatumInfo const& info)
    {
    Utf8String crsName;
    info.GetCRSName(crsName);
    if (0 == crsName.CompareToI("NGVD29 height"))
        return "NGVD29";
    if (0 == crsName.CompareToI("NAVD88 height"))
        return "NAVD88";
    if (0 == crsName.CompareToI("WGS84"))
        return "ELLIPSOID";

    Utf8String type;
    info.GetType(type);
    return type;
    }

static bool IsNumericPoint2d(BeJsConst value)
    {
    if (value.isArray())
        return value.size() == 2 && value[0].isNumeric() && value[1].isNumeric();

    return value.isObject() && value.isNumericMember("x") && value.isNumericMember("y");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus GeoServicesInterop::GetGeographicCRSInterpretation(BeJsValue results, BeJsConst props)
    {
    auto gcsFormat = props[json_format()];
    if (gcsFormat.isNull())
        {
        results[json_status()] = ERROR;
        return ERROR;
        }

    Utf8String formatCode = gcsFormat.asString();

    auto gcsJson = props[json_geographicCRSDef()];
    if (gcsJson.isNull())
        {
        results[json_status()] = ERROR;
        return ERROR;
        }

    Utf8String errorDevFacingMessage;
    StatusInt status = SUCCESS;
    GeoCoordinates::BaseGCSPtr newGCS = GeoCoordinates::BaseGCS::CreateGCS();

    if (!newGCS.IsValid())
        {
        results[json_status()] = ERROR;
        return ERROR;
        }

    if (formatCode == "JSON")
        status = newGCS->FromJson(BeJsDocument(gcsJson.asString()), errorDevFacingMessage);
    else if (formatCode == "WKT")
        status = newGCS->InitFromWellKnownText(nullptr, &errorDevFacingMessage, GeoCoordinates::BaseGCS::wktFlavorOGC, gcsJson.asString().c_str());
    else
        status = ERROR;

    results[json_status()] = status;

    if (SUCCESS == status)
        {
        newGCS->ToJson(results[json_geographicCRS()], true);
        }

    return (BentleyStatus)status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// @param extent If provided, only return CRS that contain the given extent. Minimum longitude and latitude correspond to extent.low.x and extent.low.y, respectively.
// Maximum longitude and latitude correspond to extent.high.x and extent.high.y, respectively.
// @param unitFilter If provided, only return CRS that use the specified unit (case-insensitive comparison)
//---------------------------------------------------------------------------------------
bvector<CRSListResponseProps> GeoServicesInterop::GetListOfCRS(DRange2dCP extent, bool includeWorld, Utf8CP unitFilter)
    {
    bvector<CRSListResponseProps> listOfCRS;
    char csKeyName[128];
    CRSListResponseProps props;
    for (int index = 0; (0 < GeoCoordinates::CSMap::CS_csEnum(index, csKeyName, sizeof(csKeyName))); index++)
        {
        GeoCoordinates::BaseGCSPtr crs = GeoCoordinates::BaseGCS::CreateGCS(csKeyName);

        // Don't include CRS with no range or with a range that covers the whole world
        DRange2d crsRange(DRange2d::From(crs->GetMinimumLongitude(),
                        crs->GetMinimumLatitude(),
                        crs->GetMaximumLongitude(),
                    crs->GetMaximumLatitude()));

        // Don't include world crs (area == 64800) by default
        if (!includeWorld && (crsRange.IsEmpty() || crsRange.Area() == 64800)) 
            {
            continue;
            }

        // Don't include CRS if it does not contain the extent
        if (extent)
            {
            DRange2d extentRange(DRange2d::From(extent->low.x, extent->low.y, extent->high.x, extent->high.y));
            if (!extentRange.IntersectsWith(crsRange))
                continue;
            }

        Utf8String rawUnit;
        crs->GetUnits(rawUnit);
        Utf8String mappedUnit;
        GeoCoordinates::BaseGCS::MapUnitToJsonName(mappedUnit, rawUnit.c_str());

        // Don't include CRS if it does not match the unit filter
        if (unitFilter != nullptr && strlen(unitFilter) > 0)
            {
            if (mappedUnit.empty() || !mappedUnit.EqualsIAscii(unitFilter))
                continue;
            }

        props.m_name = Utf8String(crs->GetName());
        props.m_description = Utf8String(crs->GetDescription());
        props.m_deprecated = crs->IsDeprecated();
        props.m_crsExtent = crsRange;
        props.m_unit = mappedUnit;

        listOfCRS.push_back(props);
        }
    
    return listOfCRS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
StatusInt GeoServicesInterop::GetListOfVerticalCRS(bvector<VerticalCRSListResponseProps>& results, BeJsConst props, Utf8StringR errorMessage)
    {
    results.clear();
    errorMessage.clear();

    auto pointJson = props[json_point()];
    auto extentJson = props[json_extent()];
    if (!pointJson.isNull() && !extentJson.isNull())
        {
        errorMessage = "point and extent are mutually exclusive";
        return GeoCoordinates::GEOCOORDERR_BadArg;
        }

    GeoPoint2d point;
    GeoPoint2dCP pointFilter = nullptr;
    if (!pointJson.isNull())
        {
        if (!pointJson.isObject())
            {
            errorMessage = "point must be an object";
            return GeoCoordinates::GEOCOORDERR_BadArg;
            }
        if (!IsNumericPoint2d(pointJson))
            {
            errorMessage = "point must contain numeric x and y coordinates";
            return GeoCoordinates::GEOCOORDERR_BadArg;
            }

        point.longitude = pointJson["x"].asDouble();
        point.latitude = pointJson["y"].asDouble();
        if (!std::isfinite(point.longitude) || !std::isfinite(point.latitude))
            {
            errorMessage = "point coordinates must be finite";
            return GeoCoordinates::GEOCOORDERR_BadArg;
            }
        pointFilter = &point;
        }

    DRange2d extent;
    DRange2dCP extentFilter = nullptr;
    if (!extentJson.isNull())
        {
        if (!extentJson.isObject())
            {
            errorMessage = "extent must be an object";
            return GeoCoordinates::GEOCOORDERR_BadArg;
            }
        if (!IsNumericPoint2d(extentJson["low"]) || !IsNumericPoint2d(extentJson["high"]))
            {
            errorMessage = "extent must contain numeric low and high points";
            return GeoCoordinates::GEOCOORDERR_BadArg;
            }

        BeJsGeomUtils::DRange2dFromJson(extent, extentJson);
        if (!std::isfinite(extent.low.x) || !std::isfinite(extent.low.y)
            || !std::isfinite(extent.high.x) || !std::isfinite(extent.high.y))
            {
            errorMessage = "extent coordinates must be finite";
            return GeoCoordinates::GEOCOORDERR_BadArg;
            }
        if (extent.low.y > extent.high.y)
            {
            errorMessage = "extent low latitude must not exceed high latitude";
            return GeoCoordinates::GEOCOORDERR_BadArg;
            }
        extentFilter = &extent;
        }

    bool includeIntersecting = false;
    auto includeIntersectingJson = props[json_includeIntersecting()];
    if (!includeIntersectingJson.isNull())
        {
        if (!includeIntersectingJson.isBool())
            {
            errorMessage = "includeIntersecting must be a boolean";
            return GeoCoordinates::GEOCOORDERR_BadArg;
            }
        includeIntersecting = includeIntersectingJson.asBool();
        }

    Utf8String unitFilter;
    auto unitJson = props["unit"];
    if (!unitJson.isNull())
        {
        if (!unitJson.isString())
            {
            errorMessage = "unit must be a string";
            return GeoCoordinates::GEOCOORDERR_BadArg;
            }
        unitFilter = unitJson.asString();
        }

    GeoCoordinates::VerticalDatumDictionaryPtr dictionary = GeoCoordinates::VerticalDatumDictionary::Get();
    if (!dictionary.IsValid())
        return GeoCoordinates::GEOCOORDERR_NoDictionary;
    if (SUCCESS != dictionary->GetStatus())
        return dictionary->GetStatus();

    bvector<Utf8String> names;
    StatusInt status = pointFilter
        ? dictionary->QueryVerticalDatumsAvailableAtPoint(names, *pointFilter)
        : extentFilter
            ? dictionary->QueryVerticalDatumsAvailableForRange(names, *extentFilter, includeIntersecting)
            : dictionary->QueryAllVerticalDatumsAvailable(names);

    if (GeoCoordinates::GEOCOORDERR_NotFound == status)
        return SUCCESS;

    if (SUCCESS != status)
        return status;

    auto canonicalUnitNames = GeoCoordinates::BaseGCS::GetSupportedJsonUnitNames();
    for (Utf8StringCR name : names)
        {
        GeoCoordinates::VerticalDatumInfoPtr info = dictionary->GetVerticalDatumInfoFromName(name, status);
        if (SUCCESS != status || !info.IsValid())
            return status;

        VerticalCRSListResponseProps verticalCrs;
        verticalCrs.m_crsName = name;
        verticalCrs.m_epsg = info->GetEPSGCode();
        info->GetDescription(verticalCrs.m_description);
        verticalCrs.m_deprecated = info->IsDeprecated();
        info->GetType(verticalCrs.m_type);
        Utf8String rawUnit;
        info->GetUnits(rawUnit);
        for (Utf8StringCR canonicalUnitName : canonicalUnitNames)
            {
            if (canonicalUnitName.EqualsIAscii(rawUnit))
                {
                verticalCrs.m_unit = canonicalUnitName;
                break;
                }
            }
        if (!unitFilter.empty() && !verticalCrs.m_unit.EqualsIAscii(unitFilter))
            continue;
        info->GetExtent(verticalCrs.m_extent);
        verticalCrs.m_id = GetLegacyVerticalCrsId(*info);
        results.push_back(verticalCrs);
        }

    return SUCCESS;
    }
