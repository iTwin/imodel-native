/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "IModelJsNative.h"
#include <Bentley/BeDirectoryIterator.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <ECObjects/ECJsonUtilities.h>

using namespace IModelJsNative;

BE_JSON_NAME(geographicCRSDef)
BE_JSON_NAME(geographicCRS)
BE_JSON_NAME(format)
BE_JSON_NAME(status)

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
        Json::Value CRSVal;
        newGCS->ToJson(CRSVal, true);

        // Convert old style Json value to new style
        BeJsDocument theDoc(CRSVal.toStyledString());
        results[json_geographicCRS()].From(theDoc);
        }

    return (BentleyStatus)status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// @param extent If provided, only return CRS's that contain the given extent. Minimum longitude and latitude correspond to extent.low.x and extent.low.y, respectively.
// Maximum longitude and latitude correspond to extent.high.x and extent.high.y, respectively.
//---------------------------------------------------------------------------------------
bvector<CRSListResponseProps> GeoServicesInterop::GetListOfCRS(DRange2dCP extent, bool includeWorld)
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
        Utf8String unitStr;
        crs->GetUnits(unitStr);
        props.m_name = Utf8String(crs->GetName());
        props.m_description = Utf8String(crs->GetDescription());
        props.m_deprecated = crs->IsDeprecated();
        props.m_crsExtent = crsRange;
        props.m_unit = unitStr;

        listOfCRS.push_back(props);
        }
    
    return listOfCRS;
    }
