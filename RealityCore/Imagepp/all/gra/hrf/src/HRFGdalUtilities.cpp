//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFGdalUtilities
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#ifdef IPP_HAVE_GDAL_SUPPORT

#include <ImagePP/all/h/HPMAttributeSet.h>
#include <ImagePP/all/h/HRFGdalUtilities.h>
#include <ImagePP/all/h/HCPGeoTiffKeys.h>

#include <ImagePP-GdalLib/cpl_conv.h>
#include <ImagePP-GdalLib/ogr_spatialref.h>


// All this ugly stuff below exists so we do not need to add gdal internals to our search path.
// Hopefully we are planning or redesigning how we handle geocoding. 
// #ifndef BEIJING_WIP_IMAGEPP
     #define GEO_CONFIG_H // avoid inclusion of geo_config.h stuff
     #include "../../../../ext/gdal/frmts/gtiff/libgeotiff/geo_normalize.h"
// #endif

extern "C" char CPL_DLL* GTIFGetOGISDefn(GTIF*, GTIFDefn*);
extern "C" int32_t  CPL_DLL  GTIFSetFromOGISDefn(GTIF*, const char*);

//-----------------------------------------------------------------------------
// Public
// Static method used to convert a WKT string to Geotiff keys.
//-----------------------------------------------------------------------------
HCPGeoTiffKeys* HRFGdalUtilities::ConvertOGCWKTtoGeotiffKeys(char const* pi_pOGCWKT)
    {
    HCPGeoTiffKeys* pGeoTiffKeys = new HCPGeoTiffKeys();
    
    void* pTif = NULL;
    GTIF* pGTIF = GTIFNew(pTif);
    GTIFSetFromOGISDefn(pGTIF, pi_pOGCWKT);

    //TR 234118 - It is really necessary to initialize this variable to zero, since a memcpy is used
    //by GTIFKeyGet to assign the variable.
    uint32_t ULongVal = 0;
    double DoubleVal = 0;
    char   CharVal[1000] = {0};
       
    if (GTIFKeyGet(pGTIF, GTModelTypeGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GTModelTypeGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, GTRasterTypeGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GTRasterTypeGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, GTCitationGeoKey, CharVal, 0, 1))
        {
        CharVal[sizeof(CharVal) - 1] = 0;
        pGeoTiffKeys->AddKey(GTCitationGeoKey, CharVal);
        }

    if (GTIFKeyGet(pGTIF, GeographicTypeGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GeographicTypeGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, GeogCitationGeoKey, CharVal, 0, 1))
        {
        CharVal[sizeof(CharVal) - 1] = 0;
        pGeoTiffKeys->AddKey(GeogCitationGeoKey, CharVal);
        }

    if (GTIFKeyGet(pGTIF, GeogGeodeticDatumGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GeogGeodeticDatumGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, GeogPrimeMeridianGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GeogPrimeMeridianGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, GeogLinearUnitsGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GeogLinearUnitsGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, GeogLinearUnitSizeGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GeogLinearUnitSizeGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, GeogAngularUnitsGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GeogAngularUnitsGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, GeogAngularUnitSizeGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GeogAngularUnitSizeGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, GeogEllipsoidGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GeogEllipsoidGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, GeogSemiMajorAxisGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GeogSemiMajorAxisGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, GeogSemiMinorAxisGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GeogSemiMinorAxisGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, GeogInvFlatteningGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GeogInvFlatteningGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, GeogAzimuthUnitsGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GeogAzimuthUnitsGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, GeogPrimeMeridianLongGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GeogPrimeMeridianLongGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjectedCSTypeGeoKey, &ULongVal, 0, 1))
        {
        //The GeoTIFF specification doesn't allow the use of integer value greater
        //than 65535. So it can be assumed that GDAL won't return values greater
        //than 65535 for ProjectedCSTypeGeoKey.
        HASSERT(ULongVal <= USHRT_MAX);
        pGeoTiffKeys->AddKey(ProjectedCSTypeGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, PCSCitationGeoKey, CharVal, 0, 1))
        {
        CharVal[sizeof(CharVal) - 1] = 0;
        pGeoTiffKeys->AddKey(PCSCitationGeoKey, CharVal);
        }

    if (GTIFKeyGet(pGTIF, ProjectionGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjectionGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, ProjCoordTransGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjCoordTransGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, ProjLinearUnitsGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjLinearUnitsGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, ProjLinearUnitSizeGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjLinearUnitSizeGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjStdParallel1GeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjStdParallel1GeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjStdParallel2GeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjStdParallel2GeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjNatOriginLongGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjNatOriginLongGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjNatOriginLatGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjNatOriginLatGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjFalseEastingGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjFalseEastingGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjFalseNorthingGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjFalseNorthingGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjFalseOriginLongGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjFalseOriginLongGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjFalseOriginLatGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjFalseOriginLatGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjFalseOriginEastingGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjFalseOriginEastingGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjFalseOriginNorthingGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjFalseOriginNorthingGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjCenterLongGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjCenterLongGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjCenterLatGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjCenterLatGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjCenterEastingGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjCenterEastingGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjCenterNorthingGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjCenterNorthingGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjScaleAtNatOriginGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjScaleAtNatOriginGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjScaleAtCenterGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjScaleAtCenterGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjAzimuthAngleGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjAzimuthAngleGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjStraightVertPoleLongGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjStraightVertPoleLongGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjRectifiedGridAngleGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjRectifiedGridAngleGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, VerticalCSTypeGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(VerticalCSTypeGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, VerticalCitationGeoKey, CharVal, 0, 1))
        {
        CharVal[sizeof(CharVal) - 1] = 0;
        pGeoTiffKeys->AddKey(VerticalCitationGeoKey, CharVal);
        }

    if (GTIFKeyGet(pGTIF, VerticalDatumGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(VerticalDatumGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, VerticalUnitsGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(VerticalUnitsGeoKey, ULongVal);
        }

    GTIFFree(pGTIF);

    return pGeoTiffKeys;
    }

//-----------------------------------------------------------------------------
// Public
// Static method used to convert some Geotiff keys to a WKT string.
//-----------------------------------------------------------------------------
bool HRFGdalUtilities::ConvertGeotiffKeysToOGCWKT(AStringR OGCWKT, HCPGeoTiffKeys const& geoTiffKeys)
    {
    void*                 pTif = NULL;
    GTIF*                 pGTIF = GTIFNew(pTif);
    GTIFDefn              GTIFDefn;

    uint32_t ULongVal = 0;
    double DoubleVal = 0.0;
    AString CharVal;

    if (geoTiffKeys.GetValue(GTModelTypeGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, GTModelTypeGeoKey, TYPE_SHORT, 1, (int16_t)ULongVal);
        }

    if (geoTiffKeys.GetValue(GTRasterTypeGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, GTRasterTypeGeoKey, TYPE_SHORT, 1, (int16_t)ULongVal);
        }

    if (geoTiffKeys.GetValue(GTCitationGeoKey, CharVal))
        {
        GTIFKeySet(pGTIF, GTCitationGeoKey, TYPE_ASCII, 1, CharVal.c_str());
        }

    if (geoTiffKeys.GetValue(GeographicTypeGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, GeographicTypeGeoKey, TYPE_SHORT, 1, (int16_t)ULongVal);
        }

    if (geoTiffKeys.GetValue(GeogCitationGeoKey, CharVal))
        {
        GTIFKeySet(pGTIF, GeogCitationGeoKey, TYPE_ASCII, 1, CharVal.c_str());
        }

    if (geoTiffKeys.GetValue(GeogGeodeticDatumGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, GeogGeodeticDatumGeoKey, TYPE_SHORT, 1, (int16_t)ULongVal);
        }

    if (geoTiffKeys.GetValue(GeogPrimeMeridianGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, GeogPrimeMeridianGeoKey, TYPE_SHORT, 1, (int16_t)ULongVal);
        }

    if (geoTiffKeys.GetValue(GeogLinearUnitsGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, GeogLinearUnitsGeoKey, TYPE_SHORT, 1, (int16_t)ULongVal);
        }

    if (geoTiffKeys.GetValue(GeogLinearUnitSizeGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, GeogLinearUnitSizeGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (geoTiffKeys.GetValue(GeogAngularUnitsGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, GeogAngularUnitsGeoKey, TYPE_SHORT, 1, (int16_t)ULongVal);
        }

    if (geoTiffKeys.GetValue(GeogAngularUnitSizeGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, GeogAngularUnitSizeGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (geoTiffKeys.GetValue(GeogEllipsoidGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, GeogEllipsoidGeoKey, TYPE_SHORT, 1, (int16_t)ULongVal);
        }

    if (geoTiffKeys.GetValue(GeogSemiMajorAxisGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, GeogSemiMajorAxisGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (geoTiffKeys.GetValue(GeogSemiMinorAxisGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, GeogSemiMinorAxisGeoKey, TYPE_SHORT, 1, (int16_t)ULongVal);
        }

    if (geoTiffKeys.GetValue(GeogInvFlatteningGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, GeogInvFlatteningGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (geoTiffKeys.GetValue(GeogAzimuthUnitsGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, GeogAzimuthUnitsGeoKey, TYPE_SHORT, 1, (int16_t)ULongVal);
        }

    if (geoTiffKeys.GetValue(GeogPrimeMeridianLongGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, GeogPrimeMeridianLongGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (geoTiffKeys.GetValue(ProjectedCSTypeGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, ProjectedCSTypeGeoKey, TYPE_SHORT, 1, (int16_t)ULongVal);
        }

    if (geoTiffKeys.GetValue(PCSCitationGeoKey, CharVal))
        {
        GTIFKeySet(pGTIF, PCSCitationGeoKey, TYPE_ASCII, 1, CharVal.c_str());
        }

    if (geoTiffKeys.GetValue(ProjectionGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, ProjectionGeoKey, TYPE_SHORT, 1, (int16_t)ULongVal);
        }

    if (geoTiffKeys.GetValue(ProjCoordTransGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, ProjCoordTransGeoKey, TYPE_SHORT, 1, (int16_t)ULongVal);
        }

    if (geoTiffKeys.GetValue(ProjLinearUnitsGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, ProjLinearUnitsGeoKey, TYPE_SHORT, 1, (int16_t)ULongVal);
        }

    if (geoTiffKeys.GetValue(ProjLinearUnitSizeGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjLinearUnitSizeGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (geoTiffKeys.GetValue(ProjStdParallel1GeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjStdParallel1GeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (geoTiffKeys.GetValue(ProjStdParallel2GeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjStdParallel2GeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (geoTiffKeys.GetValue(ProjNatOriginLongGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjNatOriginLongGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (geoTiffKeys.GetValue(ProjNatOriginLatGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjNatOriginLatGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (geoTiffKeys.GetValue(ProjFalseEastingGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjFalseEastingGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (geoTiffKeys.GetValue(ProjFalseNorthingGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (geoTiffKeys.GetValue(ProjFalseOriginLongGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjFalseOriginLongGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (geoTiffKeys.GetValue(ProjFalseOriginLatGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjFalseOriginLatGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (geoTiffKeys.GetValue(ProjFalseOriginEastingGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjFalseOriginEastingGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (geoTiffKeys.GetValue(ProjFalseOriginNorthingGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjFalseOriginNorthingGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (geoTiffKeys.GetValue(ProjCenterLongGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjCenterLongGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (geoTiffKeys.GetValue(ProjCenterLatGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjCenterLatGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (geoTiffKeys.GetValue(ProjCenterEastingGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjCenterEastingGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (geoTiffKeys.GetValue(ProjCenterNorthingGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjCenterNorthingGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (geoTiffKeys.GetValue(ProjScaleAtNatOriginGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjScaleAtNatOriginGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (geoTiffKeys.GetValue(ProjScaleAtCenterGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjScaleAtCenterGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (geoTiffKeys.GetValue(ProjAzimuthAngleGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjAzimuthAngleGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (geoTiffKeys.GetValue(ProjStraightVertPoleLongGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjStraightVertPoleLongGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (geoTiffKeys.GetValue(ProjRectifiedGridAngleGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjRectifiedGridAngleGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (geoTiffKeys.GetValue(VerticalCSTypeGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, VerticalCSTypeGeoKey, TYPE_SHORT, 1, (int16_t)ULongVal);
        }

    if (geoTiffKeys.GetValue(VerticalCitationGeoKey, CharVal))
        {
        GTIFKeySet(pGTIF, VerticalCitationGeoKey, TYPE_ASCII, 1, CharVal.c_str());
        }

    if (geoTiffKeys.GetValue(VerticalDatumGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, VerticalDatumGeoKey, TYPE_SHORT, 1, (int16_t)ULongVal);
        }

    if (geoTiffKeys.GetValue(VerticalUnitsGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, VerticalUnitsGeoKey, TYPE_SHORT, 1, (int16_t)ULongVal);
        }

    GTIFGetDefn(pGTIF, &GTIFDefn);

    char* pWKT = nullptr;
    pWKT = GTIFGetOGISDefn(pGTIF, &GTIFDefn);

    if(nullptr != pWKT)
        OGCWKT = pWKT;

    CPLFree(pWKT);
    GTIFFree(pGTIF);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool HRFGdalUtilities::ConvertERMToOGCWKT(  AStringR po_rOGCWKT,
                                            CharCP pi_pErmProjection, 
                                            CharCP pi_pErmDatum,
                                            CharCP pi_pErmUnits )
    {
    char* wkt = nullptr;
    OGRSpatialReference oSRS;

    // If the projection is user-defined or unknown ... we will try to use GDAL to obtain the information required to use this "user-defined"
    if( oSRS.importFromERM(pi_pErmProjection, pi_pErmDatum, pi_pErmUnits ) == OGRERR_NONE )
        {
        if (OGRERR_NONE == oSRS.exportToWkt(&wkt))
            {
            HASSERT(wkt != nullptr);

            char localCsPrefix[] = "LOCAL_CS";

            if (strncmp(localCsPrefix, wkt, strlen(localCsPrefix)) == 0)
                {
                delete wkt;

                wkt = nullptr;
                }
            }
        }
    if (wkt != nullptr)
        {
        po_rOGCWKT = wkt;
        delete wkt;
        return true;
        }
    return false;
    }
#endif
