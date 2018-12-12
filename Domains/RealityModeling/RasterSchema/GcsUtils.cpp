/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/GcsUtils.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RasterInternal.h"
#include "GcsUtils.h"

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  5/2015
//----------------------------------------------------------------------------------------
bool GcsUtils::IsHardError(ReprojectStatus status)
    {
    switch (status)
        {
        case REPROJECT_CSMAPERR_OutOfUsefulRange:
        case REPROJECT_CSMAPERR_VerticalDatumConversionError:
        case REPROJECT_Success:
            return false;

        default:
            break;
        }

    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
StatusInt GcsUtils::Reproject(DPoint3dR dest, DgnGCSCR destGcs, DPoint3dCR srcCartesian, GeoCoordinates::BaseGCSCR srcGcs)
    {
    GeoPoint srcGeo;
    if (IsHardError(srcGcs.LatLongFromCartesian(srcGeo, srcCartesian)))
        return ERROR;
        
    // Source latlong to destination latlong.
    GeoPoint destGeo;
    ReprojectStatus status = srcGcs.LatLongFromLatLong(destGeo, srcGeo, destGcs);
    
    // For datum conversion error 2 is in fact a strong warning instead of a hard math error
    if (IsHardError(status) && REPROJECT_CSMAPERR_OutOfMathematicalDomain != status)
        return ERROR;

    // Finally to destination World unit.
    if (IsHardError(destGcs.UorsFromLatLong(dest, destGeo)))
        return ERROR;

    return SUCCESS;
    }
