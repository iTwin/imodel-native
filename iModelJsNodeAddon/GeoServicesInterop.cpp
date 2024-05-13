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
// @param ignoreLegacy If true, only return GCS's that are not considered legacy.
// @param extent If provided, only return GCS's that contain the given extent. Minimum longitude and latitude correspond to extent.low.x and extent.low.y, respectively.
// Maximum longitude and latitude correspond to extent.high.x and extent.high.y, respectively.
//---------------------------------------------------------------------------------------
bvector<GCSListResponseProps> GeoServicesInterop::GetListOfGCS(bool ignoreLegacy, DRange2dCP extent)
    {
    static GeoCoordinates::BaseGCSPtr ll84GcsPtr = GeoCoordinates::BaseGCS::CreateGCS("LL84");
    bvector<GCSListResponseProps> listOfGCS;

    char csKeyName[128];
    GCSListResponseProps props;
    for (int index = 0; (0 < GeoCoordinates::CSMap::CS_csEnum(index, csKeyName, sizeof(csKeyName))); index++)
        {
        GeoCoordinates::BaseGCSPtr gcs = GeoCoordinates::BaseGCS::CreateGCS(csKeyName);

        if (ignoreLegacy) 
            {
            Utf8String gcsGroup;
            gcs->GetGroup(gcsGroup);
            if (0 == gcsGroup.CompareTo(Utf8String("LEGACY")))
                // gcs is a legacy GCS, skip it
                continue;
            }

        // Don't include GCS with no range or with a range that covers the whole world
        DRange2d gcsRange(DRange2d::From(gcs->GetMinimumLongitude(),
                        gcs->GetMinimumLatitude(),
                        gcs->GetMaximumLongitude(),
                        gcs->GetMaximumLatitude()));
        if (gcsRange.IsEmpty() || gcsRange.Area() == 64800) //64800 => Worldwide, not wanted
            {
            continue;
            }

        // Don't include GCS if it does not contain the extent
        if (extent)
            {
            DRange2d extentRange(DRange2d::From(extent->low.x, extent->low.y, extent->high.x, extent->high.y));
            if (!extentRange.IsContained(gcsRange))
                continue;
            }
 
        props.m_name = Utf8String(gcs->GetName());
        props.m_description = Utf8String(gcs->GetDescription());
        listOfGCS.push_back(props);
        }
    
    return listOfGCS;
    }
