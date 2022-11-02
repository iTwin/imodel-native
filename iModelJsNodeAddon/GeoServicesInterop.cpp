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


